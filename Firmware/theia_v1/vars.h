#ifndef EEZ_LVGL_UI_VARS_H
#define EEZ_LVGL_UI_VARS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// enum declarations
// Palette types
typedef const uint8_t palette_map_t[256][3];
typedef struct {
  char name[32];
  palette_map_t* map_ptr;
} palette_t;

//
// Palette macros
//
// Macro to convert 24-bit color to 16 bit RGB565
#define RGB_TO_16BIT(r, g, b) (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3))
// Macro to convert 24-bit color to 16 bit byte-swapped RGB565 for lv_img
//#define RGB_TO_16BIT_SWAP(r, g, b) (((b & 0xF8) << 5) | (r & 0xF8) | ((g & 0xE0) >> 5) | ((g & 0x1C) << 11))

//
// extern variables
//
extern uint16_t colormap[256];            // Current palette for fast lookup
extern uint16_t colormap_raw[256];        // colormap for browse screen        
extern uint8_t brightness_value;          // Current LCD Brightness (Default 100%)
extern char* cam_dbg_msg_buf;             // Debug Info buffer

extern uint8_t get_var_palette_num();
extern void set_var_palette_num(uint8_t value);
extern uint8_t get_var_brightness_value();
extern void set_var_brightness_value(uint8_t value);
//extern const char *get_var_dbg_msg();
//extern void set_var_dbg_msg(const char *value);
extern const char *get_var_cursor_temp();
extern void set_var_cursor_temp(const char *value);
extern const char *get_var_min_temp();
extern void set_var_min_temp(const char *value);
extern const char *get_var_max_temp();
extern void set_var_max_temp(const char *value);
extern const char *get_var_curr_runtime();
extern void set_var_curr_runtime(const char *value);
extern uint8_t get_var_lepton_gain_mode();
extern void set_var_lepton_gain_mode(uint8_t value);
extern uint8_t get_var_cur_temp_format();
extern void set_var_cur_temp_format(uint8_t value);
extern void load_palette(uint8_t n);
extern void load_palette_raw(uint8_t n);
extern uint8_t get_var_sd_card_state();
extern void set_var_sd_card_state(uint8_t value);
extern uint8_t get_var_act_scr_id();
extern void set_var_act_scr_id(uint8_t id);
extern uint8_t get_var_scr_rot();
extern void set_var_scr_rot(uint8_t rot);
extern bool get_var_lep_rst();
extern void set_var_lep_rst(bool rst);
extern bool get_var_lep_reboot();
extern void set_var_lep_reboot(bool reboot);
extern const char *get_var_file_name();
extern void set_var_file_name(const char *value);
extern const char *get_var_img_index();
extern void set_var_img_index(uint16_t current, uint16_t total);
extern bool get_var_dbg_info_trig();
extern void set_var_dbg_info_trig(bool trig);
extern bool get_var_lep_snap_to_sd();
extern void set_var_lep_snap_to_sd(bool trig);
extern bool get_var_run_ffc_trig();
extern void set_var_run_ffc_trig(bool trig);
extern bool get_var_set_gain_trig();
extern void set_var_set_gain_trig(bool trig);
extern bool get_var_notification_enable();
extern void set_var_notification_enable(bool state);
extern const char *get_var_notification_msg();
extern void set_var_notification_msg(const char *value);
extern bool get_var_browse_start_trig();
extern void set_var_browse_start_trig(bool trig);
extern bool get_var_browse_next_trig();
extern void set_var_browse_next_trig(bool trig);
extern bool get_var_browse_refresh_trig();
extern void set_var_browse_refresh_trig(bool trig);
extern bool get_var_delete_image();
extern void set_var_delete_image(bool trig);
extern bool get_var_image_format();
extern void set_var_image_format(bool trig);
extern bool get_var_delete_all();
extern void set_var_delete_all(bool trig);
extern bool get_var_lep_transfer_raw();
extern void set_var_lep_transfer_raw(bool trig);
extern bool get_var_lep_transfer_status();
extern void set_var_lep_transfer_status(bool trig);
extern bool get_var_update_palette_trig();
extern void set_var_update_palette_trig(bool trig);
extern bool get_var_update_temp_unit_trig();
extern void set_var_update_temp_unit_trig(bool trig);
extern bool get_var_update_img_format_trig();
extern void set_var_update_img_format_trig(bool trig);

extern int append_cam_revision(int index);
extern int append_fw_version(int index);
extern int append_sdk_version(int index);
extern int append_lepton_info(int index, uint8_t model, float tempFpa, float tempCase);
extern int append_storage_info(int index, bool present, int total, int used, int nbrFiles);
extern int append_mem_info(int index);
extern int append_copyright(int index);

extern void create_save_notification(bool success, bool isBmp);

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_VARS_H*/