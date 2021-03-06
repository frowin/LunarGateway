# LunarGateway

The LunarGateway is an ESP32-based gateway (as an Arduino library) for the Acaia Lunar 2021 scale. It allows you to communicate and interact with the scale. It is possible to retrieve the weight and battery status. Moreover, the library is able to send commands to the scale. This includes tare, start/stop timer and more. 

With these capabilities, a variety of new applications in the field of speciality coffee arises! With an investment of about $3, you can develop features for your portafilter machine which are - to date - only present in costly catering machines. There is more in the making!

__Pull requests and code reviews are highly appreciated! Moreover, feel free to contact me or open an issue.__

## Requirements

The library is built exclusively for the ESP32 platform. Moreover, for now, only [Acaia Lunar 2021](https://eu.acaia.co/products/lunar_2021) scales are implemented. 

## Demo

You can find a demo on Youtube:

[![Video showing an ESP32 connecting to an Acaia Lunar 2021](https://img.youtube.com/vi/KmS2LABmM3s/0.jpg)](https://www.youtube.com/watch?v=KmS2LABmM3s)

## Examples

The library comes with examples. For "with_display", a SPI OLED display with the SH1106 chip and the respective library [u8g2](https://github.com/olikraus/u8g2) is needed. The "basic" example works out of the box with an ESP32. Note, that you need to insert the BLE MAC address of your own Acaia Lunar 2021 scale.

## Hints for usage

The Acaia Lunar 2021 communicates via BLE (Bluetooth Low Energy). It uses 2 characteristics: one for commands and heartbeats (ESP32 -> Lunar) and one for data payloads (Lunar -> ESP32). The scale expects a "heartbeat" message (at least) every 3000 ms. Otherwise it will close the BLE connection. The scale does not automatically stream the weight to the ESP32: This stream has to be initiated by a notification request `notificationRequest()` which is a method of the lunarGateway class. The weight and battery status are available as attributes. After the initialization of the weight notification with `notificationRequest()`, the scale will send the weight to the ESP32 at 5 Hz. This BLE payload triggers a callback function on the ESP32 and thus enables immediate processing of the data. Be aware that the LunarGateway is tightly bonded to the BLE library for the ESP32. The examples are a good starting point for own projects.

## Acknowledgement

Hereby, I want acknowledge the [pyacaia](https://github.com/lucapinello/pyacaia) library written in python which was a great help with encoding the commands.