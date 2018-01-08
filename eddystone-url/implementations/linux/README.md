# Url advertiser for linux

A set of python scripts for scanning and advertising urls over Eddystone-URL.

## Requirements

    Linux
    Python 3
    bluez

## Installation

    sudo apt-get install bluez

## Usage

    # Advertising a url
    ./advertise-url -u http://your/url

    # Stopping the advertisement
    ./advertise-url -s

    # Running a single scan for urls
    ./scan-for-urls -s

    # A continuous scan for urls
    ./scan-for-urls

    # A continuous scan and resolving short urls to long urls
    ./scan-for-urls | ./resolve-urls -u [URLs]
