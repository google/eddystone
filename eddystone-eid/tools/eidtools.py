#!/usr/bin/python

"""A few utilities for working with the cryptography around ephemeral ids.

In particular, it can:
1. compute ephemeral ids given an Identity Key, a scaler and a timestamp;
2. compute the identity key from the Curve25519 shared secret and the public
   keys;
3. compute all the steps needed to register a beacon starting from the service
   public key;
4. compute the Curve25519 key pair starting from a source binary string;
5. compute the shared secret between two Curve25519 key pairs.

It requires pycrypto and hkdf. The easiest way of using this is by doing:
(sudo) pip install pycrypto hkdf

"""

import base64
import hashlib
import random
import sys
import time

from Crypto.Cipher import AES
from Crypto.Util import number
import hkdf


def Usage():
  """Print the usage and exits."""
  print """\
Usage:
    python eidtools.py <command> <arguments>

Usage for command "registration":
    python eidtools.py registration <public_key> <scaler> <beacon_time_seconds>

    Given the 32-bytes service public key, and a set of beacon parameters,
    generates a random key pair for the beacon, computes the shared secret, the
    identity key, and the first EID.

Usage for command "beacon":
    python eidtools.py beacon <ik> <scaler> <beacon_initial_time_seconds> \
        <service_initial_time_seconds>

    Given the identity key, the beacon rotation exponent, and the times of
    registration for the beacon and the service, this computes the eid that the
    beacon is currently broadcasting.

Usage for command "keygen":
    python eidtools.py keygen <source>

    Generates curve25519 keypair from <source>. It must have 32-bytes.

Usage for command "shared":
    python eidtools.py shared <private> <public>

    Generates the shared secret between the owner of <private> and the
    owner of the private key associate to <public>.

Usage for command "ik":
    python eidtools.py ik <shared> <service_public> <beacon_public>

    Generates an identity key from a shared 32-bytes curve25519 secret, and the
    public keys of the service and the beacon.

Usage for command "eid":
    python eidtools.py eid <ik> <scaler> <beacon_time_seconds>

    Generates an EID from the given 16-bytes identity key, the scaler and the
    beacon time in seconds.

Binary arguments are interpreted depending on their first byte:
    a: the rest of the string is interpreted as ASCII;
    h: the rest of the string is interpreted as bytes in hex;
    b: the rest of the string is interpreted as base64.
    d: the rest of the string is interpreted as decimal (in little endian).
Integer inputs can be written as decimal, or as hex as in 0x1234.

"""
  sys.exit(1)


def ToHex(data):
  """Return a string representing data in hexadecimal format."""
  s = ""
  for c in data:
    s += ("%02x" % ord(c))
  return s


def PrintSeconds(seconds):
  """Return a string representing the given time in seconds."""
  orig = seconds
  minutes = seconds // 60
  seconds %= 60
  hours = minutes // 60
  minutes %= 60
  days = hours // 24
  hours %= 24

  s = ""
  if days > 0:
    s += "%dd " % days
  if hours > 0:
    s += "%02dh " % hours
  if minutes > 0:
    s += "%02dm " % minutes
  s += "%02ds" % seconds
  s += " (%ds)" % orig
  return s


def PrintBinary(desc, data):
  """Print the binary data in data in several formats."""
  s = desc + ":\n"

  s += "  ASCII:  "
  if all("\x20" <= c < "\x80" for c in data):
    s += data + ""
  else:
    s += "[unprintable]"
  s += "\n"

  s += "  Hex:    " + ToHex(data) + "\n"
  s += "  Base64: " + base64.b64encode(data) + "\n"
  s += "  Int:    " + str(FromBinary(data)) + "\n"

  for i, c in enumerate(data):
    if i % 8 == 0:
      if i != 0:
        s += "\n"
      s += "  "
    s += ("(byte) 0x%02x, " % ord(c))
  s += "\n"

  print s


def GetBinary(data, required_length=None):
  """Read binary data from a string using the format explained in the usage."""
  ret = ""
  if data[0] == "a":
    ret = data[1:]
  elif data[0] == "h":
    ret = data[1:].decode("hex")
  elif data[0] == "b":
    ret = base64.b64decode(data[1:])
  elif data[0] == "d" and required_length is not None:
    ret = ToBinary(long(data[1:]), required_length)
  else:
    print "Invalid format for binary data."
    Usage()
  if required_length is not None and len(ret) != required_length:
    print "Binary data must have %d bytes." % required_length
    sys.exit(1)
  return ret


def ToBinary(x, length):
  """Change x to a little-endian binary string of given length."""
  # We assume that long_to_bytes() convert to a big-endian string; since we need
  # a little-endian one, we reverse it.
  ret = number.long_to_bytes(x)[::-1]
  if len(ret) < length:
    ret += "\x00" * (length - len(ret))
  return ret


