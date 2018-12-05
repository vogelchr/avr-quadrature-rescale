#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

/*
 *
 *               +---------------+
 *  MOSI(ISP)    |  1 PB0 PA0 20 |      QIN0
 *  MISO(ISP)    |  2 PB1 PA1 19 |      QIN1
 *  SCK(ISP)     |  3 PB2 PA2 18 |      Switch
 *  LED2 OC1B    |  4 PB3 PA3 17 |      TP
 *               |  5 Vcc GND 16 |
 *               |  6 GND Vcc 15 |
 *  LED1         |  7 PB4 PA4 14 |      TP
 *  QOUT1        |  8 PB5 PA5 13 |      TP
 *  QOUT0        |  9 PB6 PA6 12 |      TP
 *  \Reset       | 10 PB7 PA7 11 |      TP
 *               +---------------+
 */

/* table to decode the quadrature input, delta depending on last/curr state */
const signed char quad_state_tbl[16] PROGMEM = {
	[0b0001]=+1, /* from [..] -> [.H] */
	[0b0111]=+1, /* from [.H] -> [HH] */
	[0b1110]=+1, /* from [HH] -> [H.] */
	[0b1000]=+1, /* from [H.] -> [..] */

	[0b0010]=-1, /* from [..] -> [H.] */
	[0b1011]=-1, /* from [H.] -> [HH] */
	[0b1101]=-1, /* from [HH] -> [.H] */
	[0b0100]=-1, /* from [.H] -> [..] */
};

#define DDR_QOUT DDRB
#define PORT_QOUT PORTB
#define BV_QOUT0 _BV(6)
#define BV_QOUT1 _BV(5)

#define PORT_QIN PORTA
#define DDR_QIN  DDRA
#define PIN_QIN  PINA
#define BV_QIN0  _BV(0)
#define BV_QIN1  _BV(1)

#define DDR_LED1  DDRB
#define PORT_LED1 PORTB
#define BV_LED1   _BV(4)

#define DDR_LED2  DDRB
#define PORT_LED2 PORTB
#define BV_LED2   _BV(3)

static void
set_led1(int val) {
	if (val)
		PORT_LED1 |= BV_LED1;
	else
		PORT_LED1 &= ~BV_LED1;
}

static void
set_led2(int val) {
	if (val)
		PORT_LED2 |= BV_LED2;
	else
		PORT_LED2 &= ~BV_LED2;
}

static unsigned char
read_quad_input() {
	unsigned char q = 0;
	/* low active inputs */
	if (!(PIN_QIN & BV_QIN0))
		q |= 0x01;
	if (!(PIN_QIN & BV_QIN1))
		q |= 0x02;
	return q;
}

static void
write_quad_output(unsigned char q) {
	/* low active outputs, pulldown via DDR=1, PORT=0 */
	if (q & 0x01) {
		PORT_QOUT &= ~BV_QOUT0;
		DDR_QOUT  |= BV_QOUT0;
	} else {
		DDR_QOUT &= ~BV_QOUT0;
		PORT_QOUT |= BV_QOUT0;
	}

	if (q & 0x02) {
		PORT_QOUT &= ~BV_QOUT1;
		DDR_QOUT |= BV_QOUT1;
	} else {
		DDR_QOUT &= ~BV_QOUT1;
		PORT_QOUT |= BV_QOUT1;
	}
}

static unsigned char led2_dir;
static uint16_t led2_ctr;

/* timer overflow to amuse the user with a fading led */
ISR(TIMER1_OVF1_vect) {
	unsigned char dir = led2_dir;
	uint16_t ctr = led2_ctr;

#if 0
	if (ctr == 1023)
		dir = 0; /* count down */
	if (ctr == 0)
		dir = 1; /* count up */

	if (dir)
		ctr++;
	else
		ctr--;

	OCR1B = ((ctr >> 2)*(ctr >> 2)) >> 8;
#endif

	led2_ctr = ctr;
	led2_dir = dir;
}

static unsigned char
quadrature_encode(unsigned char ctr) {
	return (ctr & 0x03) ^ ((ctr & 0x02) >> 1);
}

static unsigned char quad_last;
static uint16_t quad_ctr=1; /* don't start at the ffff -> 0000 wraparound! */

static void
process_quadrature() {
	signed char s;
	unsigned char q=0;

	q = read_quad_input();
	if (q == quad_last)
		return;

	s = pgm_read_byte(&quad_state_tbl[(quad_last << 2) | q]);
	quad_last = q;

	quad_ctr += s;

	q = quadrature_encode(quad_ctr >> 2);
	write_quad_output(q);

	set_led1(q & 0x01);
	set_led2(q & 0x02);
}

int
main() {
	/* setup LEDs */
	DDR_LED1 |= BV_LED1;
	DDR_LED2 |= BV_LED2;
	PORT_LED1 |= BV_LED1;
	PORT_LED2 |= BV_LED2;

	/* inputs, pullup */
	DDR_QIN  &= ~ BV_QIN0; /* QIN0 is input */
	DDR_QIN  &= ~ BV_QIN1; /* QIN0 is input */
	PORT_QIN |= BV_QIN0; /* enable pullup for QIN 0 */
	PORT_QIN |= BV_QIN1; /* enable pullup for QIN 1 */

	/* outputs, enable DDR to pull low */
	DDR_QOUT  &= ~BV_QOUT0;
	DDR_QOUT  &= ~BV_QOUT1;
	PORT_QOUT &= ~BV_QOUT0;
	PORT_QOUT &= ~BV_QOUT1;

#if 0
	/* datasheet pg 73, fast PWM mode, set output on bottom, clr on match */
	TCCR1A = _BV(COM1B1) | _BV(PWM1B);
	TCCR1B = _BV(CS12); /* clkio /256 */
	TIMSK |= _BV(TOIE1) ; /* timer overflow */
	OCR1B = 0x10;
	OCR1C = 0xff;
#endif

	sei(); /* enable interrupts */

	/* initialize quadrature decoder, encoder output */
	quad_last = read_quad_input();
	write_quad_output(0);

	while (1)
		process_quadrature();
}
