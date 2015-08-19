# ARM/mbed Eddystone-URL Beacon

## Set up an ARM mbed account and compile for a target platform - just needs a browser
* Visit the [mbed](https://developer.mbed.org/account/login/?next=/) and create an account, then login.
* Select the compiler button in the top right hand corner of the web page
* From the line of buttons at the top select "Import" 
* Select 'click here' to import from URL
* Source URL: https://developer.mbed.org/users/roywant/code/mbed_EddystoneURL_Beacon/
* Import Name: mbed_EddystoneURL_Beacon
* Select "Import", and the program will appear in your Program Workspace
* Select your target hardware in the list defined in the page top-right button e.g. Nordic nRF51-Dongle
* Select "Compile" and when 'success' an mbed_EddystoneURL_Beacon.hex file will appear in your downloads
* Note: this is the same as if you had copied the files from this directory on GitHub into a new project - 
although perhaps more up-to-date with the latest changes.

## Program the target mbed hardware e.g. Nordic nRF51-Dongle on Windows, Linux, or Mac
* Insert nRF51-Dongle into a USB port
* It will create a filer window, just like plugging in a USB Flash drive
* Drag the  mbed_EddystoneURL_Beacon.hex file created above into that window. 
* It will be copied to the target hardware, and then reprogram it. Amazing eh!
* The hardware will automatically restart, and run the Eddystone-URL Beacon code.
