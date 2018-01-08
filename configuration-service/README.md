# Eddystone Configuration GATT Service

This document defines the specification for the Eddystone Configuration Service and offers some implementation guidance.

The Eddystone Configuration Service runs as a GATT service on the beacon while it is connectable and allows configuration of the advertised data, the broadcast power levels, and the advertising intervals. It also forms part of the definition of how Eddystone-EID beacons are configured and registered with a trusted resolver.

## Implementation notes for beacon manufacturers

The primary goal of a standardized configuration service is interoperability between beacon manufacturers and software developers. Your beacons, once received by the person who will perform the initial configuration, must be able to indicate to an application that they are ready for use. (And when the factory reset characteristic is written successfully, the beacon must return to this initial state.)

There are two ways to accomplish this. Your choice depends on whether your beacons are always connectable, or if they are non-connectable by default and become connectable through some action by the user.

We recommend that your beacons spend the majority of their lives in a non-connectable state, which avoids network congestion and collision issues in a dense wireless environment. In this case, the user will take some action to make a beacon temporarily connectable, perhaps by depressing a button, pulling a battery activation tab, or similar. The connectable timeout should be at the very least 30 seconds to let enough time to user to start a configuration application. Once your beacon becomes connectable, we recommend broadcasting the configuration GATT service's UUID as a 128-bit service UUID in the scan response, along with a local name data type that identifies your beacon. Configuration applications may look for both of these signals in the scan records of nearby beacons and use the information to highlight which ones are ready for configuration. (A flash of an LED or an audible beep upon connection from a client are helpful too, if your hardware has the necessary support.) 

If your beacons must be always connectable, you may wish to minimize the amount of data that's broadcast in a scan response to conserve the battery. In this case we recommend that in an occasional scan response you broadcast a short local name so that configuration clients can display it to the user and aid an easier association between your beacon and any other nearby beacons. The time between scan responses should be tuned to balance the power requirements with the patience of a human trying to find a connectable frame to configure your beacon -- once the hardware device address has been obtained it should be possible to connect to the beacon in a few seconds. (Here too, a flash of an LED or an audible beep upon connection from a client may be helpful, if your hardware has the necessary support, especially if there are multiple connectable beacons in the vicinity.)

In either case, users may find it helpful if your beacons arrive broadcasting one of the following two frames:

* An Eddystone-URL frame type whose URL points to information about using your beacons. We recommend this for ease of recognition by the person who is configuring the device.
* An Eddystone-UID frame type where the namespace part of the UID is fixed to the one you've chosen for your organization and the instance part is randomized. This ensures the beacon is ready to be registered with a service that supports the UID format with no configuration necessary.

### Default unlock code
If a beacon ships in a `0x02` unlocked state, care must be taken to ensure that it has a default unlock code that's known to the owner.

### Single connection only
While a configuration client is connected to the beacon, the beacon must refuse other connections.

### Interleaving multiple advertisement frames.
If variable advertising intervals are supported (see `IS_VARIABLE_ADV_SUPPORTED` in the Capabilities characteristic) it will be inevitable that some slots will be scheduled to broadcast at the same time. We recommend that implementers transmit those frames in slot order at the shortest permissible advertising interval (100 ms).

## Specification

<table>
<tr>
<td><strong>Service&nbsp;Definition</strong></td><td/>
</tr>
<tr>
<td>Name</td>
<td>Eddystone Configuration Service</td>
</tr>
<tr>
<td>Service UUID</td>
<td>a3c8<strong>7500</strong>-8ed3-4bdf-8a39-a01bebede295</td>
</tr>
<tr>
<td>Notes</td>
<td>Where not explicitly stated, data written and read is defined in terms of big-endian arrays of bytes. However, there are exceptions for EID related keys: the Public ECDH Key in Characteristic 8, the Identity Key in Characteristic 9, and the service's public ECDH key when writing an EID slot in Characteristic 10, all three of which are little-endian. This exception exists because the reference design for eliptic curve-25519, which has been widely adopted, was implemented using little-endian arithmetic and is the basis for the EID design. Note: the elliptic curve-25519 reference design is little-endian, even though big-endian is used almost universally for all other cryptographic protocols</td>
</tr>
</table>

