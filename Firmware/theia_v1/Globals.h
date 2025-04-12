/*
 * Theia Globals: Global definitions
 *
 * Copyright 2025 Che AhMeD
 *
 * This file is part of Theia.
 *
 * Theia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Theia.  If not, see <https://www.gnu.org/licenses/>.
 *
 */
#ifndef _GLOBALS_H
#define _GLOBALS_H

//#################################//
//        Pin Declarations         //
//#################################//
// I2C Pins: (Shared between the touch controller and Lepton)
#define BRD_I2C_SDA 38
#define BRD_I2C_SCL 39
// microSD Pins: (4-bit Mode)
#define SD_MMC_CLK  4
#define SD_MMC_CMD  15
#define SD_MMC_D0   3
#define SD_MMC_D1   5
#define SD_MMC_D2   7
#define SD_MMC_D3   6
// WS2812B Pins:
#define LED_DIN             16
// Flir Lepton VoSPI Pins:
#define LEPTON_VoSPI_MOSI   -1  // not used
#define LEPTON_VoSPI_MISO   41
#define LEPTON_VoSPI_SCK    42
#define LEPTON_VoSPI_CS     1
// Flir Lepton Control Pins:
#define LEPTON_RST_PIN      40
#define LEPTON_VSYNC_PIN    45
// TFT Control Pins
#define TFT_WR  18
#define TFT_RD  48
#define TFT_RS  17
// TFT 8 bit RGB
#define TFT_D0  9
#define TFT_D1  10
#define TFT_D2  11
#define TFT_D3  12
#define TFT_D4  13
#define TFT_D5  14
#define TFT_D6  21
#define TFT_D7  47
// TFT screen extra Pins:
#define TFT_CS   46
#define TFT_BKL  8
// Tocuh Controller IRQ
#define TC_IRQ_PIN 2
#define DEFAULT_NS2009_ADDR 0x48 //10010000

//#################################//
//         GUI Variables           //
//#################################//
// TFT Configs
#define TFT_SCREEN_WIDTH        320
#define TFT_SCREEN_HEIGHT       480
#define TFT_SCREEN_ROTATION     1
// GUI 
#define GUI_LOGO_POS_X          40
#define GUI_LOGO_POS_Y          160
#define GUI_LOGO_SIZE           160
#define GUI_APP_NAME_POS_Y      220
#define GUI_APP_NAME_POS_X      180
#define GUI_BOOT_LOG_POS_Y      280
#define GUI_BOOT_LOG_POS_X      20

// Current Colormap canvas
#define GUI_CMAP_X                19
#define GUI_CMAP_Y                32
#define GUI_CMAP_CANVAS_WIDTH     22
#define GUI_CMAP_CANVAS_HEIGHT    256
#define GUI_CMAP_PALETTE_X1       22
#define GUI_CMAP_PALETTE_X2       41
// Available palettes
#define GUI_CMAP_PALETTE_COUNT    12

// Camera feed (Where to display lepton image)
#define GUI_LEPTON_IMAGE_X         60
#define GUI_LEPTON_IMAGE_Y         40
#define GUI_LEPTON_IMAGE_WIDTH     320
#define GUI_LEPTON_IMAGE_HEIGHT    240

// Browse Screen
#define GUI_BROWSE_IMAGE_X         36
#define GUI_BROWSE_IMAGE_Y         64
#define GUI_BROWSE_IMAGE_WIDTH     320
#define GUI_BROWSE_IMAGE_HEIGHT    240

//Debug Info
#define GUI_DBG_INFO_MAX_CHARS     1024

// Lepton camera
#define LEP_3_5_SHUTTER             1    // FLIR Lepton 3.5 Shuttered
#define LEP_NOT_SUPPORTED           2    // Anything else is not supported
#define LEP_VOSPI_FRAME_SIZE        164  // Lepton Frame Size
#define LEP_VOSPI_PAK_PER_SEG       60   // Lepton Image packets per segment
#define LEP_IMAGE_X                 160  // Lepton Image Width
#define LEP_IMAGE_Y                 120  // Lepton Image Height
#define LEP_IMAGE_RESIZE_FACTOR     2    // Lepton Image resize factor (2x2 resized)
// NOTE: Lower wait period results in lower valid frames per second and more sync losses
// Tests: 1 ms => 2-3 FPS (mostly 2)
//        2 ms => 2-5 FPS (mostly 3)
//        3 ms => 2-5 FPS (mostly 4/5)
//        4 ms => 3-5 FPS (mostly 4/5)
//        5 ms => 4-5 FPS (evenly 4/5)
//        6 ms => 4-5 FPS (evenly 4/5)
//        7 ms => 4-5 FPS (evenly 4/5)
//        8 ms => 4-5 FPS (mostly 4)
//       10 ms => 2   FPS
//       20 ms => 2-3 FPS (mostly 3)
// Buffer update period (display time) is around 164ms and capture time (with no sync losses)
// is around 60 ms which gives at best 4.5 FPS (234ms)
#define LEP_IMAGE_CAPTURE_PERIOD_MS 7   // wait period after frame capture and display in ms

// Use Debug Serial Port
#define USE_DEBUG_SERIAL 0

//EEPROM (Persistant Storage) 
#define PS_RAM_SIZE                   8    // Number of bytes
#define PS_RAM_IMG_INDEX_ADDR         0x00 // 2 bytes
#define PS_RAM_PALETTE_NUM_ADDR       0x02 // 1 byte
#define PS_RAM_LEP_GAIN_MODE_ADDR     0x03 // 1 byte
#define PS_RAM_LEP_TEMP_UNIT_ADDR     0x04 // 1 byte
#define PS_RAM_LEP_IMG_FORMAT_ADDR    0x05 // 1 byte

// Saved Image
#define SAVED_FILE_PATH_MAX_CHARS 128

// Storage 
#define STORAGE_LEPTON_DIRPATH "/Lepton"
#define STORAGE_THEIA_FILEPATH "/.theia"

#define THEIA_APP_VERSION "1.0.1b"
#define THEIA_HW_REVISION "1.0.0"

#endif
