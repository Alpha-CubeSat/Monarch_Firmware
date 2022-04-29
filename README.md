---

Hi. Welcome to the Monarch_Firmware project! Hunter Adam has the copyright of the original [code](https://github.com/vha3/Monarch-Software/tree/vineyard) based on the EasyLink library. This project rewrote the radio library to support long-distance communication and other parts of the code roughly remain the same.

---

# Usage

The platforms used are listed here.
+ Code Composer Studio, version 8.3.1.00004.
+ Simplelink, version 2.30.0.20.
+ CC1310 microcontroller and launchpad.

After compiling and flushing the code into the CC1310 microcontroller, the LED should flash periodically, and the encoded data should be sent in frequency 915 MHZ. 

Note that this code is in charge of the sending part of CC1310 microcontroller. The corresponding receiving part has another [repository](https://github.com/Alpha-CubeSat/Monarch-Ground-Station).

## References
* [zac's paper](https://zacmanchester.github.io/docs/Zac_Manchester_PhD_Dissertation.pdf)
* [zac's codel](https://github.com/kicksat/arduino_sprite/blob/master/libraries/SpriteRadio/SpriteRadio.cpp)
