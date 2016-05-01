# Eddystone-TLM

Eddystone beacons may transmit data about their own operation to clients. This data is called _telemetry_ and is useful for monitoring the health and operation of a fleet of beacons. Since the Eddystone-TLM frame does not contain a beacon ID, it must be paired with an identifying frame which provides the ID, either of type Eddystone-UID or Eddystone-URL. See Interleaving Telemetry for details.

TLM frames may be broadcast either in the clear, like UID and URL frames, or, when a beacon has been configured as Eddystone-EID, encrypted with the identity key set during the EID configuration. When broadcast in the clear there is no message integrity validation and you should design your applications to be tolerant of the open nature of such a broadcast.

* [Unencrypted TLM specification](tlm-plain.md)
* [Encrypted TLM specification](tlm-encrypted.md)
