#  SH1106 / EC11 with CH32V203 using ch32fun

PlatformIO project with the [ch32fun](https://github.com/cnlohr/ch32fun) development environment. A [WCH LinkE](https://www.aliexpress.com/item/1005004881582037.html) debugger/programmer is preferred for uploading code to the microcontroller.

This example shows triangle waves in a SH1106 based 1.3 in 128×64 OLED display connected via I<sup>2</sup>C. Waves are horizontally scrolled according to a EC11 encoder's rotation direction.

Fonts included are in the Adafruit GFX format. You can convert a ttf file to this format using [this web page](https://fontconvert.huyzona.com). The fonts used in this project are:

* [Tiny Talk by Vexed](https://v3x3d.itch.io/tiny-talk)
* [Ithaca by GGBotNet](https://ggbot.itch.io/ithaca-font)

### Used parts links:

* [BluePill+ CH32V203C8T6](https://www.aliexpress.com/item/1005006117720765.html)
* [SH1106 / EC11 module](https://www.aliexpress.com/item/1005007584723503.html)

### Board connections:

| Board pin | Connected to   |
|-----------|----------------|
| SW GND    | LinkE's GND    |
| SW SCK    | LinkE's SWCLK  |
| SW DIO    | LinkE's SWDIO  |
| SW 3V3    | LinkE's 3V3    |
| 3V3       | OLED's Vᴅᴅ     |
| G         | OLED's GND     |
| A9        | LinkE's RX     |
| A10       | LinkE's TX     |
| B6        | OLED's SCL[^1] |
| B7        | OLED's SDA[^1] |
| A6        | Encoder's TRA  |
| A7        | Encoder's TRB  |

[^1]: A pull-up resistor (1KΩ or higher) might be needed.
