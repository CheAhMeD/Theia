/*
 * StatusLed.h - Library for interfacing WS2812B LED
 * using fastLed Library to show system status!
 *
 * Copyright 2025 Che AhMeD
 *
 * This file is part of Theia.
 *
 * Theia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Theia.  If not, see <https://www.gnu.org/licenses/>.
 *
 */
#ifndef StatusLed_h
#define StatusLed_h

#include <Arduino.h>

/// Predefined RGB colors
typedef enum {
  Blue = 0x0000FF,                  ///< @htmlcolorblock{0000FF}
  BlueViolet = 0x8A2BE2,            ///< @htmlcolorblock{8A2BE2}
  Cyan = 0x00FFFF,                  ///< @htmlcolorblock{00FFFF}
  DarkBlue = 0x00008B,              ///< @htmlcolorblock{00008B}
  DarkCyan = 0x008B8B,              ///< @htmlcolorblock{008B8B}
  Green=0x008000,                   ///< @htmlcolorblock{008000}
  GreenYellow=0xADFF2F,             ///< @htmlcolorblock{ADFF2F}
  Magenta = 0xFF00FF,               ///< @htmlcolorblock{FF00FF}
  Orange = 0xFFA500,                ///< @htmlcolorblock{FFA500}
  Purple = 0x800080,                ///< @htmlcolorblock{800080}
  Red = 0xFF0000,                   ///< @htmlcolorblock{FF0000}
  SkyBlue = 0x87CEEB,               ///< @htmlcolorblock{87CEEB}
  Turquoise = 0x40E0D0,             ///< @htmlcolorblock{40E0D0}
  Violet = 0xEE82EE,                ///< @htmlcolorblock{EE82EE}
  Wheat = 0xF5DEB3,                 ///< @htmlcolorblock{F5DEB3}
  White = 0xFFFFFF,                 ///< @htmlcolorblock{FFFFFF}
  Yellow = 0xFFFF00,                ///< @htmlcolorblock{FFFF00}
  YellowGreen = 0x9ACD32,           ///< @htmlcolorblock{9ACD32}
  Black = 0x000000,                 ///< @htmlcolorblock{050505}
} StatusColorCode;

// status type
typedef struct {
  char name[32];
  uint8_t code;
  StatusColorCode color;
} status_t;


class StatusLed 
{
  public:
    StatusLed(uint8_t initStatus);
    void init();
    void setStatus(uint8_t status_code);
    char* getStatusName();
  private:
    uint8_t _currentStatus;
    void update(StatusColorCode color);
};



#endif