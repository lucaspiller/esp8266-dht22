# esp8266-dht22

ESP8266 solar powered temperature sensor.

## About

This is the firmware for a ESP8266 based temperature sensor, powered by a small
solar panel. It takes a reading from a DHT22 temperature/humidity sensor, and
sends the data over TCP to a server (in this case [listening on a Raspberry Pi](https://github.com/lucaspiller/meteorpi)).

This is running on a [ES8266-12
module](http://blog.hekkers.net/wp-content/uploads/2015/03/ESP-8266-12.jpg), I
tried a few ESP8266 modules and this gave the lowest power consupmtion in deep
sleep mode (in the µA range). When the module wakes up it takes a reading,
connects to wifi and transmits the data. It then sleeps for approximately 5
minutes (the timer isn't very accurate) before doing the same again. If too
many errors occurs it also enters deep sleep mode and retries in 5 minutes.

The power source is three 800mAh AA NiMH batteries, on their own these will
give a run time of around one month. A solar panel is connected to the
batteries (directly with a Schottkey diode), which even on a not-so-sunny
British winter day is enough to keep the batteries topped up. The solar panel
is rated at 5.5V and 120mA and cost a few pounds from eBay. The reading also
includes the internal voltage of the ESP8266, so the battery level can be
monitored. The batteries are connected directly to the ESP8266, even though it
is rated at 3.3V it doesn't seem to have any issues running at slightly higher
voltages.

The sensor is housed in a plastic food container, with holes pierced in the
sides to provide air flow. The sensor has been running for a week now, so the
long term longivity is unknown. I will update this as issues occur.

## Compiling yourself

1. Setup the ESP8266 toolchain. [Docker is the easiest way](http://www.wolfe.id.au/2015/07/06/iot-development-with-docker-containers/).
2. Copy and edit `user/user_config.h.example`.
3. `make`
4. Flash it to your device.
