DEVICE_CC = attiny26
DEVICE_DUDE = t26

PROGRAMMER_DUDE = -Pusb -c dragon_isp

AVRDUDE=avrdude
OBJCOPY=avr-objcopy
OBJDUMP=avr-objdump
CC=avr-gcc
LD=avr-gcc

LDFLAGS=-Wall -g -mmcu=$(DEVICE_CC)
CPPFLAGS=
CFLAGS=-mmcu=$(DEVICE_CC) -Os -Wall -g -DF_CPU=16000000UL

MYNAME=avr-quadrature-rescale

OBJS=$(MYNAME).o

all : $(MYNAME).hex $(MYNAME).lst

$(MYNAME).bin : $(OBJS)

%.hex : %.bin
	$(OBJCOPY) -j .text -j .data -O ihex $^ $@ || (rm -f $@ ; false )

%.lst : %.bin
	$(OBJDUMP) -S $^ >$@ || (rm -f $@ ; false )

%.bin : %.o
	$(LD) $(LDFLAGS) -o $@ $^

ifneq ($(MAKECMDGOALS),clean)
include $(OBJS:.o=.d)
endif

%.d : %.c
	$(CC) -o $@ -MM $^

.PHONY : clean burn fuse
burn : $(MYNAME).hex
	$(AVRDUDE) $(PROGRAMMER_DUDE) -p $(DEVICE_DUDE) -U flash:w:$^

# Fuse: disable clkdiv/8, enable debugwire
fuse :
	$(AVRDUDE) $(PROGRAMMER_DUDE) -p $(DEVICE_DUDE) -U lfuse:w:0xe2:m -U hfuse:w:0x9f:m -U efuse:w:0xff:m

clean :
	rm -f *.bak *~ *.bin *.hex *.lst *.o *.d
