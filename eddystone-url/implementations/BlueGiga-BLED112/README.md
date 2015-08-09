# BlueGiga BLED112 Eddystone-URL Beacon

## Installing the Windows BlueGiga Development Environment
* Install the BlueGiga Development Environment - you will need to create an account
at [BlueGiga](https://www.bluegiga.com/en-US/) - see the gear icon located at the top right 
of the web page.
* The project in this directory was built using the Bluegiga SDK 1.3.2-122.
* By default the install directory will be c:/Bluegiga/ble-1.3.2-122.
* Ensure the bin directory is on your windows path (Open the 'System Properties' dialog and
select the 'Environment Variables' button. Find the path variable 'Path' and add the
bluegiga bin directory path).

## Building and Flashing the BLED112 for the FIRST time
* Open a CMD tool window and change to the directory that you copied the BLED112 Eddystone-URL code.
* Compile it using:  bgbuild eddystone-url.bgproj 
* The project should build with no errors and create a new eddystone-url.hex directory.
* Plug-in the BLED112 dongle to a USB port.
* Use the BLE GUI-1.3.2-122 tool (installed with BlueGiga) and select the Commands/DFU menu.
* Use the 'Browse' button to navigate to the eddystone-url.hex file.
* Select 'Upload' and it will program the device code block by code block, showing the percentage complete
followed by 'Finished'.
* Unplug and replug the dongle and after 30s it should start transmitting an Eddystone-URL beacon.
* The default URL will be http://physical-web.org

## Reflashing the BLED112 
* On plugging-in the BLED112 USB dongle, windows will create a virtual COM port (see device manager to find the number).
For example on our PC is shows under Ports(COM & LPT) as COM12.
* Run a terminal emulator such as Putty.exe and connect to the COM serial port at 9600 baud.
* In the terminal window press 'enter', and it should respond with ' <-- enter ASCII 0 to reboot in dfu mode'.
* Type '0', and it will reboot in the DFU mode. You can now proceed, as described in Building and Flashing, from 
step 5.

## Changing the URL and other parameters
* Replug the dongle and within 30 seconds scan for BLE tags using your favourite scanner.
It should be visible with the name 'ES Change URL' - if several devices are present, use the RSSI 
values to figure out the nearest device.
* Connect to the device and you will see three services running.
* Select the 'Eddystone-URL Configuration Service' (GATT service) and you will see several characteristics listed.
* To change the URL you can read/write the 'Data' characteristic. Remember to use the compression codes
described in the open spec so the URL does not exceed 18 characters.
* Disconnect from the GATT service and you will see the new URL transmitted by the Eddystone-URL beacon.




