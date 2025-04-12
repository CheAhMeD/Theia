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
    // SD Card routines
    void sdCardCreateDir(const char *path);
    void sdCardRemoveDir(const char *path);
    String sdCardReadFile(const char *path);
    void sdCardWriteFile(const char *path, const char *message);
    void sdCardWriteRawFile(const char *path, uint8_t *payload, uint8_t size);
    void sdCardRenameFile(const char *path1, const char *path2);
    void sdCardDeleteFile(const char *path);
    sdcard_type_t sdCardType();
    uint64_t sdCardSize();
    uint64_t sdCardTotalBytes();
    uint64_t sdCardUsedBytes();
    int sdCardSectorSize();
    int sdCardNbrOfSectors();
    bool sdCardInserted();
    uint8_t sdCardCheckSpace();
    void sdCardCheckLeptonDir();
    void sdCardCheckTheiaFile();
    uint32_t sdCardGetNbrOfFiles();
    void startBrowseSession();
    void pushImage(File img);
    void sdCardDeleteCurrentImage();
    void sdCardDeleteAllImages();
    // Persistant Storage routines
    void psCommitImageIndex(uint16_t index);
    uint16_t psGetImageIndex();
    void psCommitByte(int addr, uint8_t value);
    uint8_t psGetByte(int addr);
    void psLoadInitValues();
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