# Encrypted TLM Frame Specification

If an Eddystone compatible beacon has been configured as Eddystone-EID, then when asked to broadcast its telemetry it will use this encrypted frame type to avoid transmitting information that may make the device uniquely identifiable.

As with the plain TLM frame, the data is encoded in the advertisement as a Service Data block associated with the Eddystone service UUID. The frame type remains `0x20` and the version is `0x01`. The layout is:

Byte offset | Field | Description
------------|-------|------------
0 | Frame Type | Value = `0x20`
1 | Version | TLM version, value = `0x01`
2 | ETLM[0] | 12 bytes of Encrypted TLM data
3 | ETLM[1] |
4 | ETLM[2] |
5 | ETLM[3] |
6 | ETLM[4] |
7 | ETLM[5] |
8 | ETLM[6] |
9 | ETLM[7] |
10 | ETLM[8] |
11 | ETLM[9] |
12 | ETLM[10] |
13 | ETLM[11] |
14 | SALT[0] | 16-bit Salt
15 | SALT[1] |
16 | MIC[0] | 16 bit Message Integrity Check
17 | MIC[1] |

All multi-byte values are big-endian.

## Computing the encrypted TLM data

Since telemetry fields change on their own schedule, the encryption scheme needs to introduce additional variability when encrypting them, so that the opacity of the eTLM broadcast is independent of the variability of the underlying data. The ETLM generation process includes a time element for this purpose.

1. Generate a 48-bit nonce, consisting of a concatenation of:
  * The common time base (same used for EID broadcasts, truncated by the same factor as in the EID generation protocol).
  * 16 random bits as a salt
1. Encrypt the state signal using AES-EAX, using the nonce computed in the first step and the Identity Key from the EID configuration process. This produces:
  * Telemetry ciphertext of the same length as the cleartext input
  * A 16-bit Message Integrity Check tag
1. Transmit a concatenation of:
  * The encrypted telemetry
  * The 16 bit random salt
  * The 16-bit integrity check tag

## Decrypting the encrypted TLM data

Just as plain TLM depends on being paired with a UID to be useful, ETLM depends on being paired with an EID broadcast. To report the telemetry, the client pairs ETLM and EID broadcasts received from the same device address near the same time.

Once the ETLM is paired with a resolved EID broadcast:

1. Retrieve the Identity Key and clock offset for that beacon
1. Perform AES-EAX decryption operation using:
  * The truncated time
  * The "salt" from the broadcast, and
  * The Identity Key from storage
1. Check the AES-EAX output against the integrity check tag in the ETLM broadcast.
1. Store/use the decrypted telemetry data output from the AES operation.