<table>
<tr><td><strong>Characteristic 1</strong></td><td/></tr>
<tr><td>Characteristic&nbsp;UUID</td><td>a3c8<strong>7501</strong>-8ed3-4bdf-8a39-a01bebede295</td></tr>
<tr>
<tr><td>Name</td><td>Capabilities</td></tr>
<td>Properties</td>
<td>
byte array (read) {<br>
&nbsp;&nbsp;version_byte,<br>
&nbsp;&nbsp;max_supported_total_slots,<br>
&nbsp;&nbsp;max_supported_eid_slots,<br>
&nbsp;&nbsp;capabilities_bit_field,<br>
&nbsp;&nbsp;supported_frame_types_bit_field[0], Note: high-bits<br>
&nbsp;&nbsp;supported_frame_types_bit_field[1], Note: low-bits<br>
&nbsp;&nbsp;supported_radio_tx_power[0] ... supported_radio_tx_power[N-1]<br>
}<br>
<br>
Length: >= 7 bytes
</td>
</tr>
<tr>
<td>Description</td>
<td>
Returns a byte array that encodes:<br>
<ul>
<li> 8-bit version of this spec, <code>0x00</code> in this instance.
<li> 8-bit value of the maximum number of slots this beacon is capable of broadcasting concurrently, inclusive of EID slots. This is the total number of advertisement frames the beacon is capable of broadcasting concurrently.
<li> 8-bit value of the maximum number of independent EID slots this beacon is capable of broadcasting. (Independent in this context means full ECDH key exchange to generate the EID with separate identity keys and clock values.)
<li>8-bit bit field encoding the following properties:
<ul>
<li><code>0x01: IS_VARIABLE_ADV_SUPPORTED</code>. Set if the beacon supports individual per-slot advertising intervals. Not set if the beacon supports only a global advertising interval.
<li><code>0x02: IS_VARIABLE_TX_POWER_SUPPORTED</code>. Set if the beacon supports individual per-slot Tx powers. Not set if the beacon supports only a single global Tx power.
</ul>
<li>16-bit bit field encoding the supported frame types:
<ul>
<li><code>0x0001</code>: UID
<li><code>0x0002</code>: URL
<li><code>0x0004</code>: TLM
<li><code>0x0008</code>: EID
<li><code>0x0010</code> through <code>0x8000</code> RFU and must be unset
</ul>
<li>A variable length array of the supported radio Tx power absolute (dBm) values. Note: This array should be ordered from the lowest power supported at index 0, to the highest power supported at index N.  - see "Radio Tx Power" Characteristic. This is to allow the power table to be easily searchable.
</ul>
</td>
</tr>
<tr>
<td>Lock Requirement</td>
<td>Readable only when unlocked. Never writable.</td>
</tr>
<tr>
<td>Return Codes</td>
<td>Write Not Permitted: for any attempt to write.
<br><br>
Read Not Permitted: for an attempt to read while the beacon is locked.</td>
</tr>
</table>

<table>
<tr><td><strong>Characteristic 2</strong></td><td/></tr>
<tr>
<td>Characteristic&nbsp;UUID</td>
<td>a3c8<strong>7502</strong>-8ed3-4bdf-8a39-a01bebede295</td>
</tr>
<tr>
<tr>
<td>Name</td>
<td>Active Slot</td>
</tr>
<tr>
<td>Properties</td>
<td>
uint8_t get_active_slot (read)<br>
uint8_t set_active_slot (write)
</td>
</tr>
<tr>
<td>Description</td>
<td>
Reads and writes the active slot number, 0-indexed. The default value is 0, and beacons must set this value when a new connection is made from a configuration client.
<br><br>
Subsequent writes to other characteristics will act on this slot number, configuring the Tx power, advertising interval, and data for it.
</td>
</tr>
<tr>
<td>Lock Requirement</td>
<td>Must be unlocked for both read and write.</td>
</tr>
<tr>
<td>Return Codes</td>
<td>Invalid Attribute Length: for an attempt to write illegal values. The beacon will list the total number of available slots in the max_supported_total_slots field in the Capabilities characteristic.
<br><br>
Read Not Permitted: for any attempt to read while the beacon is locked.<br>
Write Not Permitted: for any attempt to write while the beacon is locked.
</td>
</tr>
</table>

