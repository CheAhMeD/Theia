#include <string.h>
#include <esp_heap_caps.h>
#include "screens.h"
#include "actions.h"
#include "vars.h"
#include "ui.h"
#include "Globals.h"
#include "esp_log.h"

static const char *TAG = "GUI";


objects_t objects;
lv_obj_t *tick_value_change_obj;
static lv_img_dsc_t lepton_img_dsc;
uint16_t *gui_lep_canvas_buffer;  // Loaded by lepTask for its own use
static lv_img_dsc_t browse_img_dsc;
uint16_t *gui_brw_canvas_buffer;  // Loaded by Storage for its own use
uint8_t cur_sdCard_state = 0;
const uint32_t cur_sdCard_states[4] = { 0xffff0000, 0xffffffff, 0xffffff00, 0xffff8000 };

static void event_handler_cb_settings_screen_brightness_sel(lv_event_t *e) {
  lv_event_code_t event = lv_event_get_code(e);
  if (event == LV_EVENT_VALUE_CHANGED) {
    lv_obj_t *ta = lv_event_get_target(e);
    if (tick_value_change_obj != ta) {
      int32_t value = lv_slider_get_value(ta);
      set_var_brightness_value(value);
    }
  }
}

static void event_handler_cb_settings_screen_color_map_sel(lv_event_t *e) {
  lv_event_code_t event = lv_event_get_code(e);
  if (event == LV_EVENT_VALUE_CHANGED) {
    lv_obj_t *ta = lv_event_get_target(e);
    if (tick_value_change_obj != ta) {
      int32_t value = lv_dropdown_get_selected(ta);
      set_var_palette_num(value);

    }
  }
}


static void event_handler_cb_settings_screen_lep_temp(lv_event_t *e) {
  lv_event_code_t event = lv_event_get_code(e);
  if (event == LV_EVENT_VALUE_CHANGED) {
    lv_obj_t *ta = lv_event_get_target(e);
    if (tick_value_change_obj != ta) {
      int32_t value = lv_dropdown_get_selected(ta);
      set_var_cur_temp_format(value);
      set_var_update_temp_unit_trig(true);
    }
  }
}

static void event_handler_cb_settings_screen_lepgain_sel(lv_event_t *e) {
  lv_event_code_t event = lv_event_get_code(e);
  if (event == LV_EVENT_VALUE_CHANGED) {
    lv_obj_t *ta = lv_event_get_target(e);
    if (tick_value_change_obj != ta) {
      int32_t value = lv_dropdown_get_selected(ta);
      set_var_lepton_gain_mode(value);
      set_var_set_gain_trig(true);
    }
  }
}


