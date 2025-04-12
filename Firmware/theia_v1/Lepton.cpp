#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <FS.h>
#include "Lepton.h"
#include "Globals.h"
#include "vars.h"
#include "screens.h"
#include "Storage.h"
#include "CRC8.h"
#include "CRC.h"
#include "esp_log.h"

static const char* TAG = "Lepton";

// CRC For Serial transfer
CRC8 lepCRC;
// VoSPI Frequency
static const int vospiClkFreq = 20000000;  // 20 MHz, min is 2.2, max 20 for the Lepton 3.5 module.
// SPI bus which will implement the Lepton VoSPI
SPIClass* hspi = NULL;
// VoSPI data buffers
uint16_t lepton_frame_packet[LEP_VOSPI_FRAME_SIZE / 2];
uint16_t lepton_frame_segment[60][LEP_VOSPI_FRAME_SIZE / 2];  // 60 packets per segment
// Image buffer
static uint16_t lepton_image_buf[LEP_IMAGE_X][LEP_IMAGE_Y];


double lep_max_temp = 0;
double lep_min_temp = 0;
double lep_min, lep_max;
double lep_pointer_temp = 0;
char lep_min_temp_chr[10];
char lep_max_temp_chr[10];
char lep_pointer_temp_chr[10];
bool agc_enabled = false;
lepDiagnostics_t lepDiagnostics;

Lepton::Lepton(int vospiClk, int vospiCS, int vospiMiso, int rstPin, int vsyncPin) {
  _vospiClk = vospiClk;
  _vospiCS = vospiCS;
  _vospiMiso = vospiMiso;
  _rstPin = rstPin;
  _vsyncPin = vsyncPin;  // Not used for now
}

/***********/
/*  VoSPI  */
/***********/
/* Initialize Lepton VoSPI with the selected pins */
void Lepton::init() {
  //ESP_LOGI(TAG, "Starting VoSPI over HSPI...");
  hspi = new SPIClass(HSPI);
  hspi->begin(_vospiClk, _vospiMiso, -1, _vospiCS);
  hspi->setDataMode(SPI_MODE3);
  pinMode(_vospiCS, OUTPUT);
  digitalWrite(_vospiCS, HIGH);
  pinMode(_rstPin, OUTPUT);
  digitalWrite(_rstPin, LOW);
  pinMode(_vsyncPin, INPUT);
  // Init lepton image buffer
  gui_lep_canvas_buffer = (uint16_t*)heap_caps_malloc(GUI_LEPTON_IMAGE_WIDTH * GUI_LEPTON_IMAGE_HEIGHT * 2, MALLOC_CAP_SPIRAM);
  if (gui_lep_canvas_buffer == NULL) {
    ESP_LOGE(TAG, "malloc GUI canvas buffer failed");
  }
  lepDiagnostics.syncErrorCounter = 0;
  lepDiagnostics.validFrameCounter = 0;
  // Perform an FFC
  runFFC();
}

/* Reset the Lepton Camera */
void Lepton::reset() {
  //ESP_LOGI(TAG, "Resetting the Lepton Camera...");
  show_settings_screen_msgbox("Resetting Camera...");
  digitalWrite(_rstPin, HIGH);
  delay(5);
  digitalWrite(_rstPin, LOW);
  // 950 ms required after deasserting RST pin
  // 2.1.1.1 Start-up command sequence (flir software IDD) 110-0144-04 Rev 303
  delay(1000);
  hide_settings_screen_msgbox();
}

bool Lepton::isOn() {
  Wire.beginTransmission(LEP_CCI_DEVICE_ADDRESS);
  uint8_t error = Wire.endTransmission();
  return (error == 0);
}

/* Syncing VoSPI */
// according to 4.2.3.3.1 Establishing/Re-Establishing Sync (lepton datasheet)
void Lepton::softSync() {
  //ESP_LOGI(TAG, "Establishing Sync");
  int i;
  int data = 0x0f00;

  digitalWrite(_vospiCS, HIGH);
  delay(300);  // Waits for the required time indicated by datasheet, ensures a timeout of VoSPI I/F

  while ((data & (0x0f00)) == 0x0f00) {  // I changed this because I don't think it was right

    // begin checking the first returned data packet, should be a discard packet

    // Start of VoSPI transfer for first packet, reading for discard packet
    digitalWrite(_vospiCS, LOW);
    hspi->beginTransaction(SPISettings(vospiClkFreq, MSBFIRST, SPI_MODE3));  // these arent included in others' code
    data = hspi->transfer(0x00) << 8;
    data |= hspi->transfer(0x00);
    // hspi->endTransaction();
    digitalWrite(_vospiCS, HIGH);

    // Process and discard the remaining data in the packet
    for (i = 0; i < ((LEP_VOSPI_FRAME_SIZE - 2) / 2); i++) {
      digitalWrite(_vospiCS, LOW);
      hspi->transfer(0x00);  // unused garbage data
      hspi->transfer(0x00);  // unused garbage data
      digitalWrite(_vospiCS, HIGH);
    }

    hspi->endTransaction();
  }
}

