# Alphabeat-Controller
Arduino-Based MIDI-Interface for [MIXXX DJ-Software](https://mixxx.org)

## Used Libraries
* [MIDIUSB](https://tigoe.github.io/SoundExamples/midiusb.html)
* [Wire (I2C)](https://www.arduino.cc/en/reference/wire)
* [Adafruit_LEDBackpack](https://www.adafruit.com/product/1911)
* [Encoder Lib](http://www.pjrc.com/teensy/td_libs_Encoder.html)
* [Timer](https://github.com/brunocalou/Timer)
* [EWMA Filter](https://github.com/jonnieZG/EWMA)

## Hardware
* [Arduino Pro Micro](https://www.sparkfun.com/products/12640) 5V
* 16-segment LED displays with [HT16K33](https://cdn-shop.adafruit.com/datasheets/ht16K33v110.pdf)
* 2 Linear Fader 5k
* [Rotary Encoder](https://www.bourns.com/data/global/pdfs/ECW1J.pdf)
* Inverted Schmitt-Trigger IC [SN74HC14](https://www.ti.com/lit/ds/symlink/sn74hc14.pdf)
* LEDs and Buttons

## Change Name of USB-Hardware
Open **boards.txt** located at

C:\Program Files (x86)\Arduino\hardware\arduino\avr\boards.txt

or if you installed updates for your Arduino-board

C:\Users\<User-Name>\AppData\Local\Arduino15\packages\arduino\hardware\avr\<verion>\boards.txt

search for the section `micro.name=Arduino Micro`
and change the following line only: `micro.build.usb_product="AlphaBeat-Mixxa"`
