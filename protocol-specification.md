# Eddystone Protocol Specification

## Common Elements

Every Eddystone frame type must contain the following PDU data types:

- The _Complete List of 16-bit Service UUIDs_ as defined in The Bluetooth Core Specification Supplement (CSS) v5, Part A, § 1.1. The _Complete List of 16-bit Service UUIDs_ must contain the Eddystone Service UUID of `0xFEAA`. This is included to allow background scanning on iOS devices.
- The _Service Data_ data type, _Ibid._, § 1.11. The _Service Data - 16 bit UUID_ data type must be the Eddystone Service UUID of `0xFEAA`.

The specific type of Eddystone frame is encoded in the high-order four bits of the first octet in the Service Data associated with the Service UUID. Permissible values are:

Frame Type | High-Order 4 bits | Byte Value
:----------|:-----------------:|:---------:
UID | `0000` | `0x00`
URL | `0001` | `0x10`
TLM | `0010` | `0x20`
EID | `0011` | `0x30`
RESERVED | `0100` | `0x40`


The four low-order bits are reserved for future use and must be `0000`.

Note: although the core Bluetooth data type are defined in the standard as
little-endian, Eddystone's multi-value bytes defined in the Service Data are
all big-endian.

An example frame type may look like:

Byte offset | Value | Description | Data Type
-----|:-----:|-----------|------------
 0 | `0x02` | Length | Flags. CSS v5, Part A, § 1.3
 1 | `0x01` | Flags data type value
 2 | `0x06` | Flags data
 3 | `0x03` | Length | Complete list of 16-bit Service UUIDs. _Ibid._ § 1.1
 4 | `0x03` | Complete list of 16-bit Service UUIDs data type value
 5 | `0xAA` | 16-bit Eddystone UUID
 6 | `0xFE` | ...
 7 | `0x??` | Length | Service Data. _Ibid._ § 1.11
 8 | `0x16` | Service Data data type value
 9 | `0xAA` | 16-bit Eddystone UUID
10 | `0xFE` | ...

With subsequent bytes comprising the frame-specific Service Data.

The individual frame types are listed below.

## [Eddystone-UID](eddystone-uid)

The Eddystone-UID frame broadcasts an opaque, unique 16-byte Beacon ID composed of a 10-byte namespace and a 6-byte instance. The Beacon ID may be useful in mapping a device to a record in external storage. The namespace portion of the ID may be used to group a particular set of beacons, while the instance ID identifies individual devices in the group. The division of the ID into namespace and instance components may also be used to optimize BLE scanning strategies, e.g. by filtering only on the namespace.

## [Eddystone-URL](eddystone-url)

The Eddystone-URL frame broadcasts a URL using a compressed encoding format in order to fit more within the limited advertisement packet.

Once decoded, the URL can be used by any client with access to the internet. For example, if an Eddystone-URL beacon were to broadcast the URL `https://goo.gl/Aq18zF`, then any client that received this packet could choose to [visit that url](https://goo.gl/Aq18zF).

The Eddystone-URL frame forms the backbone of the [Physical Web](http://physical-web.org), an effort to enable frictionless discovery of web content relating to one’s surroundings. Eddystone-URL incorporates all of the learnings from the [UriBeacon](http://uribeacon.org) format from which it evolved.

## [Eddystone-TLM](eddystone-tlm)

The Eddystone-TLM frame broadcasts telemetry information about the beacon itself such as battery voltage, device temperature, and counts of broadcast packets.

## [Eddystone-EID](eddystone-eid)

The Eddystone-EID frame broadcasts an encrypted ephemeral identifier that changes periodically at a rate determined during the initial registration with a web service. The broadcast ephemeral ID can be resolved remotely by the service with which it was registered, but to other observers appears to be changing randomly. This frame type is intended for use in security and privacy-enhanced devices.
