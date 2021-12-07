# Packet-Sniffer-ESP32
Use an ESP32 to sniff WIFI packets

Instructions to setup and run program:
*Note* Use Arduino IDE on a Windows machine to flash program into ESP32 (it's easier). Use Linux machine for step 4 onwards

1. Install the Espressif ESP board libraries in the Arduino IDE. In the IDE click File -> Preferences -> "Additional Boards Manager URLs" and paste the link
   "https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json" without the quotes. Click Ok.
   
   Next Click on Tools -> Board -> Board manager (from the dropdown list) and search for "ESP". There should be a esp32 by Espressif Systems library. Install it.

   Now you will see a list of ESP32 boards in Tools -> Board -> ESP32 Arduino. Select "NodeMCU-32S" from the long list.

   Go to Tools -> Upload speed (baudrate) -> Select 921600.

2. Copy contents of Arduino Folder ('libraries' and 'packet_sniffer_cpre543') into your Documents\Arduino folder (or wherever arduino projects are stored).
   The libraries folder contains the 'Time' Library by Paul Stoffregen (https://github.com/PaulStoffregen/Time). This can be manually installed into the arduino libraries
   folder which is also in Documents\Arduino\libraries or searched through the built-in library manager in the Arduino IDE.


3. Connect your ESP32, open the 'packet_sniffer_cpre543.ino' and click upload (arrow pointing right at the top of the IDE). It compile the file and try to flash it into
   the board. You will see "Connecting ......___......" If it fails after waiting to connect for a while, try to upload it again but press the button on your ESP32 to the right
   side of the micro-usb port. This puts it in 'bootloader' mode and allows flashing. (But generally should flash automatically).

4. Boot up a linux machine. Now you have to install Wireshark and PySerial.

	Wireshark : sudo apt-get install wireshark (a prompt may show up later asking if you want non root users to access capture files, select NO as default)

	PySerial : install 'pip' using 'sudo apt-get install pip' and then 'sudo pip install pyserial'

5. Check which serial port the ESP32 module is connected to, should be tty0 by default.

6. Copy the SerialShark.py file into your Desktop or preferred location.

7. Run the program using 'sudo python3 SerialShark.py' follow the commands. Simply press enter to leave everything as default. If Serial port is successfully detected,
   you need to reset the ESP32 by pressing the button labeled "EN" on the board (to the right side of the micro-usb port).

   It should automatically start up Wireshark and display captured files.