/* capturing a VoSPI frame (4 segments) */
void Lepton::captureImage(bool rotate180) {

  hspi->setDataMode(SPI_MODE3);
  hspi->setFrequency(vospiClkFreq);

  bool collectedSegments[4];
  collectedSegments[0] = false;
  collectedSegments[1] = false;
  collectedSegments[2] = false;
  collectedSegments[3] = false;
  uint8_t lastFoundSegment = 0;
  uint8_t segmentsRead = 0;

  while (!collectedSegments[0] | !collectedSegments[1] | !collectedSegments[2] | !collectedSegments[3]) {

    // Get 60 valid packets per segment
    for (int packetNumber = 0; packetNumber < 60; packetNumber++) {
      do {
        digitalWrite(_vospiCS, LOW);
        hspi->beginTransaction(SPISettings(vospiClkFreq, MSBFIRST, SPI_MODE3));
        byte dummyBuffer[164];
        hspi->transfer(dummyBuffer, sizeof(dummyBuffer));

        for (int i = 0; i < 164l; i += 2) {
          lepton_frame_packet[i / 2] = dummyBuffer[i] << 8 | dummyBuffer[i + 1];
        }

        hspi->endTransaction();
        digitalWrite(_vospiCS, HIGH);
      } while (((lepton_frame_packet[0] & 0x0f00) >> 8 == 0x0f));  // wait until a non-discard packet has been found

      // Load the packet into the segment
      for (int i = 0; i < LEP_VOSPI_FRAME_SIZE / 2; i++) {
        lepton_frame_segment[packetNumber][i] = lepton_frame_packet[i];
      }
    }

    // Load the collected segment into the image after it's all been captured
    int segmentNumber = (lepton_frame_segment[20][0] >> 12) & 0b0111;  // This should give the segment number which is held at packet 20. The transmitted packets start at number 0.

    if (segmentNumber != 0) {  // if the segment is number 0, ignore the segment
      // Makes sure that we get the segments in the right order
      if (segmentNumber == 2) {
        if (!collectedSegments[0]) {  // We haven't found the first segment
          lastFoundSegment = 0;
          segmentsRead = 0;
          lepDiagnostics.syncErrorCounter++;
          softSync();
          break;
        }
      } else if (segmentNumber == 3) {
        if (!collectedSegments[0] && !collectedSegments[1]) {
          lastFoundSegment = 0;
          segmentsRead = 0;
          lepDiagnostics.syncErrorCounter++;
          softSync();
          break;
        }
      } else if (segmentNumber == 4) {
        if (!collectedSegments[0] && !collectedSegments[1] && !collectedSegments[2]) {
          lastFoundSegment = 0;
          segmentsRead = 0;
          lepDiagnostics.syncErrorCounter++;
          softSync();
          break;
        }
      }


      segmentsRead++;
      if (segmentsRead > 4) {  // This means that we're probably out of sync for some reason
        collectedSegments[0] = false;
        collectedSegments[1] = false;
        collectedSegments[2] = false;
        collectedSegments[3] = false;
        lastFoundSegment = 0;
        segmentsRead = 0;
        lepDiagnostics.syncErrorCounter++;
        softSync();
        break;
      }

      if (segmentNumber > 4) {
        lepDiagnostics.syncErrorCounter++;
        softSync();
        break;
      }
      collectedSegments[segmentNumber - 1] = true;

      // copy the frame segments to the image buffer
      // camera is upside down
      // using rotate180 to store rotated image
      for (int packetNumber = 0; packetNumber < 60; packetNumber++) {
        for (int px = 0; px < 80; px++) {
          if (packetNumber % 2) {
            // If the packet number is odd, put it on the right side of the image
            // Placement starts at X = 80 px
            if (rotate180) {
              lepton_image_buf[79 - px][(LEP_IMAGE_Y - 1) - ((segmentNumber - 1) * 30 + (int)(packetNumber / 2))] = lepton_frame_segment[packetNumber][px + 2];
            } else {
              lepton_image_buf[80 + px][((segmentNumber - 1) * 30 + (int)(packetNumber / 2))] = lepton_frame_segment[packetNumber][px + 2];
            }
          } else {
            // Otherwise put it on the left side
            // Placement starts at X = 0 px
            if (rotate180) {
              lepton_image_buf[80 + 79 - px][(LEP_IMAGE_Y - 1) - ((segmentNumber - 1) * 30 + (int)(packetNumber / 2))] = lepton_frame_segment[packetNumber][px + 2];
            } else {
              lepton_image_buf[px][((segmentNumber - 1) * 30 + (int)(packetNumber / 2))] = lepton_frame_segment[packetNumber][px + 2];
            }
          }
        }
      }
    }
  }
  //ESP_LOGI(TAG, "(%d) --> Capture Done", esp_log_timestamp());
  lepDiagnostics.validFrameCounter++;
  hspi->setDataMode(SPI_MODE3);
  hspi->setFrequency(vospiClkFreq);
}