<table>
<tr><td><strong>Characteristic 3</strong></td><td/></tr>
<tr>
<td>Name</td>
<td>Advertising Interval</td>
</tr>
<tr>
<td>Characteristic&nbsp;UUID</td>
<td>a3c8<strong>7503</strong>-8ed3-4bdf-8a39-a01bebede295</td>
</tr>
<tr>
<td>Properties</td>
<td>
uint16_t get_adv_interval (read)  Note: big-endian<br>
uint16_t set_adv_interval (write)  Note: big-endian
</td>
</tr>
<tr>
<td>Description</td>
<td>
If <code>IS_VARIABLE_ADV_SUPPORTED</code> is set, this reads and writes the advertising interval (in milliseconds) for the active slot.
<br><br>
If <code>IS_VARIABLE_ADV_SUPPORTED</code> is not set, writing a value sets the advertising interval for all slots, regardless of the active slot, in a "last-one-wins" fashion. Reading returns the last value set.
<br><br>
If a value beyond the standard minimum or maximum is written, the beacon shall clamp the value written to the nearest legal value.
<br><br>
Additionally, if the user is placing unrealistic demands on the hardware, e.g. requesting for all slots to broadcast at the minimum interval, the beacon may clamp values to a higher minimum.
</td>
</tr>
<tr>
<td>Lock Requirement</td>
<td>Must be unlocked for both read and write.</td>
</tr>
<tr>
<td>Return Codes</td>
<td>
Read Not Permitted: for any attempt to read or write while the beacon is locked.<br>
Write Not Permitted: for any attempt to write while the beacon is locked.
</td>
</td>
</tr>
</table>

<table>
<tr><td><strong>Characteristic 4</strong></td><td/></tr>
<tr>
<td>Name</td>
<td>Radio Tx Power</td>
</tr>
<tr>
<td>Characteristic&nbsp;UUID</td>
<td>a3c8<strong>7504</strong>-8ed3-4bdf-8a39-a01bebede295</td>
</tr>
<tr>
<td>Properties</td>
<td>
int8_t get_radio_tx_power (read)<br>
int8_t set_radio_tx_power (write)
</td>
</tr>
<tr>
<td>Description</td>
<td>
If <code>IS_VARIABLE_TX_POWER_SUPPORTED</code> is set, this reads and writes the desired radio Tx power for the active slot.
<br><br>
If <code>IS_VARIABLE_TX_POWER_SUPPORTED</code> is not set, writing a value sets the desired global Tx power for all broadcasts, regardless of the active slot, in a "last-one-wins" fashion. Reading returns the last value set.
<br><br>
Note: if a power is selected that is not supported by the radio, the beacon should select the next highest power supported, or else the maximum power. As the <code>supported_tx_power</code> table is ordered from the lowest to highest power, an implementation can simply search it from low to high index, and find the first entry that is greater than or equal to the requested power, or if greater than all entries, chose the maximum power.
</td>
</tr>
<tr>
<td>Lock Requirement</td>
<td>Must be unlocked for both read and write.</td>
</tr>
<tr>
<td>Return Codes</td>
<td>
Read Not Permitted: for any attempt to read or write while the beacon is locked.<br>
Write Not Permitted: for any attempt to write while the beacon is locked.
</td>
</tr>
</table>

