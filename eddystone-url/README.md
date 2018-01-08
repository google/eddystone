# Eddystone-URL

The Eddystone-URL frame broadcasts a URL using a compressed encoding format in order to fit more within the limited advertisement packet.

Once decoded, the URL can be used by any client with access to the internet.  For example, if an Eddystone-URL beacon were to broadcast the URL `https://goo.gl/Aq18zF`, then any client that received this packet could choose to [visit that url](https://goo.gl/Aq18zF).

The Eddystone-URL frame forms the backbone of the [Physical Web](http://physical-web.org), an effort to enable frictionless discovery of web content relating to one’s surroundings. Eddystone-URL incorporates all of the learnings from the [UriBeacon](http://uribeacon.org) format from which it evolved.

## Frame Specification

Byte offset | Field | Description
------------|-------|------------
0 | Frame Type | Value = `0x10`
1 | TX Power | Calibrated Tx power at 0 m
2 | URL Scheme | Encoded Scheme Prefix
3+ | Encoded URL | Length 1-17

### Tx Power Level

Tx power is the received power at 0 meters, in dBm, and the value ranges from -100 dBm to +20 dBm to a resolution of 1 dBm.

Note to developers: the best way to determine the precise value to put into this field is to measure the actual output of your beacon from 1 meter away and then add 41dBm to that. 41dBm is the signal loss that occurs over 1 meter.

The value is a signed 8 bit integer as specified by
[TX Power Level Bluetooth Characteristic](https://developer.bluetooth.org/gatt/characteristics/Pages/CharacteristicViewer.aspx?u=org.bluetooth.characteristic.tx_power_level.xml).

#### Examples

* The value 0x12 is interpreted as +18dBm
* The value 0xEE is interpreted as -18dBm

### URL Scheme Prefix

The URL Scheme Prefix byte defines the identifier scheme, an optional prefix and how the remainder of the URL is encoded.

|Decimal  | Hex        | Expansion
|:------- | :--------- | :--------
|0        | 0x00       | `http://www.`
|1        | 0x01       | `https://www.`
|2        | 0x02       | `http://`
|3        | 0x03       | `https://`

### Eddystone-URL HTTP URL encoding

The HTTP URL scheme is defined by RFC 1738, for example
`https://goo.gl/S6zT6P`, and is used to designate Internet resources
accessible using HTTP (HyperText Transfer Protocol).

The encoding consists of a sequence of characters. Character codes
excluded from the URL encoding are used as text expansion codes. When
a user agent receives the Eddystone-URL the byte codes in the URL
identifier are replaced by the expansion text according to the table
below.

|Decimal  | Hex        | Expansion
|:------- | :--------- | :--------
|0        | 0x00       | .com/
|1        | 0x01       | .org/
|2        | 0x02       | .edu/
|3        | 0x03       | .net/
|4        | 0x04       | .info/
|5        | 0x05       | .biz/
|6        | 0x06       | .gov/
|7        | 0x07       | .com
|8        | 0x08       | .org
|9        | 0x09       | .edu
|10       | 0x0a       | .net
|11       | 0x0b       | .info
|12       | 0x0c       | .biz
|13       | 0x0d       | .gov
|14..32   | 0x0e..0x20 | Reserved for Future Use
|127..255 | 0x7F..0xFF | Reserved for Future Use

Note: 
* URLs are written only with the graphic printable characters of the US-ASCII coded character set. The octets **00-20** and **7F-FF** hexadecimal are not used. See “Excluded US-ASCII Characters” in RFC 2936.
* Range **07-13** define the same top level domains as **00-06** without a /.

## See Also

* **[Configuration Service Specification](docs/config-service-spec.md)** — An optional GATT service for configuring the Eddystone-URL output of your beacon
