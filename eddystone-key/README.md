# Eddystone-KEY

The Eddystone-KEY frame broadcasts a security key which can be used in two-factor authentication systems.

When a beacon is placed in a way that it cannot be easily moved or tampered with (e.g. buried in concrete floor of an office), using this beacon in two-factor authentication system can guarantee that a user is physically present at a certain place at a certain time.

## Frame Specification

The KEY frame is encoded in the advertisement as a Service Data block associated with the Eddystone service UUID. The layout is:

Byte offset | Field | Description
------------|-------|------------
0 | Frame Type | Value = `0x11`
1 | ALGOTYPE | Algorithm type
2 | KEYID[0] | Key identification
3 | KEYID[1]
4 | KEY[0] | Key data
5 | KEY[1]
6 | KEY[2]
7 | KEY[3]
8 | KEY[4]
9 | KEY[5]
10 | KEY[6]
11 | KEY[7]
12 | KEY[8]
13 | KEY[9]
14 | KEY[10]
15 | KEY[11]
16 | KEY[12]
17 | KEY[13]
18 | KEY[14]
19 | KEY[15]

All multi-byte values are big-endian.

### Algorithm type

Algorithm type defines what algorithm was used to generate the key broadcasted in the frame.

Decimal | Hex | Algorithm type | Key data encoding
--------|-----|----------------|------------------
`0`   | `0x00` | HOTP ([RFC 4226](https://tools.ietf.org/html/rfc4226)) | ASCII string padded with NULL
`1`   | `0x01` | TOTP ([RFC 6238](https://tools.ietf.org/html/rfc6238)) | ASCII string padded with NULL
`255` | `0xFF` | custom scheme | application specific binary data

### Key identification

Key identification is an application specific field to distinguish between different keys (or their parts).
Use `00 00` when the field is not needed in your application.

Possible use cases:

* Beacon might want to transmit more keys. The field is interpreted as the key index.
* Beacon might want to transmit longer key than 16 bytes. The field is interpreted as the part index.

### Key data

Application specific data to be interpreted as a key. This might be the whole key or its part.
Encoding depends on a specific application, but it should respect the encoding in the algorithm type table.

## Interleaving with other frames

Similarly to Telemetry [TLM](../eddystone-tlm) frames the KEY frames should be interleaved with an identifying frame type (e.g. Eddystone-UID or Eddystone-URL).

In addition, when TLM frames are broadcasted by the beacon, the `ADV_CNT` and/or `SEC_CNT` values must be used during the computation of counter dependent (e.g. HOTP) or time dependent (e.g. TOTP) keys.
