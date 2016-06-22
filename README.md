# Eddystone

<img align="left" src="res/logo/eddystone_logo.png" hspace="15" style="float: left">Eddystone is a protocol specification that defines a Bluetooth low energy (BLE) message format for proximity beacon messages. It describes several different frame types that may be used individually or in combinations to create beacons that can be used for a variety of applications.

Announced in April 2016, [Eddystone-EID](eddystone-eid) (Ephemeral ID) is a new frame type that defines a cryptographically secure method of configuring a beacon to broadcast information that only authorized people may decrypt. It includes support for a [secure transmission](eddystone-tlm/tlm-encrypted.md) of the TLM (Telemetry) information.

## Design Goals

The design of Eddystone has been driven by several key goals:

- Works well with Android and iOS Bluetooth developer APIs
- Straightforward implementation on a wide range of existing BLE devices
- Flexible architecture permitting development of new frame types
- Fully compliant with the Bluetooth Core Specification

## Protocol Specification

The common frame PDU types and the individual service data byte layouts for
the Eddystone frame formats are documented in the
[Eddystone Protocol Specification](protocol-specification.md).

## Configuration Service

Eddystone defines a [GATT configuration service](configuration-service/) to enable interoperability between hardware manufacturers and application developers. It allows the beacon to report its capabilities to apps, and for the beacon's broadcast data to be reconfigured. This service is also necessary for secure configuration and registration as an Eddystone-EID beacon.

## Tools and Code Samples

We have a variety of tools and code samples to assist developers and implementors in working with Eddystone devices.

- An Android app that performs non-exhaustive basic validation of the core Eddystone frame types is available at [tools/eddystone-validator](tools/eddystone-validator).

- An Android app that can broadcast configurable Eddystone-UID frames is available at [eddystone-uid/tools/txeddystone-uid](eddystone-uid/tools/txeddystone-uid).

- A validation tool for the dedicated Eddystone-URL configuration service is at
[eddystone-url/tools/eddystone-url-config-validator](eddystone-url/tools/eddystone-url-config-validator).

- A tool to aid in the implementation of Eddystone-EID devices is at [eddystone-eid/tools/](eddystone-eid/tools)
