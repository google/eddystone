# Eddystone-UID

The Eddystone-UID frame broadcasts an opaque, unique 16-byte Beacon ID composed of a 10-byte namespace and a 6-byte instance. The Beacon ID may be useful in mapping a device to a record in external storage. The namespace portion of the ID may be used to group a particular set of beacons, while the instance portion of the ID identifies individual devices in the group. The division of the ID into namespace and instance components may also be used to optimize BLE scanning strategies, e.g. by filtering only on the namespace.

## Frame Specification

The UID frame is encoded in the advertisement as a Service Data block associated with the Eddystone service UUID. The layout is:

Byte offset | Field | Description
------------|-------|------------
0 | Frame Type | Value = `0x00`
1 | Ranging Data | Calibrated Tx power at 0 m
2 | NID[0] | 10-byte Namespace
3 | NID[1]
4 | NID[2]
5 | NID[3]
6 | NID[4]
7 | NID[5]
8 | NID[6]
9 | NID[7]
10 | NID[8]
11 | NID[9]
12 | BID[0] | 6-byte Instance
13 | BID[1]
14 | BID[2]
15 | BID[3]
16 | BID[4]
17 | BID[5]
18 | RFU | Reserved for future use, must be`0x00`
19 | RFU | Reserved for future use, must be`0x00`

All multi-byte values are big-endian.

## Field Notes

- The length of this frame is fixed and takes up the full 31 bytes of the ADV packet. The value of the Service Data Length byte must be `0x17`. Existing UID implementations that truncate the frame to omit the RFU bytes will use `0x15`, but in future should include the RFU bytes and the full length.
- The Ranging Data is the Tx power in dBm emitted by the beacon at 0 meters. Note that this is different from other beacon protocol specifications that require the Tx power to be measured at 1 m. The value is an 8-bit integer as specified by the [Tx Power Level Characteristic](https://developer.bluetooth.org/gatt/characteristics/Pages/CharacteristicViewer.aspx?u=org.bluetooth.characteristic.tx_power_level.xml) and ranges from -100 dBm to +20 dBm to a resolution of 1 dBm. See Transmit Power below for more details.
- The 10-byte Namespace ID component is unique self-assigned beacon ID namespace. See UID Construction below for recommendations on generating this unique namespace.
- The 6-byte Instance ID component is unique within the namespace. See UID Construction below for recommendations on ensuring beacon ID uniqueness.

## Tx Power

Tx power is the received power at 0 meters measured in dBm. The value may range from -100 dBm to +20 dBm at a resolution of 1 dBm.

Note to developers: the best way to determine the precise value to put into this field is to measure the actual output of your beacon from 1 meter away and then add 41 dBm to that. 41dBm is the signal loss that occurs over 1 meter.

## UID Construction

An Eddystone-UID beacon ID is 16 bytes long, consisting of a 10-byte namespace component and 6-byte instance component. The namespace is intended to ensure ID uniqueness across multiple Eddystone implementers and may be used to filter on-device scanning for beacons. We recommend two methods for generating a unique 10-byte namespace: a truncated hash of your FQDN, or an elided UUID.

### Truncated Hash of FQDN

Produce a SHA-1 hash of a fully-qualified domain name that you own. If you desire additional obscurity and/or additional namespaces, you may wish to use a random subdomain under this FQDN. Select the first 10 bytes from that hash.

### Elided Version 4 UUID

Generate a version 4 UUID then remove bytes 5 - 10 (inclusive). For example: from this UUID: 8b0ca750-_e7a7-4e14-bd99_-095477cb3e77 remove these bytes: e7a7-4e14-bd99. This produces the following namespace: 8b0ca750095477cb3e77.

This option may be useful if you interleave Eddystone-UID with other UUID-based frame formats. It allows you to relate your Eddystone-UID namespace to the UUID in the other frames by deriving one from the other.

### Instance ID

The 6-byte instance portion of the Eddystone-UID beacon ID may be assigned via any method suitable for your application. They may be random, sequential, hierarchical, or any other scheme.