/* Transfer Raw Image over USB CDC */
void Lepton::transferImage(void) {
  const int packet_size = 256;  // Send in small packets to avoid buffer overflow
  uint8_t* data = (uint8_t*)lepton_image_buf;  // Treat as byte array
  size_t image_size = LEP_IMAGE_X * LEP_IMAGE_Y * 2;
  for (int i = 0; i < image_size; i+=packet_size) {
    Serial.write(&data[i], packet_size);
    lepCRC.add(&data[i], packet_size);
    delay(5);
  }
  Serial.write(lepCRC.calc());
  Serial.flush();
}

/* Transfer Lepton Status over USB CDC */
void Lepton::transferStatus(void) {
  uint64_t sysStatus = getSysStatus();
  uint32_t upTime = getUpTime();
  uint32_t auxTemp = getAuxTemp();
  uint32_t fpaTemp = getFpaTemp();
  uint8_t gainMode = getGainMode();
  uint8_t gpioMode = getGpioMode();
  uint8_t pn = getPartNumber();
  Serial.write(pn);
  lepCRC.add(pn);
  Serial.write((uint8_t*)&sysStatus, sizeof(sysStatus));
  lepCRC.add((uint8_t *)&sysStatus, sizeof(sysStatus));
  Serial.write((uint8_t*)&upTime, sizeof(upTime));
  lepCRC.add((uint8_t *)&upTime, sizeof(upTime));
  Serial.write((uint8_t*)&auxTemp, sizeof(auxTemp));
  lepCRC.add((uint8_t *)&auxTemp, sizeof(auxTemp));
  Serial.write((uint8_t*)&fpaTemp, sizeof(fpaTemp));
  lepCRC.add((uint8_t *)&fpaTemp, sizeof(fpaTemp));
  Serial.write(gainMode);
  lepCRC.add(gainMode);
  Serial.write(gpioMode);
  lepCRC.add(gpioMode);
  // CRC
  Serial.write(lepCRC.calc());
}


