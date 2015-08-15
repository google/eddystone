# Eddystone-URL Configuration Service

*Revision v2r2*

## 1 Introduction

The Eddystone-URL Configuration Service should only be available during Configuration Mode (i.e. connectable) and **not** be available during regular URI Advertising Mode (i.e. **not** connectable). The Eddystone-URL Configuration Service allows setting Eddystone-URL fields and transmission characteristics. This information includes the following:

*   Lock code
*   URI
*   Flags
*   Transmit Power
*   Duty Cycle

This document is formatted according to the Bluetooth
[service](https://developer.bluetooth.org/TechnologyOverview/Pages/ANS.aspx) and
[profile](https://developer.bluetooth.org/gatt/profiles/Pages/ProfileViewer.aspx?u=org.bluetooth.profile.blood_pressure.xml) formatting styles.

### 1.1 Configuration Mode
A Configuration Mode is necessary because having a continuously connectable Eddystone-URL would allow any passerby to connect to the device and stop the Eddystone-URL from broadcasting. As a consequence a third party could launch a Denial of Service (DoS) attack by permanently connecting to the beacon. For this reason a Eddystone-URL should not be connectable during regular URL Advertising mode.

A beacon can be placed in Configuration Mode in order to be configurable. Configuration Mode should only be used by an administrator, and we recommend the following options to make it difficult for a random third-party to reconfigure proprietary beacons:

* A button that when pressed puts the beacon in Configuration Mode for a short period of time. The button could be placed inside the battery compartment or outside of the beacon if the beacon is placed out of reach for typical clients/users of the Eddystone-URL system
* A beacon could be placed in Configuration Mode automatically during a short period after power on (say 30 seconds) e.g. batteries are installed, or the beacon is connected to an external power source.

When in Configuration mode the beacon should advertise a different ADV packet indicating that mode using the ADV long name parameter string, and the ADV Service 128-bit UUID of the Eddystone-URL configuration service, and an ADV TxPower parameter to allow nearness of the beacon to be determined by the signal path-loss. It's recommend that the radio TxPower should also be increased to a Medium transmit power (typ. -2dBm) and this should also be reflected in the ADV TxPower parameter(typ. -6dBm, with a loss of 4dBm in the beacon antenna). Note: including all these parameters might result in the packet exceeding 31 bytes,  in which case a larger configuration packet size of 62 bytes can be achieved using the scan/response mechanism.

### 1.2 Service Dependencies

This service is not dependent upon any other [services](https://developer.bluetooth.org/gatt/services/Pages/ServicesHome.aspx).

### 1.3 Transport Dependencies

|Transport   | Supported |
|:-----------|-----------|
| Classic    | false	 |
| Low Energy | true      |

### 1.4 Return Codes

| Code   | Description                |
|:-------|:---------------------------|
| 0x00   | Success                    |
| 0x03   | Write Not Permitted        |
| 0x08   | Insufficient Authorization |
| 0x0d   | Invalid Attribute Length   |

## 2 Service Declaration

The assigned number for `<<Eddystone-URL Configuration Service>>` is

        ee0c2080-8786-40ba-ab96-99b91ac981d8

## 3 Service Characteristics

| Characteristic             | Ref. | Requirement  |
|:---------------------------:|:-----:|:--------------------:|
| Lock State                 | [3.1](#31-lock-state) | Mandatory|
| Lock                       | [3.2](#32-lock) | Optional|
| Unlock                     | [3.3](#33-unlock) | Optional|
| URI Data                   | [3.4](#34-uri-data) | Mandatory|
| URI Flags                  | [3.5](#35-flags)  | Mandatory|
| Advertised Tx Power Levels | [3.6](#36-advertised-tx-power-levels)  | Mandatory|
| Tx Power Mode              | [3.7](#37-tx-power-mode) | Mandatory |
| Beacon Period              | [3.8](#38-beacon-period)| Mandatory|
| Reset                      | [3.9](#39-reset) | Mandatory|



|Characteristic|Broadcast|Read|Write without Response|Write|Notify|Indicate|Signed Write|Reliable Write|Writable Auxiliaries|
|:---------------------------:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|Lock State                |X|M|X|X  |X|X|X|X|X|
|Lock                      |X|X|X|O\*|X|X|X|X|X|
|Unlock                    |X|X|X|O  |X|X|X|X|X|
|URI Data                  |X|M|X|M\*|X|X|X|X|X|
|URI Flags                 |X|M|X|M\*|X|X|X|X|X|
|Advertised Tx Power Levels|X|M|X|M\*|X|X|X|X|X|
|Tx Power Mode             |X|M|X|M\*|X|X|X|X|X|
|Beacon Period             |X|M|X|M\*|X|X|X|X|X|
|Reset                     |X|X|X|M\*|X|X|X|X|X|

\* Must be in unlock state.

### 3.1 Lock State

| Name        | Lock State                                  |
|:------------|:--------------------------------------------|
| UUID        | ee0c<b>2081</b>-8786-40ba-ab96-99b91ac981d8 |
| Description | Reads the lock state.                       |
| Type        | boolean                              |
| Lock State  | Either locked or unlocked.                  |

Read returns true if the device is locked.

### 3.2 Lock

| Name | Lock |
|:------------|:--------------------------------------------|
| UUID  | ee0c<b>2082</b>-8786-40ba-ab96-99b91ac981d8|
|  Description| Locks the beacon and sets the single-use lock-code. |
|  Type | uint128 |
|  Lock State | Must be unlocked. Will be locked after successful write.|

**Return Codes**

* [Insufficient Authorization](#14-return-codes) - for attempt with a valid length value when the beacon is locked. The exception is of course when attempting to Unlock the beacon with the correct key.
* [Insufficient Authorization](#14-return-codes) - for an attempt to write a characteristic with an invalid value, e.g. invalid length. [Invalid length](#14-return-codes) is also acceptable.

### 3.3 Unlock

| Name | Unlock |
|:------------|:--------------------------------------------|
| UUID  | ee0c<b>2083</b>-8786-40ba-ab96-99b91ac981d8|
|  Description| Unlocks the beacon and clears the single-use lock-code. |
|  Type | uint128 |
|  Lock State | Will be unlocked after successful write.|

**Return Codes**

* [Insufficient Authorization](#14-return-codes) - for an unlock attempt with an valid length incorrect key when the beacon is locked.
* [Invalid length](#14-return-codes) - for an unlock attempt with an invalid length whether the beacon is locked or not.
* Success - for an unlock attempt with a valid length key when the beacon is unlocked

### 3.4 URI Data

| Name | URI Data |
|:------------|:--------------------------------------------|
| UUID  | ee0c<b>2084</b>-8786-40ba-ab96-99b91ac981d8|
|  Description| Reads/writes the URI. |
|  Type | uint8[0..18]|
|  Lock State | For write, must be unlocked.|

The URI Data characteristic is a variable length structure. The first byte contains the [URI Scheme Prefix](../README.md#url-scheme-prefix).
The remaining bytes contain the [HTTP URL encoding](../README.md#eddystone-url-http-url-encoding).
Note: URI Data must be between 0 and 18 bytes in length.

### 3.5 Flags

| Name | Flags |
|:------------|:--------------------------------------------|
| UUID  | ee0c<b>2085</b>-8786-40ba-ab96-99b91ac981d8|
|  Description| Reads/writes the flags. |
|  Type | uint8 |
|  Lock State | For write, must be unlocked.|

The Flags characteristic is a single unsigned byte value. The top 4-bits are defined by the
Eddystone-URL Frame code as 0001, and the bottom 4-bits are reserved flags for future use, and must
be set to 0000. As a result, the characteristic currently has a fixed value of 0x10, and although it is
a read/write value, it should not be changed. When the lower 4-bit flags are defined,
this documentation will be updated, and the flag values can be written accordingly.

### 3.6 Advertised TX Power Levels

| Name | Advertised TX Power Levels |
|:------------|:--------------------------------------------|
| UUID  | ee0c<b>2086</b>-8786-40ba-ab96-99b91ac981d8|
|  Description| Reads/writes the Advertised Power Levels array. |
|  Type | int8[4] |
|  Lock State | For write, must be unlocked.|

This characteristic is a fixed length array of values, in dBm, to be included in the
[Eddystone-URL TX Power Level](../README.md#tx-power-level) field of the advertisement when that mode is active. The index into the array is [TX Power Mode](#37-tx-power-mode).

**Note to developers:**
The Advertised TX Power Levels is not the same as values set internally into the radio tx power. You need to implement an internal array indexed by TX Power Mode that is used for the internal facing radio setting.

The Advertised TX Power Levels is also indexed by TX Power Mode but is outward facing, and is the value that is put into the adv packet. These are related but distinct because numbers used by radio function may not be the same as those exposed in adv packets.

The best way to determine the precise value for these output values is to measure the actual RSSI from your beacon from 1 meter away and then add 41dBm to that. 41dBm is the signal loss that occurs over 1 meter.

### 3.7 TX Power Mode

| Name | TX Power Mode |
|:------------|:--------------------------------------------|
| UUID  | ee0c<b>2087</b>-8786-40ba-ab96-99b91ac981d8|
|  Description| Reads/writes the TX Power Mode. |
|  Type | uint8 |
|  Lock State | For write, must be unlocked.|

Sets the transmission power mode to one of:

| Power Level | Value |
|:-----|:----------|
| TX_POWER_MODE_HIGH | 3 |
| TX_POWER_MODE_MEDIUM | 2 |
| TX_POWER_MODE_LOW | 1 |
| TX_POWER_MODE_LOWEST | 0 |

**Return Codes**

* [Write Not Permitted](#14-return-codes) - for an attempt to write invalid values.

### 3.8 Beacon Period

| Name        | Beacon Period                               |
|:------------|:--------------------------------------------|
| UUID        | ee0c<b>2088</b>-8786-40ba-ab96-99b91ac981d8 |
| Description | The period in milliseconds that a Eddystone-URL packet is transmitted. |
| Type        | uint16                                      |
| Lock State  | For write, must be unlocked.                |

The period in milliseconds that a Eddystone-URL packet is transmitted. **A value of zero disables Eddystone-URL transmissions.** Setting a period value that the hardware doesn't support should default to minimum value the hardware supports. For example if the user tries to set the period to 1 millisecond and the lowest value the hardware supports is 100 milliseconds, the value for the characteristic should be set to 100 milliseconds and the Eddystone-URL should broadcast at a 100 milliseconds.

### 3.9 Reset

| Name        | Reset                                       |
|:------------|:--------------------------------------------|
| UUID        | ee0c<b>2089</b>-8786-40ba-ab96-99b91ac981d8 |
| Description | Reset to default values.                    |
| Type        | boolean                                     |
|  Lock State | Must be unlocked.                           |

Writing a non-zero value to this characteristic will set all characteristics to their initial values:

| Characteristic | Default Value |
|:---------------|:--------------|
| URI Data | vendor specified |
| URI Flags | 0 |
| TX Power Mode | TX_POWER_MODE_LOW |
| Beacon Period | 1000 (1 second) |
| Lock | 00000000-00000-0000-0000-000000000000 |

### 4 Reserved Characteristics

The following Characteristic UUIDs are are reserved for future use:

| UUID |
|:----------|
|ee0c<b>2090</b>-8786-40ba-ab96-99b91ac981d8|

## Validation Tool

- An Android app that performs validation of the Eddystone-URL configuration service is available at [eddystone-url-config-validator](../tools/eddystone-url-config-validator/).
