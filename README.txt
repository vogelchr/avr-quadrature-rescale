I had an older piece of hardware which used a quadrature encoder,
unfortunately I wasn't able to purchase the exact replacement, because
I needed a low number of detents *and* only one pulse-step per detent.

The old encoder had detents and quadrature outputs which look like this:

** Old Encoder **

    detents
     v    v    v    v    v    v    v    v    v    v    v    v    v
   +----:----+         +----:----+         +----:----+         +---:-
   |         |         |         |         |         |         |
---+         +----:----+         +----:----+         +----:----+

        +----:----+         +----:----+         +----:----+         +-
        |         |         |         |         |         |         |
---:----+         +----:----+         +----:----+         +----:----+

The new decoders I found did two steps, or even one cycle per detent.

** New Encoder **

  detents
  V       V       V       V       V       V       V
 +-:-+   +-:-+   +-:-+   +-:-+   +-:-+   +-:-+   +-:-+   
 |   |   |   |   |   |   |   |   |   |   |   |   |   |   
-+   +-:-+   +-:-+   +-:-+   +-:-+   +-:-+   +-:-+   +-:-

   +-:-+   +-:-+   +-:-+   +-:-+   +-:-+   +-:-+   +-:-+   
   |   |   |   |   |   |   |   |   |   |   |   |   |   |   
-:-+   +-:-+   +-:-+   +-:-+   +-:-+   +-:-+   +-:-+   +-:-


The enclosing PCB houses a ATTiny26 (one of the most useless chips of the)
attiny series, I think, but I had a few spare. It converts puslses of a
"new encoder", divides then by a ratio of 4 steps/step, and outputs pulses
compatible to the old scheme. That's all it does.