/* Update LVGL PSRAM Buffer */
void Lepton::updateRamBuffer() {
  lep_max = lep_min = lepton_image_buf[0][0];
  // calculate pointer relative position 
  // NOTE: this is not accurate but close enough...
  int px = abs((int)lv_obj_get_x(objects.pointer) - GUI_LEPTON_IMAGE_X) / 2 + 2;
  int py = abs((int)lv_obj_get_y(objects.pointer) - GUI_LEPTON_IMAGE_Y) / 2 + 3;
  // get Min and Max temperatures
  for (int i = 0; i < LEP_IMAGE_X; i++) {
    for (int j = 0; j < LEP_IMAGE_Y; j++) {
      if (lepton_image_buf[i][j] > lep_max) {
        lep_max = lepton_image_buf[i][j];
      }
      if (lepton_image_buf[i][j] < lep_min) {
        lep_min = lepton_image_buf[i][j];
      }
    }
  }
  // Calculate the max, min and pointer temperatures
  //if Format is C
  if (get_var_cur_temp_format() == 0) {
    lep_max_temp = (lep_max / 100) - 273.15;
    lep_min_temp = (lep_min / 100) - 273.15;
    lep_pointer_temp = (lepton_image_buf[px][py] / 100) - 273.15;
    // update vars on the screen
    dtostrf(lep_max_temp, 5, 2, lep_max_temp_chr);
    dtostrf(lep_min_temp, 5, 2, lep_min_temp_chr);
    dtostrf(lep_pointer_temp, 5, 2, lep_pointer_temp_chr);
    strcat(lep_max_temp_chr, "°C");
    strcat(lep_min_temp_chr, "°C");
    strcat(lep_pointer_temp_chr, "°C");
    //if Format is F
  } else if (get_var_cur_temp_format() == 1) {
    lep_max_temp = ((lep_max / 100) - 273.15) * (9 / 5) + 32;
    lep_min_temp = ((lep_min / 100) - 273.15) * (9 / 5) + 32;
    lep_pointer_temp = ((lepton_image_buf[px][py] / 100) - 273.15) * (9 / 5) + 32;
    // update vars on the screen
    dtostrf(lep_max_temp, 5, 2, lep_max_temp_chr);
    dtostrf(lep_min_temp, 5, 2, lep_min_temp_chr);
    dtostrf(lep_pointer_temp, 5, 2, lep_pointer_temp_chr);
    strcat(lep_max_temp_chr, "°F");
    strcat(lep_min_temp_chr, "°F");
    strcat(lep_pointer_temp_chr, "°F");
    //else show Kelvin
  } else {
    lep_max_temp = (lep_max / 100);
    lep_min_temp = (lep_min / 100);
    lep_pointer_temp = (lepton_image_buf[px][py] / 100);
    // update vars on the screen
    dtostrf(lep_max_temp, 5, 2, lep_max_temp_chr);
    dtostrf(lep_min_temp, 5, 2, lep_min_temp_chr);
    dtostrf(lep_pointer_temp, 5, 2, lep_pointer_temp_chr);
    strcat(lep_max_temp_chr, "K");
    strcat(lep_min_temp_chr, "K");
    strcat(lep_pointer_temp_chr, "K");
  }

  set_var_max_temp(lep_max_temp_chr);
  set_var_min_temp(lep_min_temp_chr);
  set_var_cursor_temp(lep_pointer_temp_chr);

  // Fill the PSRAM buffer
  uint16_t* ptr = gui_lep_canvas_buffer;
  int x = 0;  // lepton_image_buf x counter
  int y = 0;  // lepton_image_buf y counter
  int z = 0;  // lepton_image_buf z counter (to resize the image vertically)
  while (ptr < (gui_lep_canvas_buffer + (GUI_LEPTON_IMAGE_WIDTH * GUI_LEPTON_IMAGE_HEIGHT))) {
    double val = (lepton_image_buf[x][y] - lep_min) / (lep_max - lep_min); 
    uint16_t color = colormap[(int)(255 * val)];
    x++;
    if (x > LEP_IMAGE_X - 1) {
      x = 0;
      z++;
    }
    if (z > LEP_IMAGE_RESIZE_FACTOR - 1) {
      z = 0;
      y++;
    }
    if (y > LEP_IMAGE_Y - 1) {
      y = 0;
    }
    // Draw the pixel twice (to resize the image horizontally)
    for (int i = 0; i < LEP_IMAGE_RESIZE_FACTOR; i++) *ptr++ = color;
  }

  // Finally invalidate the object to force it to redraw from the buffer
  invalidate_lepton_image();
}


/* Write a BMP file to SD Card */
bool Lepton::saveBMPImage(const char* path) {
  //ESP_LOGI(TAG, "Saving Lepton BMP Image...");
  // Open File
  File bmpFile = Theia_SDCard.open(path, FILE_WRITE);

  if (!bmpFile) {
    ESP_LOGE(TAG, "Couldn't Save Lepton BMP Image...");
    return false;
  } else {
    // Write the bmp header
    bmpFile.write((byte*)bmp_file_header_320x240, sizeof(bmp_file_header_320x240));
    // Write the bmp data
    byte pixel[3];

    for (int y = LEP_IMAGE_Y - 1; y >= 0; --y) {
      // write the line LEP_IMAGE_RESIZE_FACTOR (2) times
      for (int z = 0; z < LEP_IMAGE_RESIZE_FACTOR; ++z) {
        for (int x = 0; x < LEP_IMAGE_X; ++x) {

          double val = (lepton_image_buf[x][y] - lep_min) / (lep_max - lep_min);
          uint16_t color = colormap[(int)(255 * val)];
          // R8 = ( R5 * 527 + 23 ) >> 6;
          // G8 = ( G6 * 259 + 33 ) >> 6;
          // B8 = ( B5 * 527 + 23 ) >> 6;
          pixel[0] = (((color & 0x1F) * 527) + 23) >> 6;
          pixel[1] = ((((color >> 5) & 0x3F) * 259) + 33) >> 6;
          pixel[2] = ((((color >> 11) & 0x1F) * 527) + 23) >> 6;
          // write the pixel twice to resize
          bmpFile.write((byte*)pixel, 3);
          bmpFile.write((byte*)pixel, 3);
        }
        // no need to pad since size is a multiple of 4
        //bmpFile.write((byte*)bmp_file_pad, 0);
      }
    }
    bmpFile.close();
  }
  return true;
}

