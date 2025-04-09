#include <string.h>
#include <lvgl.h>
#include <esp_heap_caps.h>
#include "esp_system.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "vars.h"
#include "arctic.h"
#include "banded.h"
#include "blackhot.h"
#include "double_rainbow.h"
#include "fusion.h"
#include "ice.h"
#include "icefire.h"
#include "ironblack.h"
#include "isotherm.h"
#include "rainbow.h"
#include "sepia.h"
#include "gray.h"
#include "Globals.h"

static const char *TAG = "VARS";

static palette_t palettes[GUI_CMAP_PALETTE_COUNT] = {
  { .name = "Arctic",
    .map_ptr = &arctic_palette_map },
  { .name = "Black Hot",
    .map_ptr = &blackhot_palette_map },
  { .name = "Banded",
    .map_ptr = &banded_palette_map },
  { .name = "Fusion",
    .map_ptr = &fusion_palette_map },
  { .name = "Ice",
    .map_ptr = &ice_palette_map },
  { .name = "Ice Fire",
    .map_ptr = &icefire_palette_map },
  { .name = "Iron Black",
    .map_ptr = &ironblack_palette_map },
  { .name = "IsoTherm",
    .map_ptr = &isotherm_palette_map },
  { .name = "Rainbow",
    .map_ptr = &rainbow_palette_map },
  { .name = "Rainbow HC",
    .map_ptr = &double_rainbow_palette_map },
  { .name = "Sepia",
    .map_ptr = &sepia_palette_map },
  { .name = "White Hot",
    .map_ptr = &gray_palette_map }
};

uint16_t colormap[256];          // Current colormap for fast lookup
uint8_t palette_num = 3;         // Default 3 (Fusion)
uint16_t colormap_raw[256];      // colormap for browse screen raw display
uint8_t brightness_value = 100;  // Default 100%
char *cam_dbg_msg_buf;
char notification_msg[50] = { "" };
char cursor_temp[10] = { "" };
char file_name[50] = { "" };
char img_index[15] = { "" };
char min_temp[10] = { "" };
char max_temp[10] = { "" };
char curr_runtime[10] = { "" };
uint8_t lepton_gain_mode = 0;  // 0: High | 1: Low | 2: Auto
uint8_t cur_temp_format = 0;   // 0: Celsius | 1: Fahrenheit | 2: Kelvin
uint8_t sd_card_state = 0;     // 0: Not Connected | 1: Connected |  2: Almost Full | 3:Full
bool lep_rst_trig = false;
bool lep_reboot_trig = false;
bool dbg_info_trig = false;
bool lep_snap_to_sd = false;
bool agc_mode_trig = false;
bool rad_mode_trig = false;
bool run_ffc_trig = false;
bool set_gain_trig = false;
bool notification_enable = false;
bool image_format = true;  // default checked
bool browse_start_trig = false;
bool browse_next_trig = false;
bool browse_refresh_trig = false;
bool delete_image_trig = false;
bool delete_all = false;
bool lep_transfer_raw = false;
bool lep_transfer_status = false;
uint8_t act_scr_id = 0;   // default main screen is 0
uint8_t act_scr_rot = 0;  // default rotation = NONE = 0


uint8_t get_var_palette_num() {
  return palette_num;
}

void set_var_palette_num(uint8_t value) {
  palette_num = value;
}

uint8_t get_var_brightness_value() {
  return brightness_value;
}

void set_var_brightness_value(uint8_t value) {
  brightness_value = value;
}

bool get_var_dbg_info_trig() {
  return dbg_info_trig;
}

extern void set_var_dbg_info_trig(bool trig) {
  dbg_info_trig = trig;
}

const char *get_var_cursor_temp() {
  return cursor_temp;
}

void set_var_cursor_temp(const char *value) {
  strncpy(cursor_temp, value, sizeof(cursor_temp) / sizeof(char));
  cursor_temp[sizeof(cursor_temp) / sizeof(char) - 1] = 0;
}

const char *get_var_min_temp() {
  return min_temp;
}

void set_var_min_temp(const char *value) {
  strncpy(min_temp, value, sizeof(min_temp) / sizeof(char));
  min_temp[sizeof(min_temp) / sizeof(char) - 1] = 0;
}

const char *get_var_max_temp() {
  return max_temp;
}

