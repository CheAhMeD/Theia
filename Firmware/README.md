
# Theia Thermal Camera - Firmware

Theia firmware V1 is an Arduino IDE project. You need to have the Arduino core for the ESP32 installed to build the firmware. <br>

## Dependencies 

[Display Driver] LovyanGFX 1.2.0 (a small modification needed for IPS Displays) <br>
[GUI] LVGL 8.3.0 <br> 
[Timer] ESP32Time 2.0.6 <br>
[Status Led] FastLED 3.9.13 <br>
[CRC] CRC 1.0.3 (Rob Tillaart)

## Operating

Theia can be controlled locally using the graphics user interface (Theia GUI) or remotely using a command-based interface (Theia Serial) available via USB. Both interfaces are detailed below.

### Theia USB Connector 1
The first USB Connector is used mainly for debug purposes (baud 115200) but it can be used to provide power to the camera. (By default the debug interface is disabled).

### Theia USB Connector 2
The USB connector is used to provide power to the camera and as a programming/control interface for Theia. This port is used by Theia Tool v1.0.0b to access the camera.

### Theia SD Card
Image files (RAW or BMP) are stored in the attached SD card. If none is attached the firmware will detect the absence of the SD Card and the user won't be able to save any images nor browse the content of the card. <br> Files are saved under RAW or BMP format (.timg or .bmp) after being scaled to 320x240.

#### Note:
**The SD Card must be formatted in FAT32.**
**Theia Hardware doesn't provide a Card Detect signal, the firmware uses a dumb routines that checks if a certain file (.theia) exists on the SD Card to determine that the SD Card is attached. The file ".theia" is created at boot if it doesnt exist.**

## Theia GUI

<br>
<img src="screenshots/TheiaGUI.png" width="520" height="360">

### Boot Screen

A timed out screen that displays the logo and a boot log message.

<br>
<img src="screenshots/splash.png" width="480" height="320">

### Main Screen 

Camera main screen that contain the lepton stream and some controls.

<br>
<img src="screenshots/mainScreen.png" width="480" height="320">

#### Controls

| **Control** | **Function** |
|------|------|
| Snap Button  | Takes a snap and saves a BMP/TIMG image in the SD card under "Lepton" directory (a status message box will appear once pressed). |
| Browse Button  | Opens the browse screen. |
| FFC Button   | Performs a manual FFC. |
| Settings Button | Opens the settings screen.  |
|  Cursor    | Pressing on the lepton image updates the cursor position and the cursor temperature.   |

### Browse Screen

SD card browse screen. A very simple gui to browse and display the content of "Lepton" directory. <br>
It shows the file name, file number and total files in the directory.

<br>
<img src="screenshots/browseScreen.png" width="480" height="320">

#### Controls

| **Button** | **Function** |
|------|------|
| Close Button  | Returns to the main screen. |
| Next Button  | Displays the next image file. |
| Rewind Button   | Goes back to the first file in the directory. |
| Detete Button | (In blue) Deletes the current displayed image (a confirmation message box will appear once pressed).  |
|  Delete All Button    | (In red) Deletes all the files in Lepton Directory (a confirmation message box will appear once pressed).  |

### Settings Screen

Camera settings screen. It offers some controls over the behaviour of the camera as well as some controls over the lepton sensor.

<br>
<img src="screenshots/settingsScreen.png" width="480" height="320">

#### Controls

| **Control** | **Function** |
|------|------|
| Close Button  | Returns to the main screen. |
| Lepton Color Map | Updates the color map of the lepton image from the available 12 palettes. |
| Reset Button   | Resets the lepton sensor (a status message box will appear once pressed).  |
| Reboot Button   | Reboots the lepton sensor (a status message box will appear once pressed). |
| Gain Mode | Updates the lepton gain mode (High / Low / Auto). |
| Temp Unit   | Updates the temperature unit (°C / °F / K). |
| Save Image BMP   | Switches between .bmp / .timg formats for the saved image (using snap button). |
| Brightness   | Updates the brightness of the display. |
| Update Info   | Displays some diagnostics info in Theia Debug Info Box. |
| Rotate Screen   | Rotates the screen 180°. |

## Theia Serial

Theia Serial inteface is meant for basic controls over the camera using the native USB (In CDC mode) on the ESP32-S3. <br>
The communication is simplistic and based on frames. Meaning the host (Desktop App | Python Script ...) sends a 1-byte command (Set | Get) to the camera and the camera responds with the adequate frame if the sent command corresponds to one of the commands else it responds with a NAK byte. <br>

### Data Frame

#### Host (Command):

Get Commnad:
| **CMD ID** |
|------------|
|   1 Byte   |

Set Command:
| **CMD ID** |  **DATA**  |
|------------|------------|
|   1 Byte   |   n Bytes  |

#### Device (Response):

Unvalid Command Response:
| **NAK** |
|---------|
|  0x05   |

Get Response:
| **ACK** | **CMD ID** | **DATA** | **CRC8** | 
|---------|------------|----------|----------|
|  0x10   | 1 Byte     | n Bytes  | 1 Byte   | 

Set Response (Success/Failure Frame):
| **ACK** | **CMD ID** | **STATE** |
|---------|------------|-----------|
|  0x10   | 1 Byte     |  1 Byte   |

**STATE** is either success (0x01) or failure (0x00).

### Command list

| **Command** | **Value** | **Description** |
|------|------|------|
| Get Lepton Status  |   0x73  |   Returns a frame containing the Lepton Camera status.  |
| Get Lepton Diagnostics   |  0x64  | Returns a frame containing diagnostics data on the lepton.      |
| Get Image   |  0x44   |   Returns a frame containing the raw data captured from the camera.   |
|  Get Spot Temp  |   0x74   |   Returns a frame containing a string of current spotmeter temperature.  |
|  Get Run Time    | 0x52   |   Returns a frame containing a string of current camera runtime.   |
|  Get Storage Info   |   0x69   |  Returns a frame containing SD Card info.    |
|      |      |      |
|  Lepton Reboot    |   0x42   |   Reboots the lepton and returns a success/failure frame.  |
|  Lepton Reset    |   0x72   |   Resets the lepton and returns a success/failure frame.  |
|  Run FFC    |   0x46   |   Runs an FFC and returns a success/failure frame.  |
|      |      |      |
| Set ColorMap  |  0x43    |  Updates the colormap on the camera. Requires 1 byte of data (colormap id).    |
| Set Lepton Gain |  0x47    |  Updates the lepton gain. Requires 1 byte of data (gain type).    |
| Set Spotmeter Position |  0x70    |  Updates the position of the spotmeter on the camera. Requires 2 bytes of data (x and y).    |

#### NOTE:
**For more details about the communication framing refer to Connection.h and Connection.cpp in firmware folder.**

## Authors

- [Che Ahmed](https://github.com/CheAhMeD)


## Badges

[![AGPL License](https://img.shields.io/badge/license-GPL%20V3.0-blue.svg)](http://www.gnu.org/licenses/gpl-3.0)