/*
  Save RAW file:
  File extension : .timg
  File Format:  colormap id (1 byte) \n
                max temp (uint8_t array) \n
                min temp (uint8_t array) \n
                160x120 image data (19200*2 bytes)
*/
bool Lepton::saveRAWImage(const char* path) {
  // ESP_LOGI(TAG, "Saving Lepton RAW Image...");
  File rawFile = Theia_SDCard.open(path, FILE_WRITE);
  if (!rawFile) {
    ESP_LOGE(TAG, "Couldn't Save Lepton RAW Image...");
    return false;
  }
  // Write colormap id and min max values
  rawFile.write(get_var_palette_num());
  rawFile.write('\n');
  uint8_t tarray[4];
  floatToBytes(tarray, lep_max);
  rawFile.write((byte*)tarray, sizeof(tarray));
  rawFile.write('\n');
  floatToBytes(tarray, lep_min);
  rawFile.write((byte*)tarray, sizeof(tarray));
  rawFile.write('\n');
  // write the image data
  for (int y = 0; y < LEP_IMAGE_Y; y++) {
    for (int x = 0; x < LEP_IMAGE_X; x++) {
      uint16_t pixel = lepton_image_buf[x][y];
      rawFile.write((uint8_t*)&pixel, sizeof(pixel));
    }
  }
  rawFile.close();
  // ESP_LOGI(TAG, "Lepton RAW Image Saved...");
  return true;
}


/* Converts a float to four bytes */
void Lepton::floatToBytes(uint8_t* farray, float val) {
  union {
    float f;
    unsigned long ul;
  } u;
  u.f = val;
  farray[0] = u.ul & 0x00FF;
  farray[1] = (u.ul & 0xFF00) >> 8;
  farray[2] = (u.ul & 0xFF0000) >> 16;
  farray[3] = (u.ul & 0xFF000000) >> 24;
}

/***********/
/* CCI/TWI */
/***********/
/* Check if Lepton is busy */
bool Lepton::isBusy() {
  bool busy = true;
  uint16_t status = readCCIRegister(LEP_CCI_REG_STATUS);
  if (!(status & 0b100) & (status & 0b1)) {
    ESP_LOGW(TAG, "Camera not started yet...");
    delay(1000);
  } else {
    busy = false;
  }
  return busy;
}

/* Read a CCI register */
uint16_t Lepton::readCCIRegister(uint16_t reg) {
  uint16_t reading = 0;
  setCCIRegister(reg);

  Wire.requestFrom(LEP_CCI_DEVICE_ADDRESS, 2);

  reading = Wire.read();   // receive high byte (overwrites previous reading)
  reading = reading << 8;  // shift high byte to be high 8 bits

  reading |= Wire.read();  // receive low byte as lower 8 bits
  return reading;
}

void Lepton::setCCIRegister(uint16_t reg) {
  Wire.beginTransmission(LEP_CCI_DEVICE_ADDRESS);  // transmit to device #4
  Wire.write((reg >> 8) & 0xff);
  Wire.write(reg & 0xff);  // sends one byte
  Wire.endTransmission();  // stop transmitting
}

/*
 First reads the DATA Length Register (0x0006)
 Then reads the acutal DATA Registers:
 DATA 0 Register, DATA 1 Register etc.

 If the request length is smaller that the DATA Length it returns -1
 */
byte Lepton::readDataRegister(byte* data, int len) {
  int data_length_recv;
  byte ret;
  // Wait for lepton ready
  while (isBusy())
    ;

  // Read the data length (should be 4)
  data_length_recv = readCCIRegister(LEP_CCI_REG_DATA_LENGTH);

  if (data_length_recv < len) {
    return -1;
  }

  Wire.requestFrom(LEP_CCI_DEVICE_ADDRESS, len);
  ret = Wire.readBytes(data, len);
  Wire.endTransmission();
  return ret;
}

/*
 First writes the actual DATA Registers (0x0008).
 Then writes the DATA Length Register (0x0006)
 */
