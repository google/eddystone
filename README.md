# Eddystone

<img align="left" src="res/logo/eddystone_logo.png" hspace="15" style="float: left">Eddystone is a protocol specification that defines a Bluetooth low energy (BLE) message format for proximity beacon messages. It describes several different frame types that may be used individually or in combinations to create beacons that can be used for a variety of applications.

## Design Goals

The design of Eddystone has been driven by several key goals:

- Works well with Android and iOS Bluetooth developer APIs
- Straightforward implementation on a wide range of existing BLE devices
- Flexible architecture permitting development of new frame types
- Fully compliant with the Bluetooth Core Specification

## Protocol Specification

The common frame PDU types and the individual service data byte layouts for
the Eddystone-UID, -URL and -TLM frame formats are documented in the
[Eddystone Protocol Specification](protocol-specification.md).

## Tools and Code Samples

We have a variety of tools and code samples to assist developers and implementors in working with Eddystone devices.

- An Android app that performs non-exhaustive basic validation of all the Eddystone frame types is available at [tools/eddystone-validator](tools/eddystone-validator).

- An Android app that can broadcast configurable Eddystone-UID frames is available at [eddystone-uid/tools/txeddystone-uid](eddystone-uid/tools/txeddystone-uid).

- A validation tool for the dedicated Eddystone-URL configuration service is at
[eddystone-url/tools/eddystone-url-config-validator](eddystone-url/tools/eddystone-url-config-validator).
