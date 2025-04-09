#include <lvgl.h>
#include <ESP32Time.h>  // RTC for runtime
#include <EEPROM.h>     // Persistant Storage
#include <esp_heap_caps.h>
#include "Globals.h"
#include "Storage.h"
#include "Display.h"
#include "Lepton.h"
#include "StatusLed.h"
#include "Connection.h"
#include "ui.h"
#include "vars.h"

//runtime rtc;
ESP32Time rtc(0);  // offset in seconds GMT+1
// SD Card
Storage sdCard(SD_MMC_CLK, SD_MMC_CMD, SD_MMC_D0, SD_MMC_D1, SD_MMC_D2, SD_MMC_D3);
// Lepton Camera
Lepton lep(LEPTON_VoSPI_SCK, LEPTON_VoSPI_CS, LEPTON_VoSPI_MISO, LEPTON_RST_PIN, LEPTON_VSYNC_PIN);
// Status Led
StatusLed statusled(0);  // default status (No Error)

// Lepton camera
bool lep_rotate_180 = true;
uint8_t lep_version;
// TFT
uint8_t tft_brightness;
// SD Card
uint8_t sdCardState = 0;
const char *img_path = "/Lepton/IMG_";

// GUI
//lvgl buffers
static lv_disp_draw_buf_t lv_draw_buf;
static lv_color_t lv_line_buf[TFT_SCREEN_HEIGHT * 10];

// Tasks
TaskHandle_t gui_task;
TaskHandle_t lep_task;

//
uint32_t startTime_us;
uint32_t endTime_us;


// GUI Helpers
/* Display flushing */
void disp_flush_cb(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  tft.writePixels((lgfx::rgb565_t *)&color_p->full, w * h);
  tft.endWrite();

  lv_disp_flush_ready(disp);
}

/* LVGL Log Callback */
void print_log_cb(const char *text) {
  Serial0.println(text);
  Serial0.flush();
}

/* LVGL Read touch Callback */
void touchpad_read_cb(lv_indev_drv_t *indev_driver, lv_indev_data_t *data) {
  uint16_t touchX, touchY;
  bool touched = tft.getTouch(&touchX, &touchY);
  if (!touched) {
    data->state = LV_INDEV_STATE_REL;
  } else {
    data->state = LV_INDEV_STATE_PR;
    /*Set the coordinates*/
    data->point.x = touchX;
    data->point.y = touchY;
  }
}

/* LVGL RTC 1s Timer Callback */
static void lv_rtc_timer_1s_cb(lv_timer_t *timer) {
  LV_UNUSED(timer);
  set_var_curr_runtime(rtc.getTime("%H:%M:%S").c_str());
}

/* LVGL Sys 3s Timer Callback */
static void lv_sys_timer_3s_cb(lv_timer_t *timer) {
  LV_UNUSED(timer);
  // Check SD card only if in main screen and not saving a photo
  if ((get_var_act_scr_id() == 1) && !get_var_lep_snap_to_sd()) {
    if (sdCard.isInserted()) {
      // SD Card still inserted
      sdCardState = sdCard.checkSpace();
    } else {
      // SD Card removed
      sdCardState = 0;
    }
    set_var_sd_card_state(sdCardState);
  }
}

/* LVGL Init */
void lvgl_init() {
  lv_init();
  lv_disp_draw_buf_init(&lv_draw_buf, lv_line_buf, NULL, TFT_SCREEN_HEIGHT * 10);
  /* Initialize the display driver */
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = TFT_SCREEN_HEIGHT;
  disp_drv.ver_res = TFT_SCREEN_WIDTH;
  disp_drv.sw_rotate = 1;
  disp_drv.rotated = LV_DISP_ROT_NONE;
  disp_drv.flush_cb = disp_flush_cb;
  disp_drv.draw_buf = &lv_draw_buf;
  lv_disp_drv_register(&disp_drv);
  /* Initialize the (dummy) input device driver */
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = touchpad_read_cb;
  lv_indev_drv_register(&indev_drv);
  /* Initialize log callback */
  lv_log_register_print_cb(print_log_cb);
}

