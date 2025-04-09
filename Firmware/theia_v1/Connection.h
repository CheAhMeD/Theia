/*
 * Connection.h - a module to handle commands sent over Serial (USB CDC)
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
/*
 If a Command from the list below is received
 this module is responsible of responding to the command 
 by creating a packet and sending it back over Serial

 Response Packet Format:
 - If command is invalid:
 +-------+-----+
 | Bytes |  1  |
 +-------+-----+
 | Value | NAK |
 +-------+-----+
 - If command is valid:
 +-------+
 | GET   |
 +-------+-----+-----+--- ... ---+------+
 | Bytes |  1  |  1  |  1 .. n   |   1  |
 +-------+-----+-----+--- ... ---+------+
 | Value | ACK | CMD |  PAYLOAD  | CRC8 |
 +-------+-----+-----+--- ... ---+------+
 Payload size depends on requested data.

 +-------+
 | SET   |
 +-------+-----+-----+-----+
 | Bytes |  1  |  1  |  1  |
 +-------+-----+-----+-----+
 | Value | ACK | CMD | S/F |
 +-------+-----+-----+-----+
 S/F byte determines success or failure.
 
*/

#ifndef CONNECTION_H
#define CONNECTION_H

// Serial Commands
#define CMD_ACK               0x0A
#define CMD_NAK               0x05
#define CMD_SUCCESS           0x01
#define CMD_FAILURE           0x00
// Get Commands       
#define CMD_GET_LEP_RAWDATA       0x44 // 'D'
#define CMD_GET_LEP_STATUS        0x73 // 's'
#define CMD_GET_LEP_DIAGNOSTICS   0x64 // 'd'
#define CMD_GET_STORAGE_INFO      0x69 // 'i'
#define CMD_GET_SPOT_TEMP         0x74 // 't'
#define CMD_GET_CAM_RUNTIME       0x52 // 'R'
// Set Commands
// No payload
#define CMD_LEP_REBOOT        0x42 // 'B'
#define CMD_LEP_RESET         0x72 // 'r'
#define CMD_LEP_RUN_FFC       0x46 // 'F'
// With payload
#define CMD_SET_COLORMAP      0x43 // 'C'
#define CMD_SET_LEP_GAIN      0x47 // 'G'
#define CMD_SET_SPOT_POS      0x70 // 'p'

bool commandReceived();
void handleCommand();
void transferLeptonRawData();
void transferLeptonDiagnostics();
void transferLeptonStatus();
void triggerLeptonReboot();
void triggerLeptonReset();
void triggerLeptonFFC();
void transferStorageInfo();
void transferSpotTemp();
void transferCamRunTime();
void updateColorMap(uint8_t cmap);
void updateLeptonGain(uint8_t gain);
void updateCursorPos(uint8_t px, uint8_t py);
// TODO: find a better way to do this
uint32_t number_of_files();


#endif /* CONNECTION_H */