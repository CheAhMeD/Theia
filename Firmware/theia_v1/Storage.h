/*
 * Storage.h - Wrapper for interfacing an SD Card
 * using ESP32 SD_MMC and FS libraries
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
#ifndef Storage_h
#define Storage_h

#include <Arduino.h>
#include <FS.h>
#include <SD_MMC.h>

class Storage
{
  public:
    Storage(int clkPin, int cmdPin, int d0Pin, int d1Pin, int d2Pin, int d3Pin);
    uint8_t init();
    void listDir(const char *dirname, uint8_t levels);
    void createDir(const char *path);
    void removeDir(const char *path);
    String readFile(const char *path);
    void writeFile(const char *path, const char *message);
    void writeRawFile(const char *path, uint8_t *payload, uint8_t size);
    void renameFile(const char *path1, const char *path2);
    void deleteFile(const char *path);
    sdcard_type_t cardType();
    uint64_t cardSize();
    uint64_t totalBytes();
    uint64_t usedBytes();
    int sectorSize();
    int numSectors();
    bool isInserted();
    uint8_t checkSpace();
    void checkLeptonDir();
    void checkTheiaFile();
    uint32_t getNbrOfFiles();
    void startBrowseSession();
    void pushImage(File img);
    void deleteCurrentImage();
    void deleteAllImages();
  private:
    int _clkPin;
    int _cmdPin;
    int _d0Pin;
    int _d1Pin;
    int _d2Pin;
    int _d3Pin;
    float bytesToFloat(uint8_t *farray);
};

extern fs::SDMMCFS Theia_SDCard;
extern char *current_image_path;

#endif