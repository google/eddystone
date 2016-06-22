# PyBeacon
Python script for scanning and advertising urls over Eddystone-URL.

**Note**: This is just a stable build. For latest builds, check [Nirmanakarta/PyBeacon](https://github.com/nirmankarta/PyBeacon).

## Requirements

* Python 3.x (Scanning will not work on Python 2.x)
* Bluez
    * sudo apt-get install bluez bluez-hcidump

## Installation

    sudo pip install PyBeacon

## Upgrade

    sudo pip install PyBeacon --upgrade

## Usage
	PyBeacon [-h] [-u [URL]] [-s] [-t] [-o] [-v] [-V]

	optional arguments:
		-h, --help            show this help message and exit
		-u [URL], --url [URL] URL to advertise.
		-s, --scan            Scan for URLs.
		-t, --terminate       Stop advertising URL.
		-o, --one             Scan one URL only.
		-v, --version         Version of PyBeacon.
		-V, --verbose         Print lots of debug output.