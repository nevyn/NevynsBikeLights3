# Nevyn's Bike Lights v3

This is the followup project to 
https://github.com/nevyn/NevynsArduino/tree/master/NevynsBikeLights2

Instead of having an Arduino Uno in a box under the seat, with an eight-
cable interconnect going to the steering rod, this iteration puts a
Pro Micro in a box _on the steering rod_, which means the controls
can go directly on the box. This makes so much more sense,
I have no idea why I was making it so complicated for V2. This means
the only cable I have to pull along the bike is the 4 wires (+, -,
data and clock) for the LED strip.

As usual, I'm just going with the hardware I have home rather than
making smart decisions. So the key hardware is:

* DotStar LED strip instead of NeoPixel, _with vinyl wrapping_ to
  make it weather proof (no reason to pick DotStar over NeoPixel,
  it's just what I had at home).
* SparkFun Pro Micro (only real advantage over Uno in this case
  is that it's smaller and cheaper)
* A rotary encoder for flipping between modes (seems fun)

So there's the box with the electronics, then a single long line
of LEDS are pulled along the bike, and addressed from code in
sections. The plan is to have the following sections:

* Left edge side of the handlebar
* along front of handlebar
* Right edge of the handlebar
* along left edge of top tube
* along right edge of top tube
* along the edge of basket on rear of bike (stretching both along
  the sides and the rear, so indicators are very visible)

We'll see how far we get with the project this time...