void set_var_max_temp(const char *value) {
  strncpy(max_temp, value, sizeof(max_temp) / sizeof(char));
  max_temp[sizeof(max_temp) / sizeof(char) - 1] = 0;
}

const char *get_var_curr_runtime() {
  return curr_runtime;
}

void set_var_curr_runtime(const char *value) {
  strncpy(curr_runtime, value, sizeof(curr_runtime) / sizeof(char));
  curr_runtime[sizeof(curr_runtime) / sizeof(char) - 1] = 0;
}

const char *get_var_file_name() {
  return file_name;
}

void set_var_file_name(const char *value) {
  strncpy(file_name, value, sizeof(file_name) / sizeof(char));
  file_name[sizeof(file_name) / sizeof(char) - 1] = 0;
}

const char *get_var_img_index() {
  return img_index;
}

void set_var_img_index(uint16_t current, uint16_t total) {
  char index_buf[15];
  sprintf(index_buf, "%d/%d", current, total);
  strncpy(img_index, index_buf, sizeof(img_index) / sizeof(char));
  img_index[sizeof(img_index) / sizeof(char) - 1] = 0;
}

uint8_t get_var_lepton_gain_mode() {
  return lepton_gain_mode;
}

void set_var_lepton_gain_mode(uint8_t value) {
  lepton_gain_mode = value;
}

uint8_t get_var_cur_temp_format() {
  return cur_temp_format;
}

void set_var_cur_temp_format(uint8_t value) {
  cur_temp_format = value;
}

void load_palette(uint8_t n) {
  if (n > GUI_CMAP_PALETTE_COUNT) {
    ESP_LOGE(TAG, "Failed to load palette (loading default)");
    n = GUI_CMAP_PALETTE_DEFAULT;
  }
  
  for (int i = 0; i < 256; i++) {
    colormap[i] = RGB_TO_16BIT(
      (*(palettes[n].map_ptr))[i][0],
      (*(palettes[n].map_ptr))[i][1],
      (*(palettes[n].map_ptr))[i][2]);
  }
}


void load_palette_raw(uint8_t n) {
  if (n > GUI_CMAP_PALETTE_COUNT) {
    ESP_LOGE(TAG, "Failed to load palette (loading default)");
    n = GUI_CMAP_PALETTE_DEFAULT;
  }
  for (int i = 0; i < 256; i++) {
    colormap_raw[i] = RGB_TO_16BIT(
      (*(palettes[n].map_ptr))[i][0],
      (*(palettes[n].map_ptr))[i][1],
      (*(palettes[n].map_ptr))[i][2]);
  }
}

uint8_t get_var_act_scr_id() {
  return act_scr_id;
}

void set_var_act_scr_id(uint8_t id) {
  act_scr_id = id;
}

uint8_t get_var_sd_card_state() {
  return sd_card_state;
}

void set_var_sd_card_state(uint8_t value) {
  sd_card_state = value;
}

uint8_t get_var_scr_rot() {
  return act_scr_rot;
}

void set_var_scr_rot(uint8_t rot) {
  act_scr_rot = rot;
}

bool get_var_lep_rst() {
  return lep_rst_trig;
}

void set_var_lep_rst(bool rst) {
  lep_rst_trig = rst;
}

bool get_var_lep_reboot() {
  return lep_reboot_trig;
}

void set_var_lep_reboot(bool reboot) {
  lep_reboot_trig = reboot;
}

bool get_var_lep_snap_to_sd() {
  return lep_snap_to_sd;
}

void set_var_lep_snap_to_sd(bool trig) {
  lep_snap_to_sd = trig;
}

bool get_var_agc_mode_trig() {
  return agc_mode_trig;
}

void set_var_agc_mode_trig(bool trig) {
  agc_mode_trig = trig;
}

bool get_var_rad_mode_trig() {
  return rad_mode_trig;
}

void set_var_rad_mode_trig(bool trig) {
  rad_mode_trig = trig;
}

bool get_var_run_ffc_trig() {
  return run_ffc_trig;
}

void set_var_run_ffc_trig(bool trig) {
  run_ffc_trig = trig;
}

bool get_var_set_gain_trig(){
  return set_gain_trig;
}

void set_var_set_gain_trig(bool trig) {
  set_gain_trig = trig;
}

bool get_var_notification_enable() {
  return notification_enable;
}

void set_var_notification_enable(bool state) {
  notification_enable = state;
}

const char *get_var_notification_msg() {
  return notification_msg;
}

