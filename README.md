# rpi-pico-tableClock
### Simple application of Raspberry Pi Pico using ST7920, PIO and RTC module
### This also uses the Maker Pico from Cytron: https://my.cytron.io/pi-pico

GPIO pins used:

|gpio   |st7920 pins   |
|---|---|
|6-9   | D4-D7 |
|15   | E  |
|14   | RS  |
|26   | RST  |

|gpio   |other pins   |
|---|---|
|21  | set button |
|22  | up button | 
|27  | interrupt |

![Table Clock](/picoTableClock.jpg)

Analog clock thanks to author: https://github.com/xpress-embedo/AnalogClockOLED/