def FromBinary(x):
  """Return an integer corresponding to the little-endian binary string x."""
  # We assume that bytes_to_long() convert from a big-endian string; since we
  # have a little-endian one, we reverse it.
  return number.bytes_to_long(x[::-1])


class Curve25519(object):
  """Implementation of Curve25519 using big integers."""

  P = 2 ** 255 - 19
  BASE_POINT = "\x09" + ("\x00" * 31)

  @staticmethod
  def ToPrivate(source):
    """Translate a generic 32-bytes string into a private key."""
    return (
        chr(ord(source[0]) & 248) +
        source[1:31] +
        chr((ord(source[31]) & 127) | 64)
    )

  @staticmethod
  def ScalarMult(n, q=BASE_POINT):
    """Return the point nq."""
    n = FromBinary(Curve25519.ToPrivate(n))
    # The reference implementation ignores the most significative bit.
    q = FromBinary(q) % (2**255)
    nq = Curve25519._Multiple(n, q)
    nq = (nq.x * number.inverse(nq.z, Curve25519.P)) % Curve25519.P
    ret = ToBinary(nq, 32)
    return ret

  @staticmethod
  def _Multiple(n, q):
    """Inner version of scalarMult, accepts big integers instead of strings."""
    nq = Curve25519.Montgomery(1, 0)
    nqpq = Curve25519.Montgomery(q, 1)
    for i in range(255, -1, -1):
      if (n >> i) & 1:
        nqpq, nq = Curve25519._Montgomery(nqpq, nq, q)
      else:
        nq, nqpq = Curve25519._Montgomery(nq, nqpq, q)
    return nq

  @staticmethod
  def _Montgomery(q, p, qmp):
    """Given two points q and p, and q-p, return 2q and q+p."""
    qprime = q.ToSumDiffComponents()
    pprime = p.ToSumDiffComponents()
    pprime = pprime.CrossMul(qprime)
    qprime = qprime.ToSquareComponents()
    qpp = pprime.ToSumDiffComponents().ToSquareComponents()
    qpp.z *= qmp
    qpq = qprime.ToMulDiffComponents()
    t = qpq.z
    qpq.z = (((qpq.z * 121665) + qprime.x) * t) % Curve25519.P
    return qpq, qpp

  class Montgomery(object):
    """A number represented as x/z."""

    def __init__(self, x, z):
      self.x = x % Curve25519.P
      self.z = z % Curve25519.P

    def CrossMul(self, other):
      return Curve25519.Montgomery(self.x * other.z, self.z * other.x)

    def ToSquareComponents(self):
      return Curve25519.Montgomery(self.x ** 2, self.z ** 2)

    def ToSumDiffComponents(self):
      return Curve25519.Montgomery(self.x + self.z, self.z - self.x)

    def ToMulDiffComponents(self):
      return Curve25519.Montgomery(self.x * self.z, self.x - self.z)

    def ToNumber(self):
      return (self.x * number.inverse(self.z, Curve25519.P)) % Curve25519.P


def GetAndPrintIdentityKey(shared_secret,
                           service_public_key, beacon_public_key):
  """Compute the identity key from a Curve25519 shared secret."""
  salt = service_public_key + beacon_public_key
  PrintBinary("Salt", salt)
  prk = hkdf.hkdf_extract(salt, shared_secret, hash=hashlib.sha256)
  PrintBinary("Prk (extracted bytes)", prk)
  ik = hkdf.hkdf_expand(prk, "", 32, hash=hashlib.sha256)[:16]
  PrintBinary("Identity key", ik)
  return ik


