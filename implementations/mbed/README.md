# Eddystone for mbed
Eddystone beacons broadcast a small amount of information, such as URLs, to nearby devices that scan for them. If you'd like to learn more, please see the [Physical Web](http://physical-web.org) page.

This repo contains an fully functional Eddystone image that supports all 4 frame types (URL, UUID, TLM, and EID) as well as the complete Eddystone GATT configuration service.

To compile this image you'll need the [mbed toolchain from ARM](mbed.org). mbed is one of the most widely used embedded OS platforms. Unfortunately, we can't offer support for mbed or ARM tools. 

### Goal 1 - Lots of beacons
The first goal of this repo is to encourage a wide distribution of Eddystone beacon hardware with an open source version that anyone can freely use. If you do port this to your platform, please consider a pull request so others can compile to your hardware. There is already one comercial beacon using this image mde by [MinewTech](http://www.minewtech.com/eddystone.html) (we hope many more will follow)

### Goal 2 - Lots of devices
The second goal is to encourage other devices beyond just beacons. These would be interative devices that use new GATT services characteristics. We hope to see forks for products such as vending machines or remote control toys. With Bluetooth Javascript support in modern web browsers, it's important we have an easy way for device makers to not only broadcast a URL but to connect and control it directly.

### Porting the code
1. Edit Eddystone_config.h, most boards should work with changes only to this file.
2. There are #defines for each target board, just add your own #define to the list

**Note:** We've only compiled this for the Nordic chipsets as of Jan 2017. If you are using anything else, there will almost certainly be other changes to make throughout the code (such as persistent storage). We are encouraging new chip vendors to make these changes so you don't have to.

### Best Practices
* Be sure to test the advertised transmit power levels as each board's antenna is different.
* The URL for configuring the beacon, cf.physical-web.org is free for anyone to use. You don't need to change it.
* Offer a wide range of power levels so it's possible to broadcast only a short distance.
* Please don't default to high power. We don't want 'shouty' beacons
