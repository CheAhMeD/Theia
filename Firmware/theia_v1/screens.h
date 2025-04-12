#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _objects_t {
    lv_obj_t *main_screen;
    lv_obj_t *settings_screen;
    lv_obj_t *browse_screen;
    lv_obj_t *brightness_sel;
    lv_obj_t *debug_info_text_area;
    lv_obj_t *color_map_sel;
    lv_obj_t *rst_btn;
    lv_obj_t *lep_mode_lbl;
    lv_obj_t *ffc_btn;
    lv_obj_t *sd_detect;
    lv_obj_t *runtime_text;
    lv_obj_t *cur_temp_text;
    lv_obj_t *snap_btn;
    lv_obj_t *browse_btn;
    lv_obj_t *reboot_btn;
    lv_obj_t *settings_btn;
    lv_obj_t *max_temp_text;
    lv_obj_t *min_temp_text;
    lv_obj_t *colormap_canvas;
    lv_obj_t *lep_image_outline;
    lv_obj_t *lep_image;
    lv_obj_t *pointer;
    lv_obj_t *notification_box;
    lv_obj_t *reboot_notif_box;
    lv_obj_t *back_btn;
    lv_obj_t *get_info_btn;
    lv_obj_t *img_format_sel;
    lv_obj_t *lepgain_sel;
    lv_obj_t *lep_temp;
    lv_obj_t *rotate_screen_btn;
    lv_obj_t *brightness_value_lbl;
    lv_obj_t *close_btn;
    lv_obj_t *refresh_btn;
    lv_obj_t *next_btn;
    lv_obj_t *del_btn;
    lv_obj_t *del_all_btn;
    lv_obj_t *brw_image_outline;
    lv_obj_t *brw_image;
    lv_obj_t *no_image_msg;
    lv_obj_t *file_name_lbl;
    lv_obj_t *img_index_lbl;
    lv_obj_t *browse_msg_box;
} objects_t;

extern objects_t objects;
extern uint16_t* gui_lep_canvas_buffer;            // Loaded by guiTask for its own use
extern uint16_t* gui_brw_canvas_buffer;            // Loaded by Storage for its own use

enum ScreensEnum {
    SCREEN_ID_MAIN_SCREEN = 1,
    SCREEN_ID_SETTINGS_SCREEN = 2,
    SCREEN_ID_BROWSE_SCREEN = 3,
};

void create_screen_main_screen();
void tick_screen_main_screen();

void create_screen_settings_screen();
void tick_screen_settings_screen();

void create_screen_browse_screen();
void tick_screen_browse_screen();

void create_screens();
void tick_screen(int screen_index);

void invalidate_lepton_image();
void clear_lepton_image();
void invalidate_browse_image();
void clear_browse_image();

void hide_no_image_msg();
void show_no_image_msg();

void update_dbg_msg();

extern void show_settings_screen_msgbox(char *text);
extern void hide_settings_screen_msgbox();

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/