byte Lepton::writeDataRegister(byte* data, int len) {
  // Wait for lepton ready
  while (isBusy())
    ;

  Wire.beginTransmission(LEP_CCI_DEVICE_ADDRESS);
  // CCI/TWI Data Registers is at 0x0008
  Wire.write((LEP_CCI_REG_DATA_0 >> 8) & 0xff);
  Wire.write(LEP_CCI_REG_DATA_0 & 0xff);
  for (int i = 0; i < len; i++) {
    Wire.write(data[i]);
  }
  Wire.endTransmission();

  // CCI/TWI Data Length Register is at 0x0006
  Wire.beginTransmission(LEP_CCI_DEVICE_ADDRESS);
  Wire.write((LEP_CCI_REG_DATA_LENGTH >> 8) & 0xff);
  Wire.write(LEP_CCI_REG_DATA_LENGTH & 0xff);
  //Data length bytes
  Wire.write((len >> 8) & 0xFF);
  Wire.write(len & 0xFF);
  return Wire.endTransmission();
}

/*
 Write the command words (16-bit) via CCI
 */
byte Lepton::executeCommand(uint16_t cmd) {
  // Wait for lepton ready
  while (isBusy())
    ;

  Wire.beginTransmission(LEP_CCI_DEVICE_ADDRESS);
  Wire.write((LEP_CCI_REG_COMMAND >> 8) & 0xff);
  Wire.write(LEP_CCI_REG_COMMAND & 0xff);
  Wire.write((cmd >> 8) & 0xff);
  Wire.write(cmd & 0xff);
  return Wire.endTransmission();
}

/* Trigger a flat-field-correction on the Lepton */
bool Lepton::runFFC() {
  // SYS Run FFC Normalization
  // 0x0200 (SDK Module ID) + 0x40 (SDK Command ID) + 0x2 (RUN operation) + 0x0000 (Protection Bit) = 0x0242
  byte error = executeCommand(LEP_CCI_CMD_SYS_RUN_FFC);
  return error;
}

/* Get SYS FFC Status */
uint32_t Lepton::getFFCStatus() {
  // Request 2 bytes from data register
  byte package[1] = { 2 };
  writeDataRegister(package, sizeof(package));
  // SYS FFC Mode Control Command
  // 0x0200 (SDK Module ID) + 0x44 (SDK Command ID) + 0x0 (GET operation) + 0x0000 (Protection Bit) = 0x0244
  executeCommand(LEP_CCI_CMD_SYS_GET_FFC);
  uint16_t ls_word = readCCIRegister(LEP_CCI_REG_DATA_0);
  uint16_t ms_word = readCCIRegister(LEP_CCI_REG_DATA_1);
  return ms_word << 16 | ls_word;
}

/* Set the shutter operation to auto (mode = true) | manual (mode = false)*/
void Lepton::setFFCMode(bool mode) {
  // FFC is automatic by default
  if (mode)
    return;
  // set manual FFC
  //Contains the standard values for the FFC mode
  byte package[32] = { mode, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 224,
                       147, 4, 0, 0, 0, 0, 0, 44, 1, 52, 0 };

  writeDataRegister(package, sizeof(package));

  // SYS FFC Mode Control Command
  // 0x0200 (SDK Module ID) + 0x3C (SDK Command ID) + 0x1 (SET operation) + 0x0000 (Protection Bit) = 0x023D
  executeCommand(LEP_CCI_CMD_SYS_SET_FFC_MODE);
}

/* Get Lepton System Status */
uint64_t Lepton::getSysStatus() {
  // Request 4 bytes from data register
  byte package[1] = { 4 };
  writeDataRegister(package, sizeof(package));
  // SYS Status Command
  // 0x0200 (SDK Module ID) + 0x04 (SDK Command ID) + 0x0 (GET operation) + 0x0000 (Protection Bit) = 0x0204
  executeCommand(LEP_CCI_CMD_SYS_GET_STATUS);
  uint16_t ls_word = readCCIRegister(LEP_CCI_REG_DATA_0);
  uint16_t mword_1 = readCCIRegister(LEP_CCI_REG_DATA_1);
  uint16_t mword_2 = readCCIRegister(LEP_CCI_REG_DATA_2);
  uint16_t ms_word = readCCIRegister(LEP_CCI_REG_DATA_3);
  return ms_word << 48 | mword_2 << 32 | mword_1 << 16 | ls_word;
}

/* Get Lepton Camera Uptime */
uint32_t Lepton::getUpTime() {
  // Request 2 bytes from data register
  byte package[1] = { 2 };
  writeDataRegister(package, sizeof(package));
  // SYS FFC Mode Control Command
  // 0x0200 (SDK Module ID) + 0x0C (SDK Command ID) + 0x0 (GET operation) + 0x0000 (Protection Bit) = 0x020C
  executeCommand(LEP_CCI_CMD_SYS_GET_UPTIME);
  uint16_t ls_word = readCCIRegister(LEP_CCI_REG_DATA_0);
  uint16_t ms_word = readCCIRegister(LEP_CCI_REG_DATA_1);
  return ms_word << 16 | ls_word;
}

