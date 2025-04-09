#include <Arduino.h>
#include <FS.h>
#include <SD_MMC.h>
#include <EEPROM.h>
#include "Storage.h"
#include "Globals.h"
#include "vars.h"
#include "screens.h"
#include "esp_log.h"

static const char *TAG = "Storage";

fs::SDMMCFS Theia_SDCard = SD_MMC;
char *current_image_path;
uint16_t totalImages = 0;
uint16_t curImageIndex = 0;

Storage::Storage(int clkPin, int cmdPin, int d0Pin, int d1Pin, int d2Pin, int d3Pin) {
  _clkPin = clkPin;
  _cmdPin = cmdPin;
  _d0Pin = d0Pin;
  _d1Pin = d1Pin;
  _d2Pin = d2Pin;
  _d3Pin = d3Pin;
}

/* Initialize SD MMC with the selected pins */
uint8_t Storage::init() {
  //SDCard init (4 bit mode)
  if (!SD_MMC.setPins(_clkPin, _cmdPin, _d0Pin, _d1Pin, _d2Pin, _d3Pin)) {
    ESP_LOGE(TAG, "Pin change failed!");
    return 4;
  }
  ESP_LOGI(TAG, "Pin change successful! Starting storage...");

  // init browse screen image buffer
  gui_brw_canvas_buffer = (uint16_t *)heap_caps_malloc(GUI_BROWSE_IMAGE_WIDTH * GUI_BROWSE_IMAGE_HEIGHT * sizeof(uint16_t), MALLOC_CAP_SPIRAM);
  if (gui_brw_canvas_buffer == NULL) {
    ESP_LOGE(TAG, "malloc GUI Browse image buffer failed");
  }

  // init current image path buffer
  current_image_path = (char *)heap_caps_malloc(SAVED_FILE_PATH_MAX_CHARS, MALLOC_CAP_SPIRAM);

  // Start SD Card;
  return SD_MMC.begin("/sdcard", false, false, SDMMC_FREQ_52M, 5);
}

/* List directory in SD Card */
void Storage::listDir(const char *dirname, uint8_t levels) {
  ESP_LOGI(TAG, "Listing directory: %s", dirname);
  File root = SD_MMC.open(dirname);
  if (!root) {
    Serial0.printf("Failed to open directory %s\n", dirname);
    return;
  }
  if (!root.isDirectory()) {
    Serial0.printf("%s is not a directory!\n", dirname);
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial0.print("  DIR : ");
      Serial0.println(file.name());
      if (levels) {
        listDir(file.path(), levels - 1);
      }
    } else {
      Serial0.print("  FILE: ");
      Serial0.print(file.name());
      Serial0.print("  SIZE: ");
      Serial0.println(file.size());
    }
    file = root.openNextFile();
  }
}

/* Create a directory in SD Card */
void Storage::createDir(const char *path) {
  ESP_LOGI(TAG, "Creating Dir: %s", path);
  if (SD_MMC.mkdir(path)) {
    ESP_LOGI(TAG, "Dir created");
  } else {
    ESP_LOGE(TAG, "mkdir failed");
  }
}

/* Remove a directory from SD Card*/
void Storage::removeDir(const char *path) {
  ESP_LOGI(TAG, "Removing Dir: %s", path);
  if (SD_MMC.rmdir(path)) {
    ESP_LOGI(TAG, "Dir removed");
  } else {
    ESP_LOGE(TAG, "rmdir failed");
  }
}

/* Read a file content from SD Card */
String Storage::readFile(const char *path) {
  ESP_LOGI(TAG, "Reading file: %s", path);

  File file = SD_MMC.open(path);
  if (!file) {
    ESP_LOGE(TAG, "Failed to open file for reading");
    return "";
  }

  ESP_LOGI(TAG, "Reading from file... ");
  if (file.available())
    return file.readString();
  return "";
}

