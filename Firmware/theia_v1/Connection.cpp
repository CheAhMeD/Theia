#include <Arduino.h>
#include "Connection.h"
#include "Lepton.h"
#include "Globals.h"
#include "vars.h"
#include "actions.h"
#include "Storage.h"
#include "CRC8.h"
#include "CRC.h"

CRC8 crc;

bool commandReceived() {
  return Serial.available() > 0;
}

void handleCommand() {
  //Read received data from Serial Port
  uint8_t rcv = Serial.read();

  //Decide what to do
  switch (rcv) {
    // Send lepton raw data
    case CMD_GET_LEP_RAWDATA:
      transferLeptonRawData();
      break;
    // Send lepton sync errors and valid frames
    case CMD_GET_LEP_DIAGNOSTICS:
      transferLeptonDiagnostics();
      break;
    case CMD_GET_LEP_STATUS:
      transferLeptonStatus();
      break;
    case CMD_LEP_RESET:
      triggerLeptonReset();
      break;
    case CMD_LEP_REBOOT:
      triggerLeptonReboot();
      break;
    case CMD_LEP_RUN_FFC:
      triggerLeptonFFC();
      break;
    case CMD_GET_STORAGE_INFO:
      transferStorageInfo();
      break;
    case CMD_GET_SPOT_TEMP:
      transferSpotTemp();
      break;
    case CMD_GET_CAM_RUNTIME:
      transferCamRunTime();
      break;
    case CMD_SET_COLORMAP:
      {
        uint8_t cmap = Serial.read();
        updateColorMap(cmap);
        break;
      }
    case CMD_SET_LEP_GAIN:
      {
        uint8_t gain = Serial.read();
        updateLeptonGain(gain);
        break;
      }
    case CMD_SET_SPOT_POS:
      {
        uint8_t px = Serial.read();
        uint8_t py = Serial.read();
        updateCursorPos(px, py);
        break;
      }
    default:
      Serial.write(CMD_NAK);
      break;
  }
  Serial.flush();
}

void transferLeptonRawData() {
  // send header
  Serial.write(CMD_ACK);
  Serial.write(CMD_GET_LEP_RAWDATA);
  // trigger data transfer
  set_var_lep_transfer_raw(true);
}

void transferLeptonDiagnostics() {
  Serial.write(CMD_ACK);
  Serial.write(CMD_GET_LEP_DIAGNOSTICS);
  Serial.write((uint8_t *)&lepDiagnostics.syncErrorCounter, sizeof(uint32_t));
  crc.add((uint8_t *)&lepDiagnostics.syncErrorCounter, sizeof(uint32_t));
  Serial.write((uint8_t *)&lepDiagnostics.validFrameCounter, sizeof(uint32_t));
  crc.add((uint8_t *)&lepDiagnostics.validFrameCounter, sizeof(uint32_t));
  // CRC
  Serial.write(crc.calc());
}

void transferLeptonStatus() {
  Serial.write(CMD_ACK);
  Serial.write(CMD_GET_LEP_STATUS);
  set_var_lep_transfer_status(true);
}

void triggerLeptonReboot() {
  action_reboot_lepton();
  Serial.write(CMD_ACK);
  Serial.write(CMD_LEP_REBOOT);
  Serial.write(CMD_SUCCESS);
}

void triggerLeptonReset() {
  action_reset_lepton();
  Serial.write(CMD_ACK);
  Serial.write(CMD_LEP_RESET);
  Serial.write(CMD_SUCCESS);
}

void triggerLeptonFFC() {
  action_run_ffc();
  Serial.write(CMD_ACK);
  Serial.write(CMD_LEP_RUN_FFC);
  Serial.write(CMD_SUCCESS);
}

void transferStorageInfo() {
  uint8_t cardState = get_var_sd_card_state();
  uint64_t total = Theia_SDCard.totalBytes();
  uint64_t used = Theia_SDCard.usedBytes();
  uint32_t nbrOfFiles = number_of_files();

  Serial.write(CMD_ACK);
  Serial.write(CMD_GET_STORAGE_INFO);
  Serial.write(cardState);
  crc.add(cardState);
  Serial.write((uint8_t *)&total, sizeof(total));
  crc.add((uint8_t *)&total, sizeof(total));
  Serial.write((uint8_t *)&used, sizeof(used));
  crc.add((uint8_t *)&used, sizeof(used));
  Serial.write((uint8_t *)&nbrOfFiles, sizeof(nbrOfFiles));
  crc.add((uint8_t *)&nbrOfFiles, sizeof(nbrOfFiles));
  // CRC
  Serial.write(crc.calc());
}

void transferSpotTemp() {
  const char *cur_temp_ch = get_var_cursor_temp();
  Serial.write(CMD_ACK);
  Serial.write(CMD_GET_SPOT_TEMP);
  Serial.print(cur_temp_ch);
  crc.add((uint8_t *)&cur_temp_ch, sizeof(cur_temp_ch));
  // CRC
  Serial.write(crc.calc());
}

void transferCamRunTime() {
  const char *runtime_ch = get_var_curr_runtime();
  Serial.write(CMD_ACK);
  Serial.write(CMD_GET_CAM_RUNTIME);
  Serial.print(runtime_ch);
  crc.add((uint8_t *)&runtime_ch, sizeof(runtime_ch));
  // CRC
  Serial.write(crc.calc());
}

void updateColorMap(uint8_t cmap) {
  set_var_palette_num(cmap);
  // update the gui
  action_set_lepton_color_map();
  Serial.write(CMD_ACK);
  Serial.write(CMD_SET_COLORMAP);
  Serial.write(CMD_SUCCESS);
}

void updateLeptonGain(uint8_t gain) {
  set_var_lepton_gain_mode(gain);
  set_var_set_gain_trig(true);
  Serial.write(CMD_ACK);
  Serial.write(CMD_SET_LEP_GAIN);
  Serial.write(CMD_SUCCESS);
}

void updateCursorPos(uint8_t px, uint8_t py){
  // px is between 0 and 159
  // py is between 0 and 119
  // Map the coordinates on the GUI lep image
  uint16_t x = (px * 2) + GUI_LEPTON_IMAGE_X;
  uint16_t y = (py * 2) + GUI_LEPTON_IMAGE_Y;
  action_update_cursor_xy(x, y);
  Serial.write(CMD_ACK);
  Serial.write(CMD_SET_SPOT_POS);
  Serial.write(CMD_SUCCESS);
}

uint32_t number_of_files(){
  uint32_t files = 0;
  File root = Theia_SDCard.open(STORAGE_LEPTON_DIRPATH);
  if (!root) {
    return 0;
  }
  File file = root.openNextFile();
  while (file) {
    if (!file.isDirectory()) {
      files++;
    }
    file = root.openNextFile();
  }
  root.close();
  return files;
}