/**
 * Get the AUX (case) temperature in Kelvin x 100 (16-bit result).
 */
uint32_t Lepton::getAuxTemp() {
  // Request 2 bytes from data register
  byte package[1] = { 2 };
  writeDataRegister(package, sizeof(package));
  // SYS FFC Mode Control Command
  // 0x0200 (SDK Module ID) + 0x10 (SDK Command ID) + 0x0 (GET operation) + 0x0000 (Protection Bit) = 0x0210
  executeCommand(LEP_CCI_CMD_SYS_GET_AUX_TEMP);
  uint16_t ls_word = readCCIRegister(LEP_CCI_REG_DATA_0);
  uint16_t ms_word = readCCIRegister(LEP_CCI_REG_DATA_1);
  return ms_word << 16 | ls_word;
}

/**
 * Get the FPA (sensor) temperature in Kelvin x 100 (16-bit result).
 */
uint32_t Lepton::getFpaTemp() {
  // Request 2 bytes from data register
  byte package[1] = { 2 };
  writeDataRegister(package, sizeof(package));
  // SYS FFC Mode Control Command
  // 0x0200 (SDK Module ID) + 0x14 (SDK Command ID) + 0x0 (GET operation) + 0x0000 (Protection Bit) = 0x0214
  executeCommand(LEP_CCI_CMD_SYS_GET_FPA_TEMP);
  uint16_t ls_word = readCCIRegister(LEP_CCI_REG_DATA_0);
  uint16_t ms_word = readCCIRegister(LEP_CCI_REG_DATA_1);
  return ms_word << 16 | ls_word;
}

/*
 * Set the SYS Gain Mode
 * 0: high mode (default),
 * 1: low mode,
 * 2: auto mode
 * The measurement range for the Lepton 3.5 is (see datasheet for details):
 * High Gain Mode: 	-10 to +140 deg C
 * Low Gain Mode: 	-10 to +450 deg C
 *
 */
void Lepton::setGainMode(uint8_t mode) {
  if (mode > 2) {
    return;
  }
  //The enum value is the LSB of DATA0
  byte package[4] = { 0x00, mode, 0x00, 0x00 };
  writeDataRegister(package, sizeof(package));

  // Execute the SYS Gain Mode Set Command, so that the module applies the values
  // 0x0200 (SDK Module ID) + 0x48 (SDK Command ID) + 0x1 (SET operation) + 0x0000 (Protection Bit) = 0x0249.
  executeCommand(LEP_CCI_CMD_SYS_SET_GAIN_MODE);
}

/*
 * Returns the SYS Gain Mode
 * 0: high mode (default),
 * 1: low mode,
 * 2: auto mode
 * The measurement range for the Lepton 3.5 is (see datasheet for details):
 * High Gain Mode: 	-10 to +140 deg C
 * Low Gain Mode: 	-10 to +450 deg C
 *
 * Returns -1 if the gain mode could not be read
 */
uint8_t Lepton::getGainMode() {
  byte data[4];

  //SYS Gain Mode Get Command
  // 0x0200 (SDK Module ID) + 0x48 (SDK Command ID) + 0x0 (GET operation) + 0x0000 (Protection Bit) = 0x0248.
  executeCommand(LEP_CCI_CMD_SYS_GET_GAIN_MODE);
  readDataRegister(data, 4);

  //The enum value is the LSB of DATA0
  return data[1];
}

/* Run the OEM Reboot command */
void Lepton::reboot() {
  // OEM Run Camera Re-Boot
  // 0x0800 (SDK Module ID) + 0x42 (SDK Command ID) + 0x0 (RUN operation) + 0x4000 (Protection Bit) = 0x4842
  executeCommand(LEP_CCI_CMD_OEM_RUN_REBOOT);
  show_settings_screen_msgbox("Rebooting Camera...");
  // Sleep to allow camera to reboot and run FFC
  vTaskDelay(pdMS_TO_TICKS(6000));
  hide_settings_screen_msgbox();
}