<table>
<tr><td><strong>Characteristic 5</strong></td><td/></tr>
<tr>
<td>Name</td>
<td>(Advanced) Advertised Tx Power</td>
</tr>
<tr>
<td>Characteristic&nbsp;UUID</td>
<td>a3c8<strong>7505</strong>-8ed3-4bdf-8a39-a01bebede295</td>
</tr>
<tr>
<td>Properties</td>
<td>
int8_t get_advertised_tx_power (read)<br>
int8_t set_advertised_tx_power (write)
</td>
</tr>
<tr>
<td>Description</td>
<td>
Writes a value that shall be used as the Tx power that is advertised in the Eddystone-UID, -URL, and -EID frame types. If not set, the Tx power advertised in those frames shall be the value set in the Radio Tx Power characteristic, and the read value shall reflect that.
<br><br>
This characteristic is available as an advanced deployer option. Beacon manufacturers will make every effort to ensure that the Radio Tx Power set is accurate and is the value received at 0m, but manufacturing changes and other unforeseeable events mean that some correction may be necessary. Implementors of this GATT service should make every effort to ensure that end-users cannot set this value without understanding the effect it has on the broadcast data.
<br><br>
Once this value has been set, the user is then responsible for maintaining the state of both the Radio Tx Power characteristic value and this one.
</td>
</tr>
<tr>
<td>Lock Requirement</td>
<td>Must be unlocked for both read and write.</td>
</tr>
<tr>
<td>Return Codes</td>
<td>
Read Not Permitted: for any attempt to read or write while the beacon is locked.<br>
Write Not Permitted: for any attempt to write while the beacon is locked.
</td>
</tr>
</table>

<table>
<tr><td><strong>Characteristic 6</strong></td><td/></tr>
<tr>
<td>Name</td>
<td>Lock State</td>
</tr>
<tr>
<td>Characteristic&nbsp;UUID</td>
<td>a3c8<strong>7506</strong>-8ed3-4bdf-8a39-a01bebede295</td>
</tr>
<tr>
<td>Properties</td>
<td>
uint8 lock_state (read)
<br><br>
byte_array (write) {<br>
&nbsp;&nbsp;lock_byte,<br>
&nbsp;&nbsp;encrypted_key[0] ... encrypted_key[15]<br>
}
</td>
</tr>
<tr>
<td>Description</td>
<td>
Reads a byte value that indicates the current lock state. Status values are:
<br><br>
<code>0x00: LOCKED</code><br>
<code>0x01: UNLOCKED</code><br>
<code>0x02: UNLOCKED AND AUTOMATIC RELOCK DISABLED</code>
<br><br>
Write 1 byte to lock or 17 bytes to transition to a new lock state:
<br><br>
</code>0x00: A single byte of 0x00 written to this characteristic will transition the interface to the LOCKED state without changing the current security key value.
<br><br>
0x00 + key[16]: A single byte of 0x00 followed by a 16 byte encrypted key value written to this characteristic will transition the interface to the LOCKED state and update the security key to the unencrypted value of key[16].
<br><br>
0x02: A single byte of 0x02 written to this characteristic will disable the automatic relocking capability of the interface
<br><br>
To prevent the new lock code from being broadcast in the clear, the client shall AES-128-ECB encrypt the new code with the existing lock code. The beacon shall perform the decryption with its existing lock code and set that value as the new code.
<br><br>
Length: 1 byte or 17 bytes
</td>
</tr>
<tr>
<td>Lock Requirement</td>
<td>Readable in locked or unlocked state.<br>
Writeable only in unlocked state.</td>
</tr>
<tr>
<td>Return Codes</td>
<td>Write Not Permitted: on a write attempt to unlock with the incorrect key</td>
</tr>
</table>

