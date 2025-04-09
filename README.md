
# Theia Thermal Imaging Cameras

![Logo](/images/logo_full.png)

Theia is a DIY thermal camera designed around the ESP32-S3 and Lepton 3.5.
It is started as a way to explore the Lepton 3.5 sensor and its abilities and turned to be a full (yet simple) thermal camera.

## Theia Camera
Theia is a well featured, open source camera with a local touchscreen display, local storage and a USB interface.
It is composed of a custom PCB and a custom 3D enclosure (with optional stand).

![Theia](/images/IMG_2424.jpeg)

## PCB
The PCB has been created with KiCAD software. The project is open sourced.
Please use the gerber files provided if you want to order a PCB (The impedance control is important for the USB part to work). 

![PCB](/images/IMG_2426.jpeg)

## Firmware
The "Firmware" directory contains a Arduino IDE project for Theia. You should be able to build and load it into a camera using the normal process.

### Note:
Due to the custom pcb, to be able to upload the compiled firmware you need to set the Arduino Environment as follow:
<br>
<img src="/images/conf.png" width="80" height="120">

## Enclosure
A simple 2 piece enclosure designed to be 3D printed. Design was done with the online editor "onshape". 
The OnShape project folder can be accessed [here](https://cad.onshape.com/documents/da13b0f788f852a632ae2d5d/w/38727c48ed0b2ebb656da7c0/e/01333a5db09d1259df274e6b) and the generated STL files are included in the Enclosure directory.

![3D](/images/Enclosure.png)

## Desktop App
This DesktopApp folder contains the companion desktop application for Theia. It communicates with the cameras via a USB (Serial) connection using a predefined set of commands. 
It provides very basic functionalities to control the cameras, download and display images from them and analyze the radiometric data.

More details on the DesktopApp are available in /DesktopApp/README.md

## Python Examples
Simple Examples using a python script to perform basic operations on the camera.

## Acknowledgements
A lot of useful information were collected thanks the brilliant work done in tCam (By [Dan Julio](https://github.com/danjulio)) and diy-thermocam (By [Max Ritter](https://github.com/maxritter))

 - [tCam](https://github.com/danjulio/tCam)
 - [DIY-Thermocam](https://github.com/maxritter/diy-thermocam)



## Authors

- [Che Ahmed](https://github.com/CheAhMeD)


## Badges

[![AGPL License](https://img.shields.io/badge/license-GPL%20V3.0-blue.svg)](http://www.gnu.org/licenses/gpl-3.0)
