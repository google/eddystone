## Setup for TI SimpleLink Bluetooth Smart Development Environment
* Register for a myTI user account at www.ti.com. This is needed to obtain TI SDKs and tools.
* Download [CC2640 Bluetooth low energy Software Developer's Guide](www.ti.com/litv/pdf/swru393).
* Obtain [Code Composer Studio IDE v6](http://www.ti.com/tool/ccstudio).
* Obtain [BLE-Stack 2.1 SDK](http://www.ti.com/tool/ble-stack).
* Install BLE-Stack 2.1, CCS IDE and/or optionally IAR Workbench, following the instructions in Section 2.5 of CC2640 Bluetooth low energy Software Developer's Guide.
* Purchase [CC2650 Development Kit](http://www.ti.com/tool/CC2650DK) or [SimpleLink Bluetooth Smart SensorTag](http://www.ti.com/tool/cc2650stk) and [SimpleLink SensorTag Debugger DevPack](http://www.ti.com/tool/cc-devpack-debug)
* The project in this directory was built using the TI BLE-Stack 2.1.
* The application project files and the stack project files for CCS environment are located in SimpleEddystoneBeacon\CC26xx\CCS\SimpleEddystoneBeacon and SimpleEddystoneBeacon\CC26xx\CCS\SimpleEddystoneBeacon respectively.
* The workspace file encompassing the application prject and the stack project for IAR environment is located in SimpleEddystoneBeacon\CC26xx\IAR.
* CCS 6.1 or IAR 7.40.2 can build the projects.
* The default install directory of the BLE-Stack 2.1 is C:\ti\simplelink\ble_cc26xx_2_01_00_44423.
  If it is installed in a different path, the value of TI_BLE_SDK variable should be changed accordingly. With CCS, TI_BLE_SDK is found at Properties->Resource->Linked Resources->Path Variables when SimpleEddystoneBeacon project is opened. For IAR, TI_BLE_SDK is found in SimpleEddystoneBeacon.custom_argvars.
* The project directory can be copied into c:\ti\simplelink\ble_cc26xx_2_01_00_44423\Projects\ble\.
* SensorTag configurations are combined with OAD as well. Before building SensorTag configurations, read *CC2640 BLE OAD User's Guide*, especially Chapter 9 carefully. *CC2640 BLE OAD User's Guide* can be found at C:\ti\simplelink\ble_cc26xx_2_01_00_44423\Documents.

## Operations and Usages
* The system will start with UID frame advertising.
* Transitions between UID frame advertising and URL frame advertising are toggled by RIGHT button press.
* TLM frames are sent every 10th advertising of UID or URL frames by default.
* Transitions between URL frame advertising mode and URL Configuration mode are toggled by LEFT button press.

---
_More information is available at http://processors.wiki.ti.com/index.php/CC26xx_Eddystone_Beacon_Implementation._