<table>
<tr><td><strong>Characteristic 7</strong></td><td/></tr>
<tr>
<td>Name</td>
<td>Unlock</td>
</tr>
<tr>
<td>Characteristic&nbsp;UUID</td>
<td>a3c8<strong>7507</strong>-8ed3-4bdf-8a39-a01bebede295</td>
</tr>
<tr>
<td>Properties</td>
<td>
byte_array (read) {<br>
&nbsp;&nbsp;challenge[0] ... challenge[15]<br>
}<br>
<br>
byte_array (write) {<br>
&nbsp;&nbsp;unlock_token[0] ... unlock_token[15]<br>
}
<br><br>
Length: 16 bytes
</td>
</tr>
<tr>
<td>Description</td>
<td>
Writes to a beacon and sets it to the unlock state, protected from configuration changes or reading sensitive information.
<br><br>
Read: returns a 128-bit challenge token. This token is for one-time use and cannot be replayed.
<br><br>
Write: accepts a 128-bit encrypted value that verifies the client knows the beacon's lock code.
<br><br>
Note: this is a byte array of 16 bytes in a natural ordering. When reading from a big-endian buffer, the 0 byte comes first and the 15 byte comes last.
<br><br>
To securely unlock the beacon, the host must write a one-time use unlock_token into the characteristic.
<br><br>
To create the unlock_token, it first reads the randomly generated 16-byte challenge and generates it using AES-128-ECB.encrypt (key=beacon_lock_code[16], text=challenge[16]).
<br><br>
This unlock_token is then written to the beacon in this characteristic. (Note: as a result the secret is never sent in the clear). The beacon then repeats this process internally using the challenge and the beacon_lock_code, performing the AES-128 ECB encrypt function.
<br><br>
If the result of this calculation matches the unlock_token written to the characteristic, the beacon is unlocked.
<br><br>
Sets the lock state to <code>0x01</code> on success.
<br><br>
Note: All additional 128-bit values, such as the key and unlock_token, will need to have the same big-endian representation. Care needs to be taken with a beacon developer's kit AES library to ensure it is also using big-endian data, or if little-endian, the order of bytes in both params and results will need to be reversed.
</td>
</tr>
<tr>
<td>Lock Requirement</td>
<td>Readable only in locked state.<br>
Writeable only in locked state.</td>
</tr>
<tr>
<td>Return Codes</td>
<td>
Read Not Permitted: for any attempt to read while the beacon is unlocked.<br>
Write Not Permitted: for any attempt to write while the beacon is unlocked.
</td>
</tr>
</table>

<table>
<tr><td><strong>Characteristic 8</strong></td><td/></tr>
<tr>
<td>Name</td>
<td>Public ECDH Key</td>
</tr>
<tr>
<td>Characteristic&nbsp;UUID</td>
<td>a3c8<strong>7508</strong>-8ed3-4bdf-8a39-a01bebede295</td>
</tr>
<tr>
<td>Properties</td>
<td>
byte_array (read) {<br>
&nbsp;&nbsp;key[0] ... key[31]<br>
}
<br><br>
Length: 32 bytes
</td>
</tr>
<tr>
<td>Description</td>
<td>
Reads the beacon's 256-bit public ECDH key (little-endian).
</td>
</tr>
<tr>
<td>Lock Requirement</td>
<td>Readable only in unlocked state. Never writable.</td>
</tr>
<tr>
<td>Return Codes</td>
<td>Write Not Permitted: on any write attempt.
<br><br>
Read Not Permitted: for any attempt to read while the beacon is locked.</td>
</tr>
</table>

<table>
<tr><td><strong>Characteristic 9</strong></td><td/></tr>
<tr>
<td>Name</td>
<td>EID Identity Key</td>
</tr>
<tr>
<td>Characteristic&nbsp;UUID</td>
<td>a3c8<strong>7509</strong>-8ed3-4bdf-8a39-a01bebede295</td>
</tr>
<tr>
<td>Properties</td>
<td>
byte array (read) {<br>
&nbsp;&nbsp;encrypted_ik[0] ... encrypted_ik[15]<br>
}
<br><br>
Length: 16 bytes
</td>
</tr>
<tr>
<td>Description</td>
<td>
Reads the identity key (little-endian) for the active slot. If the slot isn't configured as EID, returns an error.
<br><br>
To prevent this data being broadcast in the clear, it shall be AES-128 encrypted with the lock code for the beacon.
</td>
</tr>
<tr>
<td>Lock Requirement</td>
<td>Must be unlocked for read. Never writable.</td>
</tr>
<tr>
<td>Return Codes</td>
<td>Write Not Permitted: on any write attempt.
<br><br>
Read Not Permitted: for any attempt to read while the beacon is locked.</td>
</tr>
</table>

