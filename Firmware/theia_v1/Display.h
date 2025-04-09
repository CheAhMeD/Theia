#define LGFX_USE_V1

#include <LovyanGFX.hpp>
#include <FS.h>
#include <SD_MMC.h>
#include "Globals.h"

class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ILI9488IPS _panel_instance;  // LCD Driver
  lgfx::Bus_Parallel8 _bus_instance;    // Instantiate an 8-bit parallel bus (ESP32 only)
  lgfx::Light_PWM _light_instance;      // PWM backlight controller
  lgfx::Touch_NS2009 _touch_instance;   // Touch Controller

public:
  LGFX(void) {
    {
      auto cfg = _bus_instance.config();

      // 8 bit settings
      cfg.port = 0;               // Use I2S port? (0 or 1) (Uses ESP32's I2S LCD mode)
      cfg.freq_write = 20000000;  // Clock (maximum 20MHz, rounded to an integer division of 80MHz)
      cfg.pin_wr = TFT_WR;        // WR pin number
      cfg.pin_rd = TFT_RD;        // RD pin number
      cfg.pin_rs = TFT_RS;        // RS(D/C) pin number

      cfg.pin_d0 = TFT_D0;
      cfg.pin_d1 = TFT_D1;
      cfg.pin_d2 = TFT_D2;
      cfg.pin_d3 = TFT_D3;
      cfg.pin_d4 = TFT_D4;
      cfg.pin_d5 = TFT_D5;
      cfg.pin_d6 = TFT_D6;
      cfg.pin_d7 = TFT_D7;

      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }

    {
      auto cfg = _panel_instance.config();

      cfg.pin_cs = TFT_CS;  // CS pin number
      cfg.pin_rst = -1;     // RST pin number (-1 = disable)
      cfg.pin_busy = -1;    // BUSY pin number (-1 = disable)

      cfg.memory_width = TFT_SCREEN_WIDTH + 1;
      cfg.memory_height = TFT_SCREEN_HEIGHT + 1;
      cfg.panel_width = TFT_SCREEN_WIDTH;
      cfg.panel_height = TFT_SCREEN_HEIGHT;
      cfg.offset_x = 0;
      cfg.offset_y = 0;
      cfg.offset_rotation = 0;
      cfg.dummy_read_pixel = 8;
      cfg.dummy_read_bits = 1;
      cfg.readable = true;
      cfg.invert = true;
      cfg.rgb_order = false;
      cfg.dlen_16bit = false;
      cfg.bus_shared = true;

      _panel_instance.config(cfg);
    }

    {
      auto cfg = _light_instance.config();

      cfg.pin_bl = TFT_BKL;  // Backlight pin number
      cfg.invert = false;    // Active high or active low?
      cfg.freq = 44100;      // PWM Frequency
      cfg.pwm_channel = 7;   // PWM Channel

      _light_instance.config(cfg);
      _panel_instance.setLight(&_light_instance);
    }

    {
      auto cfg = _touch_instance.config();

      cfg.x_min = 0;            // Touch Calibration X min
      cfg.x_max = 319;          // Touch Calibration X max
      cfg.y_min = 0;            // Touch Calibration Y min
      cfg.y_max = 479;          // Touch Calibration Y max
      cfg.pin_int = TC_IRQ_PIN; // Touch Interrupt pin
      cfg.bus_shared = true;    // is I2C bus shared?
      cfg.offset_rotation = 0;  // Adjustment when display and touch orientation do not match. Set a value between 0 and 7.


      // I2C Bus
      cfg.i2c_port = 0;                    // I2C Port Number (0 or 1)
      cfg.i2c_addr = DEFAULT_NS2009_ADDR;  // I2C address (Touch Controller)
      cfg.pin_sda = BRD_I2C_SDA;           // SDA Pin number
      cfg.pin_scl = BRD_I2C_SCL;           // SCL Pin number
      cfg.freq = 400000;                   // I2C frequency

      _touch_instance.config(cfg);
      _panel_instance.setTouch(&_touch_instance);
    }

    setPanel(&_panel_instance);
  }
};

LGFX tft;
//static LGFX_Sprite lepImageSprite(&tft);

/* Init TFT*/
void tft_init(){
  tft.begin();
  tft.setRotation(TFT_SCREEN_ROTATION);
  tft.setBrightness(255);
  uint16_t calData[8] = { 194, 182, 175, 3910, 3830, 168, 3845, 3886 };
  tft.setTouchCalibrate(calData);
  tft.clear();
}

/* Display an image from SD */
// Should be called after Storage.init();
int displayImageFromSD(String filename, int x, int y, int posx, int posy) {
  File f = SD_MMC.open(filename, FILE_READ);
  if (!f) {
    Serial0.println("Failed to open logo for reading");
    f.close();
    return 0;
  }
  // skip the header
  f.seek(54);
  int X = x;
  int Y = y;

  uint8_t RGB[3 * X];
  for (int row = 0; row < Y; row++) {
    f.seek(54 + 3 * X * row);
    f.read(RGB, 3 * X);
    tft.pushImage(0 + posx, row + posy, X, 1, (lgfx::rgb888_t *)RGB);
  }

  f.close();
  return 0;
}

/* Display the Splash Screen */
void displayBootScreen() {
  //SD_MMC.begin();  // Start SD Card to get boot logo
  tft.fillScreen(TFT_BLACK);
  displayImageFromSD("/sys/splash_logo.bmp", GUI_LOGO_SIZE, GUI_LOGO_SIZE, GUI_LOGO_POS_Y, GUI_LOGO_POS_X);
  tft.setCursor(GUI_APP_NAME_POS_X, GUI_APP_NAME_POS_Y);
  tft.setFont(&fonts::DejaVu24);
  tft.print("Theia V1");
}

/* Display boot text */
void displayBootLogText(String text) {
  tft.setFont(&fonts::DejaVu12);
  tft.fillRect(GUI_BOOT_LOG_POS_X - 4, GUI_BOOT_LOG_POS_Y - 4, 442, 20, TFT_BLACK);
  tft.setCursor(GUI_BOOT_LOG_POS_X, GUI_BOOT_LOG_POS_Y);
  tft.print(text);
}

/* Helpers */
void displayFadeOutIn() {
  for (int i = 255; i >= 0; i = i - 5) {
    tft.setBrightness(i);
    delay(100);
  }
  for (int i = 0; i <= 255; i = i + 5) {
    tft.setBrightness(i);
    delay(20);
  }
}

void setDisplayBrightness(int value) {
  tft.setBrightness(value);
}

void displayClear() {
  tft.fillScreen(TFT_BLACK);
}
