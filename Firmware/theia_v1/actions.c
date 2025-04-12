#include <esp_heap_caps.h>
#include "ui.h"
#include "actions.h"
#include "screens.h"
#include "Globals.h"
#include "vars.h"

void action_switch_to_settings() {
  set_var_act_scr_id(SCREEN_ID_SETTINGS_SCREEN);
  loadScreen(SCREEN_ID_SETTINGS_SCREEN);
}

void action_switch_to_browse() {
  set_var_act_scr_id(SCREEN_ID_BROWSE_SCREEN);
  loadScreen(SCREEN_ID_BROWSE_SCREEN);
  set_var_browse_start_trig(true);
}

void action_switch_to_main() {
  set_var_act_scr_id(SCREEN_ID_MAIN_SCREEN);
  loadScreen(SCREEN_ID_MAIN_SCREEN);
}

void action_save_snap_to_sd() {
  set_var_lep_snap_to_sd(true);
}

void action_run_ffc() {
  set_var_run_ffc_trig(true);
}

void action_show_next_image() {
  set_var_browse_next_trig(true);
}

void action_refresh_browse_image() {
  set_var_browse_refresh_trig(true);
}

void action_delete_current_lepton_image() {
  set_var_delete_image(true);
}

void action_delete_all_lepton_images() {
  set_var_delete_all(true);
}

void action_rotate_screen() {
  lv_disp_rot_t curRot = lv_disp_get_rotation(NULL);
  lv_disp_set_rotation(NULL, (curRot == LV_DISP_ROT_NONE) ? LV_DISP_ROT_180 : LV_DISP_ROT_NONE);
  set_var_scr_rot((curRot == LV_DISP_ROT_NONE) ? LV_DISP_ROT_180 : LV_DISP_ROT_NONE);
}

void action_get_camera_debug_info() {
  set_var_dbg_info_trig(true);
}

void action_reset_lepton() {
  set_var_lep_rst(true);
}

void action_reboot_lepton() {
  set_var_lep_reboot(true);
}

void action_set_lepton_color_map() {
  int32_t cur_pal = get_var_palette_num();
  // draw the canvas
  update_color_map_canvas(cur_pal);
  // trigger storage update
  set_var_update_palette_trig(true);
}

void update_color_map_canvas(int32_t n) {
  load_palette(n);
  // update canvas
  for (int i = 0; i < GUI_CMAP_CANVAS_HEIGHT; i++) {
    // drawing in a 1px smaller canvas to give a cool 3D effect
    for (int p = GUI_CMAP_PALETTE_X1 + 1; p < GUI_CMAP_PALETTE_X2; p++) {
      lv_canvas_set_px(objects.colormap_canvas, p, i, (lv_color_t)colormap[255 - i]);
    }
  }
}

void action_update_cursor_position() {
  lv_point_t p;
  lv_indev_t* indev = lv_indev_get_act();
  lv_indev_get_point(indev, &p);
  // center the pointer to the selected point
  // pointer size is 14x16 (center is @ x-7+2, y-8+2)
  action_update_cursor_xy((uint16_t)p.x - 5, (uint16_t)p.y - 6);
}

void action_update_cursor_xy(uint16_t x, uint16_t y) {
  lv_obj_set_pos(objects.pointer, x, y);
}

void action_set_image_format() {
  if (lv_obj_has_state(objects.img_format_sel, LV_STATE_CHECKED)) {
    set_var_image_format(true);
  } else {
    set_var_image_format(false);
  }
  // trigger storage update
  set_var_update_img_format_trig(true);
}

static void event_handler_cb_browse_msg_box(lv_event_t *e) {
  lv_obj_t *obj = lv_event_get_current_target(e);
  bool deleteAll = (bool) lv_event_get_user_data(e);
  if (lv_msgbox_get_active_btn(obj) == 0) {
    if (deleteAll) {
      action_delete_all_lepton_images();
    } else {
      action_delete_current_lepton_image();
    }
    
  }
  lv_msgbox_close(obj);
}

void action_show_browse_msg_box(lv_event_t* e) {
  bool deleteAll = (bool) lv_event_get_user_data(e);
  static const char* btns[] = { "Yes", "No", "" };
  char *text;
  text = (char *)heap_caps_malloc(54, MALLOC_CAP_SPIRAM);
  // browseMsgBox
  if (deleteAll) {
    strcpy(text, "All Images will be permenatly deleted! Confirm?");
  } else {
    strcpy(text, "Current Image will be permenatly deleted! Confirm?");
  }

  lv_obj_t* obj = lv_msgbox_create(lv_scr_act(), "", text, btns, false);
  objects.browse_msg_box = obj;
  lv_obj_set_pos(obj, 96, 124);
  lv_obj_set_size(obj, 200, 120);
  lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_align(obj, LV_ALIGN_DEFAULT, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_add_event_cb(obj, event_handler_cb_browse_msg_box, LV_EVENT_VALUE_CHANGED, (void*)deleteAll);

}