void sysTimerInit() {
  /* Initialize first 1s timer */
  lv_timer_t *rtc_timer_1s = lv_timer_create(lv_rtc_timer_1s_cb, 1000, NULL);
  lv_timer_ready(rtc_timer_1s);
  /* Initialize second 3s timer */
  lv_timer_t *sys_timer_3s = lv_timer_create(lv_sys_timer_3s_cb, 3000, NULL);
  lv_timer_ready(sys_timer_3s);
}

void setup() {
  // Initialize serial port
#if defined(USE_DEBUG_SERIAL)
  Serial0.begin(115200);
#endif
  // USB CDC
  Serial.begin(921600);
  Serial.setTxBufferSize(1024);
  // LED
  statusled.init();
  statusled.setStatus(0);
  // Init tft
  tft_init();
  // Init SD card
  sdCardState = sdCard.init();
  // Display Splash Screen and log text
  displayBootScreen();
  // EEPROM
  displayBootLogText("Starting Persistant Storage...");
  delay(500);
  EEPROM.begin(PS_RAM_SIZE);
  displayBootLogText("Checking SD Card...");
  delay(500);
  set_var_sd_card_state(sdCardState);
  if (sdCardState != 0) {
    // Check if lepton dir exists otherwise create it
    sdCard.checkLeptonDir();
    // Check if .theia file exists otherwise create it
    sdCard.checkTheiaFile();
  }
  //LVGL
  lvgl_init();
  displayBootLogText("LVGL Init Done...");
  delay(1000);

#if defined(USE_DEBUG_SERIAL)
  // SD Card tests
  if (sdCardState != 0) {
    Serial0.println("======= Testing SD Card ========");
    Serial0.printf("Total space: %lluMB\n", sdCard.totalBytes() / (1024 * 1024));
    Serial0.printf("Used space: %lluMB\n", sdCard.usedBytes() / (1024 * 1024));
    Serial0.println("====== End Testing SD Card =======");
    delay(1000);
  }
#endif
  // NOTE: Resetting the lepton causes all sort of sync loss....
  //lep.reset();

  displayBootLogText("Waiting for camera startup...");
  delay(1000);
  // Check if lepton is on
  while (!lep.isOn()) {}

  displayBootLogText("Camera boot complete...");
  delay(500);
  lep_version = lep.getPartNumber();
#if defined(USE_DEBUG_SERIAL)
  if (lep_version == LEP_3_5_SHUTTER) {
    Serial0.println("Lepton Version: 3.5 Shutter");
  } else {
    Serial0.println("Lepton Version: UNSUPPORTED");
  }
  Serial0.printf("Lepton Uptime: %d\n", lep.getUpTime());
  Serial0.printf("Lepton case temp: %2.2f°C\n", (lep.getAuxTemp() / 100) - 273.15);
  Serial0.printf("Lepton sensor temp: %2.2f°C\n", (lep.getFpaTemp() / 100) - 273.15);
  Serial0.flush();
#endif

  displayBootLogText("Starting System Tasks & Timers...");
  // Start lepton task
  xTaskCreatePinnedToCore(lepTask, "lep_task", 4096, NULL, 1, &lep_task, 0);
  // Start Gui Task
  xTaskCreatePinnedToCore(guiTask, "gui_task", 4096, NULL, 1, &gui_task, 1);
  sysTimerInit();
  // Turn the LED OFF
  statusled.setStatus(11);
  displayClear();
}

