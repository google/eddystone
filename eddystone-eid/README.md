# Eddystone-EID

The Eddystone-EID frame broadcasts an encrypted ephemeral identifier that changes periodically at a rate determined during the initial registration with a web service. The broadcast ephemeral ID can be resolved remotely by the service with which it was registered, but to other observers appears to be changing randomly. This frame type is intended for use in security and privacy-enhanced devices.

This document describes the frame specification. A separate document specifies the [cryptographic computation of the EID value](eid-computation.md) and the protocol for registering the beacon with a trusted resolver.

## Frame Specification

The EID frame is encoded in the advertisement as a Service Data block associated with the Eddystone service UUID. The layout is:

Byte offset | Field | Description
------------|-------|------------
0 | Frame Type | Value = `0x30`
1 | Ranging Data | Calibrated Tx power at 0 m
2 | EID[0] | 8-byte Ephemeral Identifier
3 | EID[1] |
4 | EID[2] |
5 | EID[3] |
6 | EID[4] |
7 | EID[5] |
8 | EID[6] |
9 | EID[7] |

## Field Notes

- The length of this frame is fixed and shall be truncated after the EID bytes.

## Implementation Guidelines

- Since the purpose of this frame type is to afford better privacy, we do not recommend interleaving advertisement of EID frames with any other frame type that is unique to the same device.
- The beacon should rotate its BD_ADDR in addition to the EID value to prevent tracking beyond the rotation period.