<table>
<tr><td><strong>Characteristic 10</strong></td><td/></tr>
<tr>
<td>Name</td>
<td>ADV Slot Data</td>
</tr>
<tr>
<td>Characteristic&nbsp;UUID</td>
<td>a3c8<strong>750a</strong>-8ed3-4bdf-8a39-a01bebede295</td>
</tr>
<tr>
<td>Properties</td>
<td>
byte array (read/write) {<br>
&nbsp;&nbsp;frame_type,<br>
&nbsp;&nbsp;data[0] ... data[N]<br>
}
<br><br>
Write length: 17 bytes (UID), 19 bytes (URL), 1 byte (TLM), 34 or 18 bytes (EID)
</td>
</tr>
<tr>
<td>Description</td>
<td>
Reads the data set in the active slot, and sets the data/parameters to be broadcast. The interpretation of the read and written data is characterised by the frame_type byte, which maps to the Eddystone frame type bytes of <code>0x00</code> (UID), <code>0x10</code> (URL), <code>0x20</code> (TLM), <code>0x30</code> (EID).
<br><br>
Note that the format of read and written data differ slightly, as detailed below. The read data is just the broadcast data for that slot, and may include the Tx power; the written data is just the frame type and any ID-related information, and doesn't include the Tx power since that is controlled by characteristics 4 (Radio Tx Power) and 5 (Advertised Tx Power).
<br><br>
Note: All arrays of bytes used in these parameters are in big-endian order.
<br><br>
Reading:<br>
In the case of a UID, URL or TLM frame, the length and data are those of the broadcast data, including any adjustments to the advertised Tx power applied via the Tx power calibration table.
<br><br>
If the slot is configured to advertise EID frames, the length is 14:
1 byte frame type, 1 byte exponent, 4-byte clock value, 8-byte EID. These are the parameters required for registration, along with the beacon's public key, which is exposed through a separate characteristic, and the resolver's public key, which the provisioning/registering beacon knows.
<br><br>
If the slot is currently not broadcasting, reading the slot data shall return either an empty array or a single byte of <code>0x00</code>.
<br><br>
Writing:<br>
In the case of UID and URL frames, the data to be broadcast is supplied in the array after the frame type. UID is 16 bytes of namespace + instance, URL is up to 18 bytes.
<br><br>
In the case of a TLM frame, the data is just the frame type byte, <code>0x20</code>. If another slot on the beacon has been configured as an EID frame type, the beacon shall broadcast the ETLM variety of telemetry. Otherwise, the plain TLM frame shall be broadcast. If the beacon is currently broadcasting a plain TLM frame and an EID frame is configured, the beacon shall switch to broadcasting the ETLM variety. If the beacon is configured to broadcast multiple EID frames, then the beacon should cycle through the set identity keys and use them in turn to broadcast an equal number of ETLM frames.
<br><br>
In the case of an EID frame, the length is either 34 or 18. If 34, it's the frame type, the 32-byte service's public ECDH key (little-endian) and the exponent byte. This is the prefered method of provisioning an EID beacon. If 18, it's the frame type, the result of encrypting the 16-byte identity key (little-endian) with the beacon's lock code (big-endian), and the exponent. This is less secure and any provisioner who implements this should make it clear to the user.
<br><br>
Writing an empty array, or a single <code>0x00</code> byte clears the slot and stops Tx. If configured as an EID beacon this will also destroy the peripheral's state for this frame data.
</td>
</tr>
<tr>
<td>Lock Requirement</td>
<td>Must be unlocked for both read and write.</td>
</tr>
<tr>
<td>Return Codes</td>
<td>
Read Not Permitted: for any attempt to read while the beacon is locked.<br>
Write Not Permitted: for any attempt to write while the beacon is locked.
<br><br>
Invalid Attribute Length: on any attempt to write with an illegal number of bytes.</td>
</tr>
</table>

