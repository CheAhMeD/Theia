#ifndef EEZ_LVGL_UI_EVENTS_H
#define EEZ_LVGL_UI_EVENTS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void action_switch_to_settings();
extern void action_switch_to_main();
extern void action_switch_to_browse();
extern void action_save_snap_to_sd();
extern void action_run_ffc();
extern void action_rotate_screen();
extern void action_get_camera_debug_info();
extern void action_reset_lepton();
extern void action_reboot_lepton();
extern void action_set_lepton_color_map();
extern void action_update_cursor_position();
extern void action_update_cursor_xy(uint16_t x, uint16_t y);
extern void update_color_map_canvas(int32_t n);
extern void action_delete_current_lepton_image();
extern void action_delete_all_lepton_images();
extern void action_show_next_image();
extern void action_refresh_browse_image();
extern void action_set_image_format();
extern void action_show_browse_msg_box(lv_event_t * e);



#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_EVENTS_H*/