void set_var_notification_msg(const char *value) {
  strncpy(notification_msg, value, sizeof(notification_msg) / sizeof(char));
  notification_msg[sizeof(notification_msg) / sizeof(char) - 1] = 0;
}

bool get_var_browse_start_trig() {
  return browse_start_trig;
}

void set_var_browse_start_trig(bool trig) {
  browse_start_trig = trig;
}

bool get_var_browse_next_trig() {
  return browse_next_trig;
}

void set_var_browse_next_trig(bool trig) {
  browse_next_trig = trig;
}

bool get_var_browse_refresh_trig() {
  return browse_refresh_trig;
}

void set_var_browse_refresh_trig(bool trig) {
  browse_refresh_trig = trig;
}

bool get_var_delete_image() {
  return delete_image_trig;
}

void set_var_delete_image(bool trig) {
  delete_image_trig = trig;
}

bool get_var_image_format() {
  return image_format;
}

void set_var_image_format(bool isBmp) {
  image_format = isBmp;
}

bool get_var_delete_all() {
  return delete_all;
}

void set_var_delete_all(bool trig) {
  delete_all = trig;
}

bool get_var_lep_transfer_raw() {
  return lep_transfer_raw;
}

void set_var_lep_transfer_raw(bool trig) {
  lep_transfer_raw = trig;
}

bool get_var_lep_transfer_status() {
  return lep_transfer_status;
}

void set_var_lep_transfer_status(bool trig) {
  lep_transfer_status = trig;
}


// Debug Info helpers
int append_cam_revision(int index) {
  sprintf(&cam_dbg_msg_buf[index], "HW Version: %s\n", THEIA_HW_REVISION);
  return (strlen(cam_dbg_msg_buf));
}

int append_fw_version(int index) {
  const esp_app_desc_t *app_desc;
  app_desc = esp_app_get_description();
  sprintf(&cam_dbg_msg_buf[index], "FW Version: %s (%s)\n", THEIA_APP_VERSION, app_desc->date);
  return (strlen(cam_dbg_msg_buf));
}

int append_sdk_version(int index) {
  sprintf(&cam_dbg_msg_buf[index], "SDK Version: %s\n", esp_get_idf_version());
  return (strlen(cam_dbg_msg_buf));
}

int append_lepton_info(int index, uint8_t model, float tempFpa, float tempCase) {
  if (model == LEP_3_5_SHUTTER) {
    sprintf(&cam_dbg_msg_buf[index], "[Lepton 3.5]");
  } else {
    sprintf(&cam_dbg_msg_buf[index], "[Lepton Unsupported]");
  }
  index = strlen(cam_dbg_msg_buf);
  sprintf(&cam_dbg_msg_buf[index], "\nTemp: FPA %2.2f°C,  Housing %2.2f°C\n", tempFpa, tempCase);
  return (strlen(cam_dbg_msg_buf));
}

int append_storage_info(int index, bool present, int total, int used, int nbrFiles) {
  if (present) {
    sprintf(&cam_dbg_msg_buf[index], "[Storage]\n%d MB used of %d MB\n%d Files!\n",
            used, total, nbrFiles);
  } else {
    sprintf(&cam_dbg_msg_buf[index], "[Storage]\nNo media\n");
  }

  return (strlen(cam_dbg_msg_buf));
}

int append_mem_info(int index) {
  sprintf(&cam_dbg_msg_buf[index], "[Heap Free]\nInternal %d (%d)\nPSRAM %d (%d)\n",
          heap_caps_get_free_size(MALLOC_CAP_INTERNAL),
          heap_caps_get_minimum_free_size(MALLOC_CAP_INTERNAL),
          heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
          heap_caps_get_minimum_free_size(MALLOC_CAP_SPIRAM));

  return (strlen(cam_dbg_msg_buf));
}

int append_copyright(int index) {
  sprintf(&cam_dbg_msg_buf[index], "\n[THEIA] Camera copyright (C) 2025.\nChe Ahmed.  All rights reserved.\n");

  return (strlen(cam_dbg_msg_buf));
}

void create_save_notification(bool success, bool isBmp) {
  if (success) {
    if (isBmp) {
      set_var_notification_msg("BMP Image Saved to SD!");
    } else {
      set_var_notification_msg("RAW Image Saved to SD!");
    }
  } else {
    set_var_notification_msg("Failed to Save Lepton Image!");
  }

  set_var_notification_enable(true);
}