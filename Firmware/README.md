
# Theia Thermal Camera - Firmware

Theia firmware V1 is an Arduino IDE project. You need to have the Arduino core for the ESP32 installed to build the firmware. <br>

## Dependencies 

[Display Driver] LovyanGFX 1.2.0 (a small modification needed for IPS Displays) <br>
[GUI] LVGL 8.3.0 <br> 
[Timer] ESP32Time 2.0.6 <br>
[Status Led] FastLED 3.9.13 <br>

## Operating

Theia can be controlled locally using the graphics user interface (Theia GUI) or remotely using a command-based interface (Theia Serial) available via USB. Both interfaces are detailed below.

### Theia USB Connector 1
The first USB Connector is used mainly for debug purposes (baud 115200) but it can be used to provide power to the camera. (By default the debug interface is disabled).

### Theia USB Connector 2
The USB connector is used to provide power to the camera and as a programming/control interface for Theia. This port is used by Theia Tool v1.0.0b to access the camera.

### Theia SD Card
Image files (RAW or BMP) are stored in the attached SD card. If none is attached the firmware will detect the absence of the SD Card and the user won't be able to save any images nor browse the content of the card. <br> Files are saved under RAW or BMP format (.timg or .bmp) after being scaled to 320x240.

## Theia GUI

<br>
<img src="/screenshots/TheiaGUI.png" width="480" height="360">

### Boot Screen
<br>
<img src="/screenshots/splash.png" width="320" height="240">
### Main Screen 
<br>
<img src="/screenshots/mainScreen.png" width="320" height="240">
### Browse Screen
<br>
<img src="/screenshots/browseScreen.png" width="320" height="240">
### Settings Screen
<br>
<img src="/screenshots/settingsScreen.png" width="320" height="240">
## Theia Serial



## Authors

- [Che Ahmed](https://github.com/CheAhMeD)


## Badges

[![AGPL License](https://img.shields.io/badge/license-GPL%20V3.0-blue.svg)](http://www.gnu.org/licenses/gpl-3.0)