void lepTask(void *pvParameters) {
  // Theory
  // NOTE:  Valid Segment Timing (no loss of synchronization) = 1/106 sec =~ 9434 µs =~ 9.434 ms
  //        => Valid Frame Timing = 4*1/106 =~ 37.736 ms
  // However the rate of unique and valid frames is just below 9 Hz => Valid Frame Timing =~ 112 ms
  // Practice
  // NOTE:  Capture time (No Sync Loss) =~ 60ms
  //        Capture time (With Sync Loss) =~ 60ms - When Sync is lost + numberSyncLoss * SyncTime (n*300ms)
  //                                      => between 300 and 400 ms
  // NOTE:  Display time =~ 164ms

  // Init Hardware
  // Setup the VoSPI bus
  lep.init();
  vTaskDelay(pdMS_TO_TICKS(1000));  // wait 1s
  while (1) {
    // Only capture the image when in main screen (save some processing power?)
    if (get_var_act_scr_id() == 1) {
      lep.captureImage(lep_rotate_180);
      // TODO: only update the buffer when valid image is captured
      lep.updateRamBuffer();
      // If take a snap is triggered
      if (get_var_lep_snap_to_sd()) {
        bool success = false;
        bool isBmp = get_var_image_format();
        // get last file index from eeprom
        char *img_name;
        char index_buffer[5];
        // index is uint16_t
        uint16_t index = EEPROM.readUShort(PS_RAM_IMG_SUFFIX_ADDR) + 1;
        ultoa(index, index_buffer, 10);
        img_name = (char *)heap_caps_malloc(SAVED_FILE_PATH_MAX_CHARS, MALLOC_CAP_SPIRAM);
        strcpy(img_name, img_path);
        strcat(img_name, index_buffer);
        if (isBmp) {
          strcat(img_name, ".bmp");
          success = lep.saveBMPImage(img_name);

        } else {
          strcat(img_name, ".timg");
          success = lep.saveRAWImage(img_name);
        }
        heap_caps_free(img_name);
        create_save_notification(success, isBmp);
        if (success) {
          // Commit the new index on success
          EEPROM.writeUShort(PS_RAM_IMG_SUFFIX_ADDR, index);
          EEPROM.commit();
        }
        set_var_lep_snap_to_sd(false);
      }
      // If transfer raw data is triggered
      if (get_var_lep_transfer_raw()){
        lep.transferImage();
        set_var_lep_transfer_raw(false);
      }
      // If transfer lepton status is triggered
      if (get_var_lep_transfer_status()){
        lep.transferStatus();
        set_var_lep_transfer_status(false);
      }
    }
    // NOTE: Read Globals.h for wait period...
    vTaskDelay(pdMS_TO_TICKS(LEP_IMAGE_CAPTURE_PERIOD_MS));
  }
  vTaskDelete(NULL);
}

void guiTask(void *pvParameters) {
  // GUI Init
  ui_init();
  while (1) {
    lv_timer_handler(); /* let the GUI do its work */
    ui_tick();
  }
  vTaskDelete(NULL);
}

void loop() {
  // system:
  // check and update the GUI icons, messages ... etc
  /* Settings Screen handeling */
  if (get_var_lep_reboot()) {
    statusled.setStatus(1);
    lep.reboot();  // reboot the lepton
    set_var_lep_reboot(false);
    statusled.setStatus(11);
  }
  // if reset lepton is triggered
  if (get_var_lep_rst()) {
    statusled.setStatus(2);
    // Hard reset the camera
    lep.reset();
    set_var_lep_rst(false);
    statusled.setStatus(11);
  }
  // if set gain is triggered
  if (get_var_set_gain_trig()) {
    uint8_t gain = get_var_lepton_gain_mode();
    lep.setGainMode(gain);
    set_var_set_gain_trig(false);
  }
  // if get debug triggered
  if (get_var_dbg_info_trig()) {
    int i = 0;
    i = append_cam_revision(i);
    i = append_fw_version(i);
    i = append_sdk_version(i);
    i = append_lepton_info(i, lep_version,
                           ((lep.getFpaTemp() / 100) - 273.15),
                           ((lep.getAuxTemp() / 100) - 273.15));
    i = append_storage_info(i, get_var_sd_card_state() != 0,
                            sdCard.totalBytes() / (1024 * 1024),
                            sdCard.usedBytes() / (1024 * 1024),
                            sdCard.getNbrOfFiles());
    i = append_mem_info(i);
    i = append_copyright(i);
    set_var_dbg_info_trig(false);
    update_dbg_msg();
  }
  // run FFC is triggered
  if (get_var_run_ffc_trig()) {
    set_var_run_ffc_trig(false);
    lep.runFFC();
  }
  // If brightness changed
  if (tft_brightness != get_var_brightness_value()) {
    tft_brightness = get_var_brightness_value();
    setDisplayBrightness(tft_brightness * 255 / 100);
  }
  /* Browse Screen handeling */
  // if browse start triggered
  if (get_var_browse_start_trig()) {
    sdCard.startBrowseSession();
    set_var_browse_start_trig(false);
  }
  /* Serial comm handeling */
  if (commandReceived()) {
    handleCommand();
  }

  vTaskDelay(pdMS_TO_TICKS(50));  // wait 50 ms
}
