# Unencrypted TLM Frame Specification

The TLM frame is encoded in the advertisement as a Service Data block associated with the Eddystone service UUID. The layout is:

Byte offset | Field | Description
------------|-------|------------
0 | Frame Type | Value = `0x20`
1 | Version | TLM version, value = `0x00`
2 | VBATT[0] | Battery voltage, 1 mV/bit
3 | VBATT[1] |
4 | TEMP[0] | Beacon temperature
5 | TEMP[1] |
6 | ADV_CNT[0] | Advertising PDU count
7 | ADV_CNT[1] |
8 | ADV_CNT[2] |
9 | ADV_CNT[3] |
10 | SEC_CNT[0] | Time since power-on or reboot
11 | SEC_CNT[1] |
12 | SEC_CNT[2] |
13 | SEC_CNT[3] |

All multi-byte values are big-endian.

## Field Notes

- The length of this frame is fixed and must be truncated after the "time since power-on" bytes. The value of the Service Data Length byte must be `0x11`.
- TLM version allows for future development of this frame type. At present the value must be `0x00`.
- Battery voltage is the current battery charge in millivolts, expressed as 1 mV per bit. If not supported (for example in a USB-powered beacon) the value should be zeroed.
- Beacon temperature is the temperature in degrees Celsius sensed by the beacon and expressed in a signed [8.8 fixed-point notation](https://www.google.com/url?q=https%3A%2F%2Fcourses.cit.cornell.edu%2Fee476%2FMath%2F&sa=D&sntz=1&usg=AFQjCNG3AHS46J3FlyEoV5NY4lTYoSVOCA). If not supported the value should be set to 0x8000, -128 °C.
- ADV_CNT is the running count of advertisement frames of all types emitted by the beacon since power-up or reboot, useful for monitoring performance metrics that scale per broadcast frame. If this value is reset (e.g. on reboot), the current time field must also be reset.
- SEC_CNT is a 0.1 second resolution counter that represents time since beacon power-up or reboot. If this value is reset (e.g. on a reboot), the ADV count field must also be reset.

## Interleaving Telemetry

Telemetry frames should be interleaved with an identifying frame type (e.g. Eddystone-UID or Eddystone-URL). The TLM frame depends on the client to associate it with the identifying frames via the device address (since no beacon ID exists in the TLM frame). Thus it is important that a beacon that rotates or randomizes its device address uses the same device address for some period that allows for multiple ID frames and multiple TLM frames to be broadcast from the same device address.

You may decide to interleave one TLM frame for each ID frame, or use some lower ratio depending on how critical the TLM data is for your application. For example, if your application is using the TLM for fleet management, it may be satisfied receiving one TLM frame per day, so your beacons deployed in a high traffic area may broadcast TLM only once per minute or longer, giving a high probability that a client receives the TLM frame and transmits it to your service once per day. In a low-traffic environment such an application may require a higher TLM rate.

Finally, if your application uses TLM to (e.g.) supply up-to-the-minute runtime status information to an end user, you may wish to interleave TLM with ID frames 1 to 1.
