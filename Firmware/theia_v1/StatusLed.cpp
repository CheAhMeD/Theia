#include <Arduino.h>
#include <FastLED.h>
#include "StatusLed.h"
#include "Globals.h"
#include "esp_log.h"

static const char *TAG = "StatusLed";

CRGB statusLED[1];

static status_t statusList[12] = {
  { .name  = "No Error",
    .code  = 0,
    .color = Green},
  { .name  = "Lepton Rebooting",
    .code  = 1,
    .color = Blue},
  { .name  = "Lepton Hard Reset",
    .code  = 2,
    .color = BlueViolet},
  { .name  = "Lepton Sync Error",
    .code  = 3,
    .color = DarkBlue},
  { .name  = "ERROR4",
    .code  = 4,
    .color = Orange},
  { .name  = "ERROR5",
    .code  = 5,
    .color = Yellow},
  { .name  = "ERROR6",
    .code  = 6,
    .color = YellowGreen},
  { .name  = "ERROR7",
    .code  = 7,
    .color = GreenYellow},
  { .name  = "ERROR8",
    .code  = 8,
    .color = Magenta},
  { .name  = "Generic Error",
    .code  = 9,
    .color = Red},
  { .name  = "Led On",
    .code  = 10,
    .color = White},
  { .name  = "Led Off",
    .code  = 11,
    .color = Black},
};

StatusLed::StatusLed(uint8_t initStatus) {
  _currentStatus = initStatus;
}

/* Initialize WS2812B */
void StatusLed::init() {
  // Status LED
  FastLED.addLeds<WS2812B, LED_DIN, GRB>(statusLED, 1);
  update(statusList[_currentStatus].color);
}

void StatusLed::update(StatusColorCode color){
  FastLED.clear();
  FastLED.show();
  statusLED[0] = color;
  FastLED.show();
}

void StatusLed::setStatus(uint8_t status_code){
  if (status_code < 0 | status_code > 11) {
    _currentStatus = 11;
  } else {
    _currentStatus = status_code;
  }
  update(statusList[_currentStatus].color);
}
char* StatusLed::getStatusName() {
  return statusList[_currentStatus].name;
}