<table>
<tr><td><strong>Characteristic 11</strong></td><td/></tr>
<tr>
<td>Name</td>
<td>(Advanced) Factory reset</td>
</tr>
<tr>
<td>Characteristic&nbsp;UUID</td>
<td>a3c8<strong>750b</strong>-8ed3-4bdf-8a39-a01bebede295</td>
</tr>
<tr>
<td>Properties</td>
<td>
uint8 reset_boolean (write)
</td>
</tr>
<tr>
<td>Description</td>
<td>
If a value of <code>0x0B</code> is written, the beacon shall reset to its factory state. This does not include the unlock key, which shall remain set to its current value. All user-configured slot data (including EID identity keys) shall be destroyed and the beacon shall return to the factory state where it was broadcasting either a URL or a UID frame.
<br><br>
Any other value shall be ignored.
<br><br>
In addition, any write shall be ignored if the lock state is not <code>0x01</code>. The beacon must have been purposefully unlocked by the current client before a factory reset can be performed.
</td>
</tr>
<tr>
<td>Lock Requirement</td>
<td>Must be unlocked for write. Lock state must be 0x01.</td>
</tr>
<tr>
<td>Return Codes</td>
<td>Write Not Permitted: for any attempt to write while the beacon is locked, or to write while the beacon's lock state is not 0x01.</td>
</tr>
</table>

<table>
<tr><td><strong>Characteristic 12</strong></td><td/></tr>
<tr>
<td>Name</td>
<td>(Advanced) Remain Connectable</td>
</tr>
<tr>
<td>Characteristic&nbsp;UUID</td>
<td>a3c8<strong>750c</strong>-8ed3-4bdf-8a39-a01bebede295</td>
</tr>
<tr>
<td>Properties</td>
<td>
uint8 is_non_connectable_supported (read)<br>
uint8 remain_connectable_boolean (write)
</td>
</tr>
<tr>
<td>Description</td>
<td>
On read, returning a non-zero value indicates that the beacon is capable of becoming non-connectable and that writing a zero value will transition the beacon to a non-connectable state after the client disconnects. Returning a zero value indicates that the beacon is limited to running in an always connectable state.
<br><br>
If any non-zero value is written, the beacon shall remain in its connectable state until any other value is written. For beacons that are not capable of becoming non-connectable, this is a NOP and ignored.
<br><br>
If a zero value is written, the beacon shall resume its normal operation of becoming non-connectable after the central disconnects. Typically this function is used by an application designed to automatically validate BLE advertisement updates achieved by reading or writing characteristics in this service. If the beacon is not capable of becoming non-connectable, this is a NOP. Clients should inspect the capabilities first by reading from this characteristic.
</td>
</tr>
<tr>
<td>Lock Requirement</td>
<td>Must be unlocked for write.</td>
</tr>
<tr>
<td>Return Codes</td>
<td>Write Not Permitted: for any attempt to write while the beacon is locked.</td>
</tr>
</table>

## GATT Error Codes

<table>
<tr><td>Code</td><td>Description</td><td>Notes</td></tr>
<tr>
<td><code>0x02</code></td>
<td>Read Not Permitted</td>
<td>Returned when accessing an authenticated characteristic while the beacon is locked</td>
</tr>
<tr>
<td><code>0x03</code></td>
<td>Write Not Permitted</td>
<td>Returned for read only characteristics</td>
</tr>
<tr>
<td><code>0x0d</code></td>
<td>Invalid Attribute Length</td>
<td>Returned when a parameter is out of range, or the number of bytes representing a parameter is different from expected.</td>
</tr>
</table>