/* Print to a file in SD Card */
void Storage::writeFile(const char *path, const char *message) {
  ESP_LOGI(TAG, "Writing file: %s", path);

  File file = SD_MMC.open(path, FILE_WRITE);
  if (!file) {
    ESP_LOGE(TAG, "Failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    ESP_LOGI(TAG, "File written");
  } else {
    ESP_LOGE(TAG, "Write failed");
  }
  file.close();
}

/* Write a byte array to a file */
void Storage::writeRawFile(const char *path, uint8_t *payload, uint8_t size) {
  ESP_LOGI(TAG, "Writing file: %s", path);
  File rawfile = SD_MMC.open(path, FILE_WRITE);
  rawfile.write(payload, size);
  rawfile.close();
}


/* Rename a file in SD Card */
void Storage::renameFile(const char *path1, const char *path2) {
  ESP_LOGI(TAG, "Renaming file %s to %s", path1, path2);
  if (SD_MMC.rename(path1, path2)) {
    ESP_LOGI(TAG, "File renamed");
  } else {
    ESP_LOGE(TAG, "Rename failed");
  }
}

/* Delete a file from SD Card */
void Storage::deleteFile(const char *path) {
  ESP_LOGI(TAG, "Deleting file: %s\n", path);
  if (SD_MMC.remove(path)) {
    ESP_LOGI(TAG, "File deleted");
  } else {
    ESP_LOGE(TAG, "Delete failed");
  }
}

/* Card info (inherited) */
sdcard_type_t Storage::cardType() {
  return SD_MMC.cardType();
}
uint64_t Storage::cardSize() {
  return SD_MMC.cardSize();
}
uint64_t Storage::totalBytes() {
  return SD_MMC.totalBytes();
}
uint64_t Storage::usedBytes() {
  return SD_MMC.usedBytes();
}
int Storage::sectorSize() {
  return SD_MMC.sectorSize();
}
int Storage::numSectors() {
  return SD_MMC.numSectors();
}

/* Stupid way to check if the SD Card is removed */
bool Storage::isInserted() {
  SD_MMC.end();
  SD_MMC.begin();
  return SD_MMC.exists(STORAGE_THEIA_FILEPATH);
}

/* Check if the SD Card is full */
uint8_t Storage::checkSpace() {
  int spaceLeft;
  spaceLeft = (totalBytes() / (1024 * 1024)) - (usedBytes() / (1024 * 1024));
  if (spaceLeft <= 10) {
    // if less than 10MB left
    return 3;
  } else if (spaceLeft > 10 && spaceLeft <= 1000) {
    //if less than 1GB left
    return 2;
  } else {
    // Plenty of space left
    return 1;
  }
}

/* Check if Lepton dir exists otherwise create it */
void Storage::checkLeptonDir() {
  File lep_dir = SD_MMC.open(STORAGE_LEPTON_DIRPATH);
  if (!lep_dir.isDirectory()) {
    ESP_LOGI(TAG, "Lepton DIR doesn't exist! Creating...");
    createDir(STORAGE_LEPTON_DIRPATH);
  } else {
    ESP_LOGI(TAG, "Lepton DIR exists!");
  }
}

/* Check if .theia file exists otherwise create it */
void Storage::checkTheiaFile() {
  uint8_t payload[2] = { 7, 0 };  // dummy payload
  if (!SD_MMC.exists(STORAGE_THEIA_FILEPATH)) {
    ESP_LOGI(TAG, ".theia FILE doesn't exist! Creating...");
    writeRawFile(STORAGE_THEIA_FILEPATH, (uint8_t *)payload, 2);
  } else {
    ESP_LOGI(TAG, ".theia FILE exists!");
  }
}

/* get number of image files in STORAGE_LEPTON_DIRPATH */
uint32_t Storage::getNbrOfFiles() {
  uint32_t files = 0;
  File root = SD_MMC.open(STORAGE_LEPTON_DIRPATH);
  if (!root) {
    return 0;
  }
  File file = root.openNextFile();
  while (file) {
    if (!file.isDirectory()) {
      files++;
    }
    file = root.openNextFile();
  }
  root.close();
  return files;
}

/* Init browse Session for global use */
void Storage::startBrowseSession() {
  // Get all the files in STORAGE_LEPTON_DIRPATH
  totalImages = getNbrOfFiles();
  // NOTE: totalImages can be less than lastIndex (if we previously deleted an image file)
  curImageIndex = 1;
  // Set index on screen
  File root = SD_MMC.open(STORAGE_LEPTON_DIRPATH);
  File file = root.openNextFile();  // get the first file in dir
  if (!file) {
    // No files in dir
    clear_browse_image();
    invalidate_browse_image();
    set_var_img_index(0, 0);
    set_var_file_name("");
    show_no_image_msg();
    return;
  }
  // Hide message
  hide_no_image_msg();
  // Display first image
  pushImage(file);
  //File prevFile;
  set_var_img_index(curImageIndex, totalImages);

  while (get_var_act_scr_id() == SCREEN_ID_BROWSE_SCREEN) {
    // If navigate next is triggered
    if (get_var_browse_next_trig()) {
      file = root.openNextFile();
      pushImage(file);
      set_var_browse_next_trig(false);
      if (curImageIndex >= totalImages) {
        curImageIndex = totalImages;
      } else {
        curImageIndex++;
      }
      set_var_img_index(curImageIndex, totalImages);
    }
    // If rewind dir is triggered
    if (get_var_browse_refresh_trig()) {
      root.rewindDirectory();
      file = root.openNextFile();
      pushImage(file);
      set_var_browse_refresh_trig(false);
      curImageIndex = 1;
      set_var_img_index(curImageIndex, totalImages);
    }
    // If delete all is triggered
    if (get_var_delete_all()) {
      deleteAllImages();
      // reset the index
      EEPROM.writeUShort(PS_RAM_IMG_SUFFIX_ADDR, 0);
      EEPROM.commit();
      set_var_delete_all(false);
      curImageIndex = 1;
      totalImages = 0;
      clear_browse_image();
      invalidate_browse_image();
      set_var_img_index(0, 0);
      set_var_file_name("");
      show_no_image_msg();
    }
    // If delete image is triggered
    if (get_var_delete_image()) {
      deleteCurrentImage();
      set_var_delete_image(false);
      curImageIndex--;
      totalImages--;
      file = root.openNextFile();
      pushImage(file);
      set_var_img_index(curImageIndex, totalImages);
    }
  }
}

/* Display an image in gui_brw_canvas_buffer */
void Storage::pushImage(File img) {
  if (!img) return;
  strcpy(current_image_path, img.path());
  set_var_file_name(current_image_path);
  bool isBmp = String(img.name()).endsWith("bmp");
  if (isBmp) {
    // Draw BMP in gui_brw_canvas_buffer
    // Fill the PSRAM buffer
    uint16_t *ptr;
    ptr = gui_brw_canvas_buffer + (GUI_BROWSE_IMAGE_WIDTH * GUI_BROWSE_IMAGE_HEIGHT);
    // skip the hearder
    img.seek(54);
    // Read the image row by row from bottom to top
    for (int y = 0; y < GUI_BROWSE_IMAGE_HEIGHT; y++) {
      for (int x = 0; x < GUI_BROWSE_IMAGE_WIDTH; x++) {
        byte pixel[3];  // blue, green, red
        img.read((byte *)pixel, 3);
        // Convert to RGB565
        uint16_t color = ((pixel[2] & 0xF8) << 8) | ((pixel[1] & 0xFC) << 3) | (pixel[0] >> 3);
        // Store it in the correct position: flipping the Y-axis
        gui_brw_canvas_buffer[(GUI_BROWSE_IMAGE_HEIGHT - 1 - y) * GUI_BROWSE_IMAGE_WIDTH + x] = color;
      }
    }
    img.close();
  } else {
    // Draw RAW in gui_brw_canvas_buffer
    uint16_t *ptr = gui_brw_canvas_buffer;
    // get palette id and the max min
    uint8_t paletteNum = img.read();
    load_palette_raw(paletteNum);
    uint8_t tarray[4];
    double lepMax, lepMin;
    img.seek(2);
    img.read((byte *)tarray, 4);
    lepMax = bytesToFloat(tarray);
    img.seek(7);
    img.read((byte *)tarray, 4);
    lepMin = bytesToFloat(tarray);
    int x = 0;  // raw img x counter
    int y = 0;  // raw img y counter
    int z = 0;  // raw img z counter (to resize the image vertically)
    // position of image data in file
    // pos = 0 + 1 byte (palette) + 1 byte ('\n') + 4 bytes (lepMax) + 1 byte ('\n') + 4 bytes (lepMin) + 1 byte ('\n')
    img.seek(12);
    while (ptr < (gui_brw_canvas_buffer + (GUI_BROWSE_IMAGE_WIDTH * GUI_BROWSE_IMAGE_HEIGHT))) {
      int pos = x + (y * LEP_IMAGE_X);
      uint16_t pixel;
      img.seek(12 + pos * sizeof(pixel));
      img.read((uint8_t*)&pixel, sizeof(pixel));
      double val = (pixel - lepMin) / (lepMax - lepMin);
      uint16_t color = colormap_raw[(int)(255 * val)];
      x++;
      if (x > LEP_IMAGE_X - 1) {
        x = 0;
        z++;
      }
      if (z > LEP_IMAGE_RESIZE_FACTOR - 1) {
        z = 0;
        y++;
      }
      if (y > LEP_IMAGE_Y - 1) {
        y = 0;
      }

      // Draw the pixel twice (to resize the image horizontally)
      for (int i = 0; i < LEP_IMAGE_RESIZE_FACTOR; i++) *ptr++ = color;
    }
    img.close();
  }

  // force redraw the image
  invalidate_browse_image();
}

/* Converts four bytes back to float */
float Storage::bytesToFloat(uint8_t *farray) {
  union {
    float f;
    unsigned long ul;
  } u;
  u.ul = (farray[3] << 24) | (farray[2] << 16) | (farray[1] << 8) | (farray[0]);
  return u.f;
}

/* delete currently displayed image from STORAGE_LEPTON_DIRPATH */
void Storage::deleteCurrentImage() {
  const char *cur_img_path = get_var_file_name();
  SD_MMC.remove(cur_img_path);
}

/* delete all images from STORAGE_LEPTON_DIRPATH */
void Storage::deleteAllImages() {
  ESP_LOGI(TAG, "Deleting all images...");
  File root = SD_MMC.open(STORAGE_LEPTON_DIRPATH);
  if (!root) {
    ESP_LOGE(TAG, "Couldn't open LEpton Directory!");
    return;
  }

  // Delete all files in the root directory
  File file = root.openNextFile();
  if (!file) {
    // dir is empty?
    ESP_LOGI(TAG, "Lepton directory is already empty!");
    return;
  }

  while (file) {
    SD_MMC.remove("/Lepton/" + String(file.name()));
    file = root.openNextFile();
  }
  root.close();
  ESP_LOGI(TAG, "All images deleted!");
}
