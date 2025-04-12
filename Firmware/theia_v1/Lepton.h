/*
 * Lepton.h - Library for interfacing Flir Lepton 3.5 shuttered camera
 * using ESP32 HSPI (for VoSPI) and Wire (for CCI) libraries
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
#ifndef Lepton_h
#define Lepton_h

// Register map
#define LEP_CCI_DEVICE_ADDRESS        0x2A  // CCI Address
// CCI register locations
#define LEP_CCI_REG_STATUS            0x0002
#define LEP_CCI_REG_COMMAND           0x0004
#define LEP_CCI_REG_DATA_LENGTH       0x0006
#define LEP_CCI_REG_DATA_0            0x0008
#define LEP_CCI_REG_DATA_1            0x000A
#define LEP_CCI_REG_DATA_2            0x000C
#define LEP_CCI_REG_DATA_3            0x000E
#define LEP_CCI_REG_DATA_4            0x0010
#define LEP_CCI_REG_DATA_5            0x0012
#define LEP_CCI_REG_DATA_6            0x0014
#define LEP_CCI_REG_DATA_7            0x0016
#define LEP_CCI_REG_DATA_8            0x0018
#define LEP_CCI_REG_DATA_9            0x001A
#define LEP_CCI_REG_DATA_10           0x001C
#define LEP_CCI_REG_DATA_11           0x001E
#define LEP_CCI_REG_DATA_12           0x0020
#define LEP_CCI_REG_DATA_13           0x0022
#define LEP_CCI_REG_DATA_14           0x0024
#define LEP_CCI_REG_DATA_15           0x0026
#define LEP_CCI_BLOCK_BUF_0           0xF800
#define LEP_CCI_BLOCK_BUF_1           0xFC00

// Commands
#define LEP_CCI_CMD_SYS_RUN_PING      0x0202
#define LEP_CCI_CMD_SYS_GET_STATUS    0x0204
#define LEP_CCI_CMD_SYS_GET_UPTIME    0x020C
#define LEP_CCI_CMD_SYS_GET_AUX_TEMP  0x0210
#define LEP_CCI_CMD_SYS_GET_FPA_TEMP  0x0214
//#define LEP_CCI_CMD_SYS_GET_TELEMETRY_ENABLE_STATE 0x0218
//#define LEP_CCI_CMD_SYS_SET_TELEMETRY_ENABLE_STATE 0x0219
//#define LEP_CCI_CMD_SYS_GET_TELEMETRY_LOCATION 0x021C
//#define LEP_CCI_CMD_SYS_SET_TELEMETRY_LOCATION 0x021D
#define LEP_CCI_CMD_SYS_RUN_FFC       0x0242
#define LEP_CCI_CMD_SYS_GET_FFC       0x0244
#define LEP_CCI_CMD_SYS_SET_FFC_MODE  0x023D
#define LEP_CCI_CMD_SYS_GET_GAIN_MODE 0x0248
#define LEP_CCI_CMD_SYS_SET_GAIN_MODE 0x0249

// #define LEP_CCI_CMD_RAD_GET_RADIOMETRY_ENABLE_STATE 0x4E10
// #define LEP_CCI_CMD_RAD_SET_RADIOMETRY_ENABLE_STATE 0x4E11
// #define LEP_CCI_CMD_RAD_GET_RADIOMETRY_FLUX_LINEAR_PARAMS 0x4EBC
// #define LEP_CCI_CMD_RAD_SET_RADIOMETRY_FLUX_LINEAR_PARAMS 0x4EBD
#define LEP_CCI_CMD_RAD_GET_RADIOMETRY_TLINEAR_ENABLE_STATE 0x4EC0
#define LEP_CCI_CMD_RAD_SET_RADIOMETRY_TLINEAR_ENABLE_STATE 0x4EC1
// #define LEP_CCI_CMD_RAD_GET_RADIOMETRY_TLINEAR_AUTO_RES 0x4EC8
// #define LEP_CCI_CMD_RAD_SET_RADIOMETRY_TLINEAR_AUTO_RES 0x4EC9
// #define LEP_CCI_CMD_RAD_GET_RADIOMETRY_SPOT_ROI 0x4ECC
// #define LEP_CCI_CMD_RAD_SET_RADIOMETRY_SPOT_ROI 0x4ECD

#define LEP_CCI_CMD_AGC_GET_AGC_ENABLE_STATE 0x0100
#define LEP_CCI_CMD_AGC_SET_AGC_ENABLE_STATE 0x0101
//#define LEP_CCI_CMD_AGC_GET_CALC_ENABLE_STATE 0x0148
//#define LEP_CCI_CMD_AGC_SET_CALC_ENABLE_STATE 0x0149

#define LEP_CCI_CMD_OEM_RUN_REBOOT    0x4842
#define LEP_CCI_CMD_OEM_GET_GPIO_MODE 0x4854
#define LEP_CCI_CMD_OEM_SET_GPIO_MODE 0x4855
#define LEP_CCI_CMD_OEM_GET_PART_NUM  0x481C

// frame status type
typedef struct {
  uint32_t validFrameCounter;
  uint32_t syncErrorCounter;
} lepDiagnostics_t;

extern lepDiagnostics_t lepDiagnostics;

class Lepton {
public:
  Lepton(int vospiClk, int vospiCS, int vospiMiso, int rstPin, int vsyncPin);
  // hardware
  void init();
  void reset();
  bool isOn();
  // VoSPI
  void softSync();
  void captureImage(bool rotate180);
  void updateRamBuffer();
  // Storage
  bool saveBMPImage(const char *path);
  bool saveRAWImage(const char *path);
  // Serial transfer
  void transferImage();
  void transferStatus();
  // CCI
  bool isBusy();
  uint16_t readCCIRegister(uint16_t reg);
  void setCCIRegister(uint16_t reg);
  byte readDataRegister(byte* data, int len);
  byte writeDataRegister(byte* data, int len);
  byte executeCommand(uint16_t cmd);
  // helpers
  // Module: SYS
  bool runFFC();
  uint32_t getFFCStatus();
  void setFFCMode(bool mode);
  uint64_t getSysStatus();
  uint32_t getUpTime();
  uint32_t getAuxTemp();
  uint32_t getFpaTemp();
  void setGainMode(uint8_t mode);
  uint8_t getGainMode();
  // Module: OEM
  void reboot();
  void setGpioMode(bool vsync_enabled);
  uint8_t getGpioMode();
  uint8_t getPartNumber();
  // Module: AGC
  void setAGCState(bool enable);
  uint32_t getAGCState();
  // Module: RAD
  void setRADState(bool enable);
  uint32_t getRADState();
private:
  int _vospiClk;
  int _vospiCS;
  int _vospiMiso;
  int _rstPin;
  int _vsyncPin;
  void floatToBytes(uint8_t *farray, float val);
};

// 320 x 240 bitmap header
// pad size =  (4 - (image width * 3) % 4) % 4 = 0
// image bits size = image width * image height * 3 + image height * pad size = 230400 (0x38400)
// Size in bytes = image bits size + sizeof(bmp_file_header_160x120) = 230454 (0x38436)
static const byte bmp_file_header_320x240[54] = { 
  // file header
  'B', 'M',                   // Magic
  0x36, 0x84, 0x03, 0x00,     // Size in bytes = 0x38436
  0x00, 0x00,                 // app data
  0x00, 0x00,                 // app data
  0x36, 0x00, 0x00, 0x00,     // start of data offset (= sizeof(bmp_file_header_320x240))= 54 (0x36)
  // info header
  0x28, 0x00, 0x00, 0x00,     // info hd size = 40 (0x28)
  0x40, 0x01, 0x00, 0x00,     // width = 320 (0x140)
  0xF0, 0x00, 0x00, 0x00,     // height = 240 (0xF0)
  0x01, 0x00,                 // number color planes
  0x18, 0x00,                 // bits per pixel (24 bbp)
  0x00, 0x00, 0x00, 0x00,     // compression is none 
  0x00, 0x84, 0x03, 0x00,     // image bits size = 230400 (0x38400)
  0x13, 0x0B, 0x00, 0x00,     // horz resoluition in pixel / m
  0x13, 0x0B, 0x00, 0x00,     // vert resolutions (0x03C3 = 96 dpi, 0x0B13 = 72 dpi)
  0x00, 0x00, 0x00, 0x00,     // #colors in pallete
  0x00, 0x00, 0x00, 0x00,     // #important colors
};

static const byte bmp_file_pad[3] = { 0, 0, 0};

#endif