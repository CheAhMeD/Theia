import serial
import time

## Replace port with Theia usb number
theia_port = 'COM15'


ser = serial.Serial(port=theia_port, 
                    baudrate=115200, 
                    parity=serial.PARITY_NONE,
                    stopbits=serial.STOPBITS_ONE,
                    bytesize=serial.EIGHTBITS,
                    xonxoff=False,
                    rtscts=True,
                    dsrdtr=True,
                    timeout=3)

get_commands = [("Get Lepton Status", b'\x73', 26), 
                ("Get Lepton Diagnostics", b'\x64', 11), 
                ("Get Storage Info", b'\x69', 24), 
                ("Get Spotmeter Temperature", b'\x74', 11), 
                ("Get Camera Runtime", b'\x52', 11)]

reboot_lep_cmd = (b'\x42', 3)
reset_lep_cmd  = (b'\x72', 3)
run_ffc_cmd    = (b'\x46', 3)

set_colormap_cmd = (b'\x43', 3)
set_lep_gain_cmd = (b'\x47', 3)

def getRawdata():
    CMD_GET_LEP_RAWDATA = (b'\x44', 38403)
    rawData = bytes()
    if ser and ser.is_open:
        # send command
        ser.write(CMD_GET_LEP_RAWDATA[0])
        # sleep(0.1)
        rawData = ser.read(CMD_GET_LEP_RAWDATA[1])
        print(list(rawData))
    
def getCommand(cmd:tuple):
    if ser and ser.is_open:
        ser.write(cmd[0])
        resp = ser.read(cmd[1])
        print(list(resp))
    
def setCommand(cmd:tuple, data:bytearray):
    if ser and ser.is_open:
        ser.write(cmd[0])
        ser.write(data)
        resp = ser.read(cmd[1])
        print(list(resp))


def getCommands():
    for command in get_commands:
        print("Running Command " + str(command[0]) + "...")
        ser.write(command[1])
        response = ser.read(command[2])
        print("Response: " + str(list(response)))
        time.sleep(1)

def rebootLepton():
    print("Rebooting Lepton...")
    getCommand(reboot_lep_cmd)
    print("Waiting for full reboot")
    for i in range(6):
        print(str(i+1))
        time.sleep(1)

def resetLepton():
    print("Resetting Lepton...")
    getCommand(reset_lep_cmd)

def runFFC():
    print("Running FFC...")
    getCommand(run_ffc_cmd)

def setCommands():
    print("Running Set Colormap command")
    cmap = int(input("Enter colormap id (0-11): "))
    if cmap not in range(0,12):
        print("Colormap id should be between 0 and 11...")
    else:
        setCommand(set_colormap_cmd, bytes([cmap]))
    
    print("Running Set Lepton Gain command")
    gain = int(input("Enter Lepton Gain (0: High, 1: Low, 2: Auto): "))
    if gain not in range(0,3):
        print("Gain should be between 0 and 2...")
    else:
        setCommand(set_lep_gain_cmd, bytes([gain]))


def print_menu():
    print("  ┌───────────────────────────────────────────────────┐")
    print("  │                                                   │")
    print("  │  [Theia Test Menu]                                │")
    print("  │  Uses Serial To Control Theia Thermal Camera      │")
    print("  │                                                   │")
    print("  │  1 - Get Raw Data                                 │")
    print("  │  2 - Get Commands                                 │")
    print("  │  3 - Reboot Lepton                                │")
    print("  │  4 - Reset Lepton                                 │")
    print("  │  5 - Run FFC                                      │")
    print("  │  6 - Set Commands                                 │")
    print("  │  0 - Exit                                         │")
    print("  │                                                   │")
    print("  └───────────────────────────────────────────────────┘")

def main_menu():
    while True:
        print_menu()
        choice = input("Enter your choice (0-6): ")
        if choice == "1":
            getRawdata()
        elif choice == "2":
            getCommands()
        elif choice == "3":
            rebootLepton()
        elif choice == "4":
            resetLepton()
        elif choice == "5":
            runFFC()
        elif choice == "6":
            setCommands()
        elif choice == "0":
            print("Exiting!")
            break
        else:
            print("Invalid choice. Please try again.")

if __name__ == "__main__":
    main_menu()