def GetAndPrintEid(ik, scaler, beacon_time_seconds):
  """Return the EID generated by the given parameters."""
  tkdata = (
      "\x00" * 11 +
      "\xFF" +
      "\x00" * 2 +
      chr((beacon_time_seconds / (2 ** 24)) % 256) +
      chr((beacon_time_seconds / (2 ** 16)) % 256))
  PrintBinary("Temporary Key data", tkdata)
  tk = AES.new(ik, AES.MODE_ECB).encrypt(tkdata)
  PrintBinary("Temporary Key", tk)
  beacon_time_seconds = (beacon_time_seconds // 2 ** scaler) * (2 ** scaler)
  eiddata = (
      "\x00" * 11 +
      chr(scaler) +
      chr((beacon_time_seconds / (2 ** 24)) % 256) +
      chr((beacon_time_seconds / (2 ** 16)) % 256) +
      chr((beacon_time_seconds / (2 ** 8)) % 256) +
      chr((beacon_time_seconds / (2 ** 0)) % 256))
  PrintBinary("Ephemeral Id data", eiddata)
  eid = AES.new(tk, AES.MODE_ECB).encrypt(eiddata)[:8]
  PrintBinary("Ephemeral Id", eid)
  return eid


def main(command, args):
  if command == "registration":
    if len(args) != 3:
      Usage()
    service_public_key = GetBinary(args[0], 32)
    PrintBinary("Service public key", service_public_key)
    beacon_source = "".join(chr(random.randint(0, 255)) for _ in range(32))
    beacon_private_key = Curve25519.ToPrivate(beacon_source)
    PrintBinary("Beacon private key (random)", beacon_private_key)
    beacon_public_key = Curve25519.ScalarMult(beacon_private_key)
    PrintBinary("Beacon public key", beacon_public_key)
    shared = Curve25519.ScalarMult(beacon_private_key, service_public_key)
    PrintBinary("Shared secret", shared)
    if shared == "\x00" * 32:
      print "NOTE: shared key is invalid, due to an invalid service public key."
    ik = GetAndPrintIdentityKey(shared, service_public_key, beacon_public_key)
    scaler = int(args[1], 0)
    beacon_time_seconds = int(args[2], 0)
    eid = GetAndPrintEid(ik, scaler, beacon_time_seconds)
    # Print the request to register this beacon.
    print
    print "Registration line:"
    print ("  hpost /v1beta1/beacons:register "
           """{"advertisedId": {type:"EDDYSTONE", "id":"<beacon_id>"}, """
           """status:"ACTIVE", ephemeral_id_registration: """
           """{beacon_ecdh_public_key: "%s", """
           """service_ecdh_public_key: "%s", """
           """initial_clock_value: %s, """
           """rotation_period_exponent: %s, """
           """initial_eidr: "%s" }}""" % (
               base64.b64encode(beacon_public_key),
               base64.b64encode(service_public_key),
               beacon_time_seconds,
               scaler,
               base64.b64encode(eid))
          )
    # Print the request to register this beacon.
    print
    print "Broadcast line:"
    print ("  python %s beacon b%s %s %s %s" % (
        sys.argv[0],
        base64.b64encode(ik),
        scaler,
        beacon_time_seconds,
        int(time.time())))

  elif command == "beacon":
    if len(args) != 4:
      Usage()
    ik = GetBinary(args[0], 16)
    PrintBinary("Identity key", ik)
    scaler = int(args[1], 0)
    beacon_initial_time_seconds = int(args[2], 0)
    service_initial_time_seconds = int(args[3], 0)
    beacon_time_seconds = beacon_initial_time_seconds + (
        int(time.time()) - service_initial_time_seconds)

    quantum = beacon_time_seconds // (2 ** scaler)
    print "Beacon time in seconds: %s" % PrintSeconds(beacon_time_seconds)
    print "Beacon quantum: %s" % quantum
    print "Start of quantum: %s" % PrintSeconds(quantum * (2 ** scaler))
    print "End of quantum: %s" % PrintSeconds((quantum + 1) * (2 ** scaler))
    print

    eid = GetAndPrintEid(ik, scaler, beacon_time_seconds)
    # Print the request to get the beacon.
    print
    print "Get beacon:"
    print "  hget /v1beta1/beacons/4!%s" % ToHex(eid)

  elif command == "keygen":
    if len(args) != 1:
      Usage()
    source = Curve25519.ToPrivate(GetBinary(args[0], 32))
    PrintBinary("Private", source)
    public = Curve25519.ScalarMult(source)
    PrintBinary("Public", public)

  elif command == "shared":
    if len(args) != 2:
      Usage()
    private = Curve25519.ToPrivate(GetBinary(args[0], 32))
    PrintBinary("Private", private)
    public = GetBinary(args[1], 32)
    PrintBinary("Public", public)
    PrintBinary("Shared secret", Curve25519.ScalarMult(private, public))

  elif command == "ik":
    if len(args) != 3:
      Usage()
    shared = GetBinary(args[0], 32)
    PrintBinary("Shared secret", shared)
    service_public_key = GetBinary(args[1], 32)
    PrintBinary("Service public key", service_public_key)
    beacon_public_key = GetBinary(args[2], 32)
    PrintBinary("Beacon public key", beacon_public_key)
    ik = GetAndPrintIdentityKey(shared, service_public_key, beacon_public_key)

  elif command == "eid":
    if len(args) != 3:
      Usage()
    ik = GetBinary(args[0], 16)
    PrintBinary("Identity Key", ik)
    scaler = int(args[1], 0)
    beacon_time_seconds = int(args[2], 0)
    GetAndPrintEid(ik, scaler, beacon_time_seconds)

  else:
    Usage()

  return 0


if __name__ == "__main__":
  if len(sys.argv) < 2:
    Usage()
  sys.exit(main(sys.argv[1], sys.argv[2:]))

