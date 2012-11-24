# WeatherStation

Arduino code, tools and utilities to receive and publish data from an
Oregon Scientific weather station.

The Arduino code here is a modified version of the decoder found at
[http://jeelabs.net/projects/cafe/wiki/Decoding_the_Oregon_Scientific_V2_protocol](http://jeelabs.net/projects/cafe/wiki/Decoding_the_Oregon_Scientific_V2_protocol).

## Hardware

* Arduino-compatible board.
* 433MHz receiver. I bought the [Freetronics 433MHz shield](http://www.freetronics.com/products/receiver),
  but any old 433MHz receiver will do. The data pin from the receiver should
  be connected to analogue pin 1 (the Freetronics shield connects the 
  input to D8, I ran a jumper from it to A1).
* An Oregon Scientific [THGN132N](http://uk.oregonscientific.com/cat-Weather-sub-Sensors-prod-THGN132N-Sensor.html) temperature / humidity sensor. This was
  the sensor provided with [BAR808HG](http://au.oregonscientific.com/cat-Weather-sub-ECO-Solar-prod-Solar-Weather-Station-Advanced.html) weather station.
* A Linux server. The arduino is plugged in to my Ubuntu media server, but
  any current distribution should be fine.

## Software

The Linux server will need:
* rrdtool (`rrdtool` package in Debian/Ubuntu)
* python, and python rrdtool interface (`python-rrdtool` in Debian/Ubuntu)
* cron
* A web server. I use apache on this box.

udev is optional but nice to have.

As well as the arduino sketch, this repository contains:
* `weather-station.rules`. A udev config fragment to give the arduino a
  persistent device name. Update this file with your arduino's serial number,
  stick it in `/etc/udev/rules.d/`, and the arduino will always be available
  as `/dev/weatherstation` when it's plugged in.
* `rrdtool-create.sh` will create an RRD to store temperature and humidity.
* `serial-test.py` will poll the arduino and retrieve the latest weather data.
* `weatherpoll.py` polls the arduino, parses the received data, and updates
  the RRD.
* `suntimes.sh` downloads sunrise and sunset data, which is used in graphs.
* `graph.sh` generates graphs.
* `weatherstation.cron` is entries from my user crontab to run everything.

## THGN132N Decoding

This sensor uses the Oregon Scientific V2 protocol. The `Ook_OSv2` sketch
from [http://jeelabs.net/projects/cafe/wiki/Decoding_the_Oregon_Scientific_V2_protocol](http://jeelabs.net/projects/cafe/wiki/Decoding_the_Oregon_Scientific_V2_protocol)
decodes the data packets. A data packet looks similar to Oregon Scientific
devices - the header stuff is identical, and numbers from the sensor are
stored in binary-coded decimal. A sample packet looks like this:

`1A 2D 10 EC 32 27 50 06 44 25`

The nibbles that I know about are:

* 0-3: Device ID. The ID for THGN132N sensors is `1A2D`.
* 4: Channel. This corresponds to the channel slider on the back of the
  sensor.
* 5: Battery? All of my readings have 0 for this nibble. I'm half-expecting
  it to become non-zero on low battery.
* 6-7: Rolling code. This is a unique identifier for the sensor. It resets
  when the battery is replaced.
* 8: The tenths digit of the temperature.
* 10: The tens digit of the temperature.
* 11: The unit digit of the temperature.
* 12: The unit digit of the humidity.
* 13: The sign for the temperature. This nibble will be 0 for a +ve temp,
  and non-zero for -ve. During my testing with the sensor in the freezer,
  I've only seen this return 0 or 8.
* 15: The tens digit of the humidity.

The sample packet above is from a THGN132N on channel 1 with rolling code
`EC`. It's returning a temperature of +27.3°C, and humidity 65%.

I'm expecting the checksum to work like other Oregon Scientific devices,
but haven't yet implemented it.