void create_screen_main_screen() {
  lv_obj_t *obj = lv_obj_create(0);
  objects.main_screen = obj;
  lv_obj_set_pos(obj, 0, 0);
  lv_obj_set_size(obj, 480, 320);
  lv_obj_set_style_bg_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
  {
    lv_obj_t *parent_obj = obj;
    {
      // sdDetect
      lv_obj_t *obj = lv_label_create(parent_obj);
      objects.sd_detect = obj;
      lv_obj_set_pos(obj, 340, 0);
      lv_obj_set_size(obj, 40, 40);
      lv_label_set_text(obj, LV_SYMBOL_SD_CARD);
      lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_pad_top(obj, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_text_color(obj, lv_color_hex(cur_sdCard_states[get_var_sd_card_state()]), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    {
      // runtimeText
      lv_obj_t *obj = lv_label_create(parent_obj);
      objects.runtime_text = obj;
      lv_obj_set_pos(obj, 156, 6);
      lv_obj_set_size(obj, 130, 23);
      lv_label_set_text(obj, "00:00:00");
      lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_text_font(obj, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_text_color(obj, lv_color_hex(0xff2196f3), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    {
      // curTempText
      lv_obj_t *obj = lv_label_create(parent_obj);
      objects.cur_temp_text = obj;
      lv_obj_set_pos(obj, 146, 291);
      lv_obj_set_size(obj, 149, 23);
      lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_text_font(obj, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_text_color(obj, lv_color_hex(0xffffc200), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    {
      // snapBtn
      lv_obj_t *obj = lv_btn_create(parent_obj);
      objects.snap_btn = obj;
      lv_obj_set_pos(obj, 401, 10);
      lv_obj_set_size(obj, 60, 60);
      lv_obj_add_event_cb(obj, action_save_snap_to_sd, LV_EVENT_PRESSED, (void *)0);
      lv_obj_set_style_bg_opa(obj, 100, LV_PART_MAIN | LV_STATE_DISABLED);
      lv_obj_add_state(obj, LV_STATE_DISABLED);
      {
        lv_obj_t *parent_obj = obj;
        {
          lv_obj_t *obj = lv_label_create(parent_obj);
          lv_obj_set_pos(obj, 0, 0);
          lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
          lv_label_set_text(obj, LV_SYMBOL_SAVE);
          lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
          lv_obj_set_style_text_font(obj, &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
      }
    }
    {
      // browseBtn
      lv_obj_t *obj = lv_btn_create(parent_obj);
      objects.browse_btn = obj;
      lv_obj_set_pos(obj, 401, 90);
      lv_obj_set_size(obj, 60, 60);
      lv_obj_add_event_cb(obj, action_switch_to_browse, LV_EVENT_PRESSED, (void *)0);
      lv_obj_set_style_bg_opa(obj, 100, LV_PART_MAIN | LV_STATE_DISABLED);
      lv_obj_add_state(obj, LV_STATE_DISABLED);
      {
        lv_obj_t *parent_obj = obj;
        {
          lv_obj_t *obj = lv_label_create(parent_obj);
          lv_obj_set_pos(obj, 0, 0);
          lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
          lv_label_set_text(obj, LV_SYMBOL_DIRECTORY);
          lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
          lv_obj_set_style_text_font(obj, &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
      }
    }
    {
      // ffcBtn
      lv_obj_t *obj = lv_btn_create(parent_obj);
      objects.ffc_btn = obj;
      lv_obj_set_pos(obj, 401, 170);
      lv_obj_set_size(obj, 60, 60);
      lv_obj_add_event_cb(obj, action_run_ffc, LV_EVENT_PRESSED, (void *)0);
      {
        lv_obj_t *parent_obj = obj;
        {
          lv_obj_t *obj = lv_label_create(parent_obj);
          lv_obj_set_pos(obj, 0, 0);
          lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
          lv_label_set_text(obj, LV_SYMBOL_EYE_CLOSE);
          lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
          lv_obj_set_style_text_font(obj, &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
      }
    }
    {
      // settingsBtn
      lv_obj_t *obj = lv_btn_create(parent_obj);
      objects.settings_btn = obj;
      lv_obj_set_pos(obj, 401, 250);
      lv_obj_set_size(obj, 60, 60);
      lv_obj_add_event_cb(obj, action_switch_to_settings, LV_EVENT_PRESSED, (void *)0);
      {
        lv_obj_t *parent_obj = obj;
        {
          lv_obj_t *obj = lv_label_create(parent_obj);
          lv_obj_set_pos(obj, 0, 0);
          lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
          lv_label_set_text(obj, LV_SYMBOL_SETTINGS);
          lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
          lv_obj_set_style_text_font(obj, &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
      }
    }
    {
      // maxTempText
      lv_obj_t *obj = lv_label_create(parent_obj);
      objects.max_temp_text = obj;
      lv_obj_set_pos(obj, 5, 10);
      lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    }
    {
      // minTempText
      lv_obj_t *obj = lv_label_create(parent_obj);
      objects.min_temp_text = obj;
      lv_obj_set_pos(obj, 5, 295);
      lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    }
    {
      // colormapCanvas
      lv_obj_t *obj = lv_canvas_create(parent_obj);
      objects.colormap_canvas = obj;
      lv_obj_set_pos(obj, GUI_CMAP_X, GUI_CMAP_Y);
      lv_obj_set_size(obj, GUI_CMAP_CANVAS_WIDTH, GUI_CMAP_CANVAS_HEIGHT);
      static lv_color_t cbuf[(32 * GUI_CMAP_CANVAS_WIDTH) / 8 * GUI_CMAP_CANVAS_HEIGHT];
      lv_canvas_set_buffer(obj, cbuf, GUI_CMAP_CANVAS_WIDTH, GUI_CMAP_CANVAS_HEIGHT, LV_IMG_CF_TRUE_COLOR);
      lv_obj_set_style_border_color(obj, lv_color_hex(0xff2196f3), LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
      // update color map
      update_color_map_canvas(get_var_palette_num());
    }
    {
      // lepImageOutline
      lv_obj_t *obj = lv_obj_create(parent_obj);
      objects.lep_image_outline = obj;
      lv_obj_set_pos(obj, 59, 39);
      lv_obj_set_size(obj, 322, 242);
      lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_border_color(obj, lv_color_hex(0xff2196f3), LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    {
      // lepImage
      lepton_img_dsc.header.always_zero = 0;
      lepton_img_dsc.header.cf = LV_IMG_CF_TRUE_COLOR;
      lepton_img_dsc.header.w = GUI_LEPTON_IMAGE_WIDTH;
      lepton_img_dsc.header.h = GUI_LEPTON_IMAGE_HEIGHT;
      lepton_img_dsc.data_size = GUI_LEPTON_IMAGE_WIDTH * GUI_LEPTON_IMAGE_HEIGHT * 2;
      lepton_img_dsc.data = (uint8_t *)gui_lep_canvas_buffer;
      lv_obj_t *obj = lv_img_create(parent_obj);
      objects.lep_image = obj;
      lv_img_set_src(obj, &lepton_img_dsc);
      lv_obj_set_pos(obj, GUI_LEPTON_IMAGE_X, GUI_LEPTON_IMAGE_Y);
      lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);
      lv_obj_add_event_cb(obj, action_update_cursor_position, LV_EVENT_PRESSED, (void *)0);
    }
    {
      // pointer
      lv_obj_t *obj = lv_label_create(parent_obj);
      objects.pointer = obj;
      lv_obj_set_pos(obj, 212, 152);
      lv_obj_set_size(obj, 14, 16);
      lv_label_set_text(obj, "X");
      lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_text_font(obj, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_border_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_radius(obj, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
    }
  }
}

void tick_screen_main_screen() {
  {
    const uint8_t new_val = get_var_sd_card_state();
    if (new_val != cur_sdCard_state) {
      uint32_t c = cur_sdCard_states[new_val];
      lv_obj_set_style_text_color(objects.sd_detect, lv_color_hex(c), LV_PART_MAIN | LV_STATE_DEFAULT);
      cur_sdCard_state = new_val;
      //disable snap & browse btns
      if (cur_sdCard_state == 0) {
        lv_obj_add_state(objects.browse_btn, LV_STATE_DISABLED);
        lv_obj_add_state(objects.snap_btn, LV_STATE_DISABLED);
      } else {
        lv_obj_clear_state(objects.browse_btn, LV_STATE_DISABLED);
        lv_obj_clear_state(objects.snap_btn, LV_STATE_DISABLED);
      }
    }
  }
  {
    const char *new_val = get_var_curr_runtime();
    const char *cur_val = lv_label_get_text(objects.runtime_text);
    if (strcmp(new_val, cur_val) != 0) {
      tick_value_change_obj = objects.runtime_text;
      lv_label_set_text(objects.runtime_text, new_val);
      tick_value_change_obj = NULL;
    }
  }
  {
    const char *new_val = get_var_cursor_temp();
    const char *cur_val = lv_label_get_text(objects.cur_temp_text);
    if (strcmp(new_val, cur_val) != 0) {
      tick_value_change_obj = objects.cur_temp_text;
      lv_label_set_text(objects.cur_temp_text, new_val);
      tick_value_change_obj = NULL;
    }
  }
  {
    const char *new_val = get_var_max_temp();
    const char *cur_val = lv_label_get_text(objects.max_temp_text);
    if (strcmp(new_val, cur_val) != 0) {
      tick_value_change_obj = objects.max_temp_text;
      lv_label_set_text(objects.max_temp_text, new_val);
      tick_value_change_obj = NULL;
    }
  }
  {
    const char *new_val = get_var_min_temp();
    const char *cur_val = lv_label_get_text(objects.min_temp_text);
    if (strcmp(new_val, cur_val) != 0) {
      tick_value_change_obj = objects.min_temp_text;
      lv_label_set_text(objects.min_temp_text, new_val);
      tick_value_change_obj = NULL;
    }
  }
  {
    const bool new_val = get_var_notification_enable();
    if (new_val) {
      set_var_notification_enable(false);
      // Create a Notification
      {
        lv_obj_t *obj = lv_msgbox_create(lv_scr_act(), get_var_notification_msg(), "", 0, true);
        objects.notification_box = obj;
        lv_obj_set_pos(obj, 79, 130);
        lv_obj_set_size(obj, 280, 60);
        lv_obj_add_flag(obj, LV_OBJ_FLAG_FLOATING);
        lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_CHAIN_HOR | LV_OBJ_FLAG_SCROLL_CHAIN_VER | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_WITH_ARROW | LV_OBJ_FLAG_SNAPPABLE);
        lv_obj_set_style_align(obj, LV_ALIGN_DEFAULT, LV_PART_MAIN | LV_STATE_DEFAULT);
      }
    }
  }
}

void create_screen_settings_screen() {
  lv_obj_t *obj = lv_obj_create(0);
  objects.settings_screen = obj;
  lv_obj_set_pos(obj, 0, 0);
  lv_obj_set_size(obj, 480, 320);
  lv_obj_set_style_bg_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
  {
    lv_obj_t *parent_obj = obj;
    {
      // backBtn
      lv_obj_t *obj = lv_btn_create(parent_obj);
      objects.back_btn = obj;
      lv_obj_set_pos(obj, 437, 5);
      lv_obj_set_size(obj, 35, 35);
      lv_obj_add_event_cb(obj, action_switch_to_main, LV_EVENT_PRESSED, (void *)0);
      {
        lv_obj_t *parent_obj = obj;
        {
          lv_obj_t *obj = lv_label_create(parent_obj);
          lv_obj_set_pos(obj, 0, 0);
          lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
          lv_label_set_text(obj, LV_SYMBOL_CLOSE);
          lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
          lv_obj_set_style_text_font(obj, &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
      }
    }
    {
      lv_obj_t *obj = lv_label_create(parent_obj);
      lv_obj_set_pos(obj, 12, 9);
      lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
      lv_label_set_text(obj, "Camera Settings");
      lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_text_font(obj, &lv_font_montserrat_26, LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    {
      // brightnessSel
      lv_obj_t *obj = lv_slider_create(parent_obj);
      objects.brightness_sel = obj;
      lv_obj_set_pos(obj, 297, 68);
      lv_obj_set_size(obj, 122, 10);
      lv_slider_set_range(obj, 10, 100);
      lv_obj_add_event_cb(obj, event_handler_cb_settings_screen_brightness_sel, LV_EVENT_ALL, 0);
      lv_obj_add_flag(obj, LV_OBJ_FLAG_SCROLL_CHAIN_VER | LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    }
    {
      // colorMapLbl
      lv_obj_t *obj = lv_label_create(parent_obj);
      lv_obj_set_pos(obj, 12, 65);
      lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
      lv_label_set_text(obj, "Lepton Color Map:");
      lv_obj_set_style_text_color(obj, lv_color_hex(0xff2196f3), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    {
      // brightnessLbl
      lv_obj_t *obj = lv_label_create(parent_obj);
      lv_obj_set_pos(obj, 190, 65);
      lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
      lv_label_set_text(obj, "Brightness:");
      lv_obj_set_style_text_color(obj, lv_color_hex(0xff2196f3), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    {
      // leptonSettingsLbl
      lv_obj_t *obj = lv_label_create(parent_obj);
      lv_obj_set_pos(obj, 12, 141);
      lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
      lv_label_set_text(obj, "Lepton Settings:");
      lv_obj_set_style_text_color(obj, lv_color_hex(0xff2196f3), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    {
      // brightnessValueLbl
      lv_obj_t *obj = lv_label_create(parent_obj);
      objects.brightness_value_lbl = obj;
      lv_obj_set_pos(obj, 435, 65);
      lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
      lv_label_set_text(obj, "");
    }
    {
      lv_obj_t *obj = lv_label_create(parent_obj);
      lv_obj_set_pos(obj, 459, 65);
      lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
      lv_label_set_text(obj, "%");
    }
    {
      // debugInfoTextArea
      lv_obj_t *obj = lv_textarea_create(parent_obj);
      objects.debug_info_text_area = obj;
      lv_obj_set_pos(obj, 189, 93);
      lv_obj_set_size(obj, 282, 161);
      lv_textarea_set_max_length(obj, 1024);
      lv_textarea_set_placeholder_text(obj, "Theia Debug Info...");
      lv_textarea_set_one_line(obj, false);
      lv_textarea_set_password_mode(obj, false);
      lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_SCROLL_CHAIN_HOR | LV_OBJ_FLAG_SCROLL_CHAIN_VER | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_ON_FOCUS | LV_OBJ_FLAG_SCROLL_WITH_ARROW | LV_OBJ_FLAG_SNAPPABLE);
      lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_AUTO);
      lv_obj_set_scroll_snap_x(obj, LV_DIR_VER);
      lv_obj_set_style_bg_color(obj, lv_color_hex(0xff15171a), LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_border_color(obj, lv_color_hex(0xff15171a), LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_text_font(obj, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_border_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    {
      lv_obj_t *obj = lv_obj_create(parent_obj);
      lv_obj_set_pos(obj, 189, 254);
      lv_obj_set_size(obj, 67, 38);
      lv_obj_set_style_bg_color(obj, lv_color_hex(0xff15171a), LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    {
      // getInfoBtn
      lv_obj_t *obj = lv_btn_create(parent_obj);
      objects.get_info_btn = obj;
      lv_obj_set_pos(obj, 190, 266);
      lv_obj_set_size(obj, 66, 37);
      lv_obj_add_event_cb(obj, action_get_camera_debug_info, LV_EVENT_PRESSED, (void *)0);
      {
        lv_obj_t *parent_obj = obj;
        {
          lv_obj_t *obj = lv_label_create(parent_obj);
          lv_obj_set_pos(obj, 0, 0);
          lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
          lv_label_set_text(obj, LV_SYMBOL_DOWNLOAD);
          lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
      }
    }
    {
      lv_obj_t *obj = lv_label_create(parent_obj);
      lv_obj_set_pos(obj, 193, 253);
      lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
      lv_label_set_text(obj, "Update Info");
      lv_obj_set_style_text_font(obj, &lv_font_montserrat_10, LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_text_color(obj, lv_color_hex(0xff2196f3), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    {
      // rotateScreenBtn
      lv_obj_t *obj = lv_btn_create(parent_obj);
      objects.rotate_screen_btn = obj;
      lv_obj_set_pos(obj, 406, 266);
      lv_obj_set_size(obj, 65, 37);
      lv_obj_add_event_cb(obj, action_rotate_screen, LV_EVENT_PRESSED, (void *)0);
      {
        lv_obj_t *parent_obj = obj;
        {
          lv_obj_t *obj = lv_label_create(parent_obj);
          lv_obj_set_pos(obj, 0, 0);
          lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
          lv_label_set_text(obj, LV_SYMBOL_LOOP);
          lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
      }
    }
    {
      lv_obj_t *obj = lv_label_create(parent_obj);
      lv_obj_set_pos(obj, 403, 253);
      lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
      lv_label_set_text(obj, "Rotate Screen");
      lv_obj_set_style_text_font(obj, &lv_font_montserrat_10, LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_text_color(obj, lv_color_hex(0xff2196f3), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    {
      // colorMapSel
      lv_obj_t *obj = lv_dropdown_create(parent_obj);
      objects.color_map_sel = obj;
      lv_obj_set_pos(obj, 12, 93);
      lv_obj_set_size(obj, 159, LV_SIZE_CONTENT);
      lv_dropdown_set_options(obj, "Arctic\nBlack Hot\nBanded\nFusion\nIce\nIce Fire\nIron Black\nIsoTherm\nRainbow\nRainbow HC\nSepia\nWhite Hot");
      lv_obj_add_event_cb(obj, event_handler_cb_settings_screen_color_map_sel, LV_EVENT_ALL, 0);
      lv_obj_add_event_cb(obj, action_set_lepton_color_map, LV_EVENT_VALUE_CHANGED, (void *)0);
    }
    {
      // resetBtn
      lv_obj_t *obj = lv_btn_create(parent_obj);
      objects.rst_btn = obj;
      lv_obj_set_pos(obj, 9, 162);
      lv_obj_set_size(obj, 79, 40);
      lv_obj_add_event_cb(obj, action_reset_lepton, LV_EVENT_PRESSED, (void *)0);
      {
        lv_obj_t *parent_obj = obj;
        {
          // lepModeLbl
          lv_obj_t *obj = lv_label_create(parent_obj);
          objects.lep_mode_lbl = obj;
          lv_obj_set_pos(obj, 0, 0);
          lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
          lv_label_set_text(obj, "Reset");
          lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
      }
    }

    {
      // rebootBtn
      lv_obj_t *obj = lv_btn_create(parent_obj);
      objects.reboot_btn = obj;
      lv_obj_set_pos(obj, 100, 162);
      lv_obj_set_size(obj, 79, 40);
      lv_obj_add_event_cb(obj, action_reboot_lepton, LV_EVENT_PRESSED, (void *)0);
      {
        lv_obj_t *parent_obj = obj;
        {
          lv_obj_t *obj = lv_label_create(parent_obj);
          lv_obj_set_pos(obj, 0, 0);
          lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
          lv_label_set_text(obj, "Reboot");
          lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
      }
    }
    {
      lv_obj_t *obj = lv_label_create(parent_obj);
      lv_obj_set_pos(obj, 10, 208);
      lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
      lv_label_set_text(obj, "Gain Mode:");
    }
    {
      // lepgainSel
      lv_obj_t *obj = lv_dropdown_create(parent_obj);
      objects.lepgain_sel = obj;
      lv_obj_set_pos(obj, 10, 229);
      lv_obj_set_size(obj, 79, LV_SIZE_CONTENT);
      lv_dropdown_set_options(obj, "High\nLow\nAuto");
      lv_obj_add_event_cb(obj, event_handler_cb_settings_screen_lepgain_sel, LV_EVENT_ALL, 0);
    }
    {
      lv_obj_t *obj = lv_label_create(parent_obj);
      lv_obj_set_pos(obj, 102, 208);
      lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
      lv_label_set_text(obj, "Temp Unit:");
    }
    {
      // lepTemp
      lv_obj_t *obj = lv_dropdown_create(parent_obj);
      objects.lep_temp = obj;
      lv_obj_set_pos(obj, 102, 229);
      lv_obj_set_size(obj, 79, LV_SIZE_CONTENT);
      lv_dropdown_set_options(obj, "°C\n°F\n°K");
      lv_obj_add_event_cb(obj, event_handler_cb_settings_screen_lep_temp, LV_EVENT_ALL, 0);
    }
    {
      // imgFormatSel
      lv_obj_t *obj = lv_checkbox_create(parent_obj);
      objects.img_format_sel = obj;
      lv_obj_set_pos(obj, 12, 281);
      lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
      lv_checkbox_set_text(obj, "Save Images BMP");
      lv_obj_add_event_cb(obj, action_set_image_format, LV_EVENT_VALUE_CHANGED, (void *)0);
    }
  }
  {
    cam_dbg_msg_buf = heap_caps_malloc(GUI_DBG_INFO_MAX_CHARS, MALLOC_CAP_SPIRAM);
    if (cam_dbg_msg_buf == NULL) {
      ESP_LOGE(TAG, "malloc debug info buffer failed");
    }
  }
}

void tick_screen_settings_screen() {
  {
    int32_t new_val = get_var_brightness_value();
    int32_t cur_val = lv_slider_get_value(objects.brightness_sel);
    if (new_val != cur_val) {
      tick_value_change_obj = objects.brightness_sel;
      lv_slider_set_value(objects.brightness_sel, new_val, LV_ANIM_OFF);
      tick_value_change_obj = NULL;
    }
  }
  {
    char new_val[8];
    lv_snprintf(new_val, sizeof(new_val), "%d", (int32_t)get_var_brightness_value());
    const char *cur_val = lv_label_get_text(objects.brightness_value_lbl);
    if (strcmp(new_val, cur_val) != 0) {
      tick_value_change_obj = objects.brightness_value_lbl;
      lv_label_set_text(objects.brightness_value_lbl, new_val);
      tick_value_change_obj = NULL;
    }
  }
  {
    if (!(lv_obj_get_state(objects.color_map_sel) & LV_STATE_EDITED)) {
      int32_t new_val = get_var_palette_num();
      int32_t cur_val = lv_dropdown_get_selected(objects.color_map_sel);
      if (new_val != cur_val) {
        tick_value_change_obj = objects.color_map_sel;
        lv_dropdown_set_selected(objects.color_map_sel, new_val);
        tick_value_change_obj = NULL;
      }
    }
  }
  {
    if (!(lv_obj_get_state(objects.lep_temp) & LV_STATE_EDITED)) {
      int32_t new_val = get_var_cur_temp_format();
      int32_t cur_val = lv_dropdown_get_selected(objects.lep_temp);
      if (new_val != cur_val) {
        tick_value_change_obj = objects.lep_temp;
        lv_dropdown_set_selected(objects.lep_temp, new_val);
        tick_value_change_obj = NULL;
      }
    }
  }
  {
    if (!(lv_obj_get_state(objects.lepgain_sel) & LV_STATE_EDITED)) {
      int32_t new_val = get_var_lepton_gain_mode();
      int32_t cur_val = lv_dropdown_get_selected(objects.lepgain_sel);
      if (new_val != cur_val) {
        tick_value_change_obj = objects.lepgain_sel;
        lv_dropdown_set_selected(objects.lepgain_sel, new_val);
        tick_value_change_obj = NULL;
      }
    }
  }
  {
    bool new_val = get_var_image_format();
    bool cur_val = lv_obj_has_state(objects.img_format_sel, LV_STATE_CHECKED);
    if (new_val != cur_val) {
      tick_value_change_obj = objects.img_format_sel;
      if (new_val) lv_obj_add_state(objects.img_format_sel, LV_STATE_CHECKED);
      else lv_obj_clear_state(objects.img_format_sel, LV_STATE_CHECKED);
      tick_value_change_obj = NULL;
    }
  }
}

void create_screen_browse_screen() {
  lv_obj_t *obj = lv_obj_create(0);
  objects.browse_screen = obj;
  lv_obj_set_pos(obj, 0, 0);
  lv_obj_set_size(obj, 480, 320);
  lv_obj_set_style_bg_color(obj, lv_color_hex(0xff000000), LV_PART_MAIN | LV_STATE_DEFAULT);
  {
    lv_obj_t *parent_obj = obj;
    {
      // closeBtn
      lv_obj_t *obj = lv_btn_create(parent_obj);
      objects.close_btn = obj;
      lv_obj_set_pos(obj, 437, 5);
      lv_obj_set_size(obj, 35, 35);
      lv_obj_add_event_cb(obj, action_switch_to_main, LV_EVENT_PRESSED, (void *)0);
      {
        lv_obj_t *parent_obj = obj;
        {
          lv_obj_t *obj = lv_label_create(parent_obj);
          lv_obj_set_pos(obj, 0, 0);
          lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
          lv_label_set_text(obj, LV_SYMBOL_CLOSE);
          lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
          lv_obj_set_style_text_font(obj, &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
      }
    }
    {
      lv_obj_t *obj = lv_label_create(parent_obj);
      lv_obj_set_pos(obj, 12, 5);
      lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
      lv_label_set_text(obj, "Browse Images");
      lv_obj_set_style_text_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_text_font(obj, &lv_font_montserrat_26, LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    {
      // nextBtn
      lv_obj_t *obj = lv_btn_create(parent_obj);
      objects.next_btn = obj;
      lv_obj_set_pos(obj, 384, 63);
      lv_obj_set_size(obj, 50, 50);
      lv_obj_add_event_cb(obj, action_show_next_image, LV_EVENT_PRESSED, (void *)0);
      {
        lv_obj_t *parent_obj = obj;
        {
          lv_obj_t *obj = lv_label_create(parent_obj);
          lv_obj_set_pos(obj, 0, 0);
          lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
          lv_label_set_text(obj, LV_SYMBOL_RIGHT);
          lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
      }
    }
    {
      // refreshBtn
      lv_obj_t *obj = lv_btn_create(parent_obj);
      objects.refresh_btn = obj;
      lv_obj_set_pos(obj, 384, 124);
      lv_obj_set_size(obj, 50, 50);
      lv_obj_add_event_cb(obj, action_refresh_browse_image, LV_EVENT_PRESSED, (void *)0);
      {
        lv_obj_t *parent_obj = obj;
        {
          lv_obj_t *obj = lv_label_create(parent_obj);
          lv_obj_set_pos(obj, 0, 0);
          lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
          lv_label_set_text(obj, LV_SYMBOL_REFRESH);
          lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
      }
    }
    {
      // delBtn
      lv_obj_t *obj = lv_btn_create(parent_obj);
      objects.del_btn = obj;
      lv_obj_set_pos(obj, 384, 194);
      lv_obj_set_size(obj, 50, 50);
      lv_obj_add_event_cb(obj, action_show_browse_msg_box, LV_EVENT_PRESSED, (void *)0);
      {
        lv_obj_t *parent_obj = obj;
        {
          lv_obj_t *obj = lv_label_create(parent_obj);
          lv_obj_set_pos(obj, 0, 0);
          lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
          lv_label_set_text(obj, LV_SYMBOL_TRASH);
          lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
      }
    }
    {
      // delAllBtn
      lv_obj_t *obj = lv_btn_create(parent_obj);
      objects.del_all_btn = obj;
      lv_obj_set_pos(obj, 384, 254);
      lv_obj_set_size(obj, 50, 50);
      lv_obj_add_event_cb(obj, action_show_browse_msg_box, LV_EVENT_PRESSED, (void *)1);
      lv_obj_set_style_bg_color(obj, lv_color_hex(0xfff32121), LV_PART_MAIN | LV_STATE_DEFAULT);
      {
        lv_obj_t *parent_obj = obj;
        {
          lv_obj_t *obj = lv_label_create(parent_obj);
          lv_obj_set_pos(obj, 0, 0);
          lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
          lv_label_set_text(obj, LV_SYMBOL_TRASH);
          lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
      }
    }
    {
      // brwImageOutline
      lv_obj_t *obj = lv_obj_create(parent_obj);
      objects.brw_image_outline = obj;
      lv_obj_set_pos(obj, 35, 63);
      lv_obj_set_size(obj, 322, 242);
      lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_border_color(obj, lv_color_hex(0xff2196f3), LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    {
      // brwImage
      browse_img_dsc.header.always_zero = 0;
      browse_img_dsc.header.cf = LV_IMG_CF_TRUE_COLOR;
      browse_img_dsc.header.w = GUI_BROWSE_IMAGE_WIDTH;
      browse_img_dsc.header.h = GUI_BROWSE_IMAGE_HEIGHT;
      browse_img_dsc.data_size = GUI_BROWSE_IMAGE_WIDTH * GUI_BROWSE_IMAGE_HEIGHT * 2;
      browse_img_dsc.data = (uint8_t *)gui_brw_canvas_buffer;
      lv_obj_t *obj = lv_img_create(parent_obj);
      objects.brw_image = obj;
      lv_img_set_src(obj, &browse_img_dsc);
      lv_obj_set_pos(obj, GUI_BROWSE_IMAGE_X, GUI_BROWSE_IMAGE_Y);
      lv_obj_set_size(obj, GUI_BROWSE_IMAGE_WIDTH, GUI_BROWSE_IMAGE_HEIGHT);
    }
    {
      lv_obj_t *obj = lv_label_create(parent_obj);
      objects.no_image_msg = obj;
      lv_obj_set_pos(obj, 113, 174);
      lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
      lv_label_set_text(obj, "No Images Found!");
      lv_obj_set_style_text_color(obj, lv_color_hex(0xffffc200), LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_text_font(obj, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    {
      // StorageLbl
      lv_obj_t *obj = lv_label_create(parent_obj);
      lv_obj_set_pos(obj, 35, 41);
      lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
      lv_label_set_text(obj, "/SD");
    }
    {
      // fileNameLbl
      lv_obj_t *obj = lv_label_create(parent_obj);
      objects.file_name_lbl = obj;
      lv_obj_set_pos(obj, 61, 41);
      lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
      lv_label_set_text(obj, "");
    }
    {
      // imgIndexLbl
      lv_obj_t *obj = lv_label_create(parent_obj);
      objects.img_index_lbl = obj;
      lv_obj_set_pos(obj, 261, 41);
      lv_obj_set_size(obj, 95, LV_SIZE_CONTENT);
      lv_label_set_text(obj, "");
      lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
    }
  }
}


void tick_screen_browse_screen() {
  {
    const char *new_val = get_var_file_name();
    const char *cur_val = lv_label_get_text(objects.file_name_lbl);
    if (strcmp(new_val, cur_val) != 0) {
      tick_value_change_obj = objects.file_name_lbl;
      lv_label_set_text(objects.file_name_lbl, new_val);
      tick_value_change_obj = NULL;
    }
  }
  {
    const char *new_val = get_var_img_index();
    const char *cur_val = lv_label_get_text(objects.img_index_lbl);
    if (strcmp(new_val, cur_val) != 0) {
      tick_value_change_obj = objects.img_index_lbl;
      lv_label_set_text(objects.img_index_lbl, new_val);
      tick_value_change_obj = NULL;
    }
  }
}

void create_screens() {
  lv_disp_t *dispp = lv_disp_get_default();
  lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), true, LV_FONT_DEFAULT);
  lv_disp_set_theme(dispp, theme);

  create_screen_main_screen();
  create_screen_settings_screen();
  create_screen_browse_screen();
}

typedef void (*tick_screen_func_t)();

tick_screen_func_t tick_screen_funcs[] = {
  tick_screen_main_screen,
  tick_screen_settings_screen,
  tick_screen_browse_screen,
};

void tick_screen(int screen_index) {
  tick_screen_funcs[screen_index]();
}

void invalidate_lepton_image() {
  lv_obj_invalidate(objects.lep_image);
}

void clear_lepton_image() {
  // clear lepton image
  uint16_t *ptr;
  ptr = gui_lep_canvas_buffer;
  while (ptr < (gui_lep_canvas_buffer + (GUI_LEPTON_IMAGE_WIDTH * GUI_LEPTON_IMAGE_HEIGHT))) {
    *ptr++ = 0;
  }
}

void invalidate_browse_image() {
  lv_obj_invalidate(objects.brw_image);
}

void clear_browse_image() {
  // clear lepton image
  uint16_t *ptr;
  ptr = gui_brw_canvas_buffer;
  while (ptr < (gui_brw_canvas_buffer + (GUI_BROWSE_IMAGE_WIDTH * GUI_BROWSE_IMAGE_HEIGHT))) {
    *ptr++ = 0;
  }
}

void hide_no_image_msg() {
  lv_obj_add_flag(objects.no_image_msg, LV_OBJ_FLAG_HIDDEN);
}

void show_no_image_msg() {
  lv_obj_clear_flag(objects.no_image_msg, LV_OBJ_FLAG_HIDDEN);
}

void update_dbg_msg() {
  lv_textarea_set_text(objects.debug_info_text_area, cam_dbg_msg_buf);
}


void show_settings_screen_msgbox(char *text) {
  lv_obj_t *obj = lv_msgbox_create(NULL, text, "", 0, false);
  objects.reboot_notif_box = obj;
  lv_obj_set_pos(obj, 100, 130);
  lv_obj_set_size(obj, 280, 60);
  lv_obj_add_flag(obj, LV_OBJ_FLAG_FLOATING);
  lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_CHAIN_HOR | LV_OBJ_FLAG_SCROLL_CHAIN_VER | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_WITH_ARROW | LV_OBJ_FLAG_SNAPPABLE);
  lv_obj_set_style_align(obj, LV_ALIGN_DEFAULT, LV_PART_MAIN | LV_STATE_DEFAULT);
}

void hide_settings_screen_msgbox() {
  lv_msgbox_close(objects.reboot_notif_box);
}