void Lepton::setGpioMode(bool vsync_enabled) {
  byte gpio_mode = 0x00;
  if (vsync_enabled)
    gpio_mode = 0x05;

  //The enum value is the LSB of DATA0
  byte package[4] = { 0x00, gpio_mode, 0x00, 0x00 };

  writeDataRegister(package, sizeof(package));

  // OEM GPIO Mode Select Set Command
  // 0x0800 (SDK Module ID) + 0x54 (SDK Command ID) + 0x1 (SET operation) + 0x4000 (Protection Bit) = 0x4855.
  executeCommand(LEP_CCI_CMD_OEM_SET_GPIO_MODE);
}

/* Returns the GPIO Mode */
uint8_t Lepton::getGpioMode() {
  byte data[4];

  //SYS Gain Mode Get Command
  // 0x0200 (SDK Module ID) + 0x48 (SDK Command ID) + 0x0 (GET operation) + 0x0000 (Protection Bit) = 0x0248.
  executeCommand(LEP_CCI_CMD_OEM_GET_GPIO_MODE);
  readDataRegister(data, 4);

  //The enum value is the LSB of DATA0
  return data[0];
}

/* Returns the Part Number
 * 1: lepton 3.5 Shutter,
 * 2: Unsupported Camera
 */
uint8_t Lepton::getPartNumber() {
  char pn[33];
  // OEM FLIR Systems Part Number
  // 0x0800 (SDK Module ID) + 0x1C (SDK Command ID) + 0x0 (GET operation) + 0x4000 (Protection Bit) = 0x481C
  byte error = executeCommand(LEP_CCI_CMD_OEM_GET_PART_NUM);
  //Lepton I2C error
  if (error != 0) {
    ESP_LOGE(TAG, "Failed to retreive PartNumber...");
    return 0;
  }
  readDataRegister((byte*)pn, 32);

  if ((strstr(pn, "05-070170") != NULL) || (strstr(pn, "05-070850") != NULL)) {
    return LEP_3_5_SHUTTER;
  } else {
    return LEP_NOT_SUPPORTED;
  }
}

/* Not Supported Yet */
/*
 * Set the AGC enable state.
 */
void Lepton::setAGCState(bool enable) {
  uint8_t agcState = enable;
  // Set AGC state
  byte package[1] = { agcState };
  writeDataRegister(package, sizeof(package));
  // AGC Enable Set Control Command
  // 0x0100 (SDK Module ID) + 0x00 (SDK Command ID) + 0x1 (SET operation) + 0x0000 (Protection Bit) = 0x020C
  executeCommand(LEP_CCI_CMD_AGC_SET_AGC_ENABLE_STATE);
  agc_enabled = enable;
}

/*
 * Get the AGC enable state.
 */
uint32_t Lepton::getAGCState() {
  // Request 2 bytes from data register
  byte package[1] = { 2 };
  writeDataRegister(package, sizeof(package));
  //  AGC Enable Get Control Command
  // 0x0100 (SDK Module ID) + 0x00 (SDK Command ID) + 0x0 (GET operation) + 0x0000 (Protection Bit) = 0x0214
  executeCommand(LEP_CCI_CMD_AGC_GET_AGC_ENABLE_STATE);
  uint16_t ls_word = readCCIRegister(LEP_CCI_REG_DATA_0);
  uint16_t ms_word = readCCIRegister(LEP_CCI_REG_DATA_1);
  return ms_word << 16 | ls_word;
}

/*
 * Set the RAD enable state.
 */
void Lepton::setRADState(bool enable) {
  uint8_t radState = enable;
  // Set radiometry tlinear state
  byte package[1] = { radState };
  writeDataRegister(package, sizeof(package));
  // RAD TLINEAR Enable Set Control Command
  // 0x0E00 (SDK Module ID) + 0xC0 (SDK Command ID) + 0x1 (SET operation) + 0x4000 (Protection Bit) = 0x4EC1
  executeCommand(LEP_CCI_CMD_RAD_SET_RADIOMETRY_TLINEAR_ENABLE_STATE);
}

/*
 * Get the RAD enable state.
 */
uint32_t Lepton::getRADState() {
  // Request 2 bytes from data register
  byte package[1] = { 2 };
  writeDataRegister(package, sizeof(package));
  //  AGC Enable Get Control Command
  // 0x0E00 (SDK Module ID) + 0xC0 (SDK Command ID) + 0x0 (GET operation) + 0x4000 (Protection Bit) = 0x4EC0
  executeCommand(LEP_CCI_CMD_RAD_GET_RADIOMETRY_TLINEAR_ENABLE_STATE);
  uint16_t ls_word = readCCIRegister(LEP_CCI_REG_DATA_0);
  uint16_t ms_word = readCCIRegister(LEP_CCI_REG_DATA_1);
  return ms_word << 16 | ls_word;
}