#!/usr/bin/env python3
#
# Copyright 2015 Opera Software ASA. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import sys
import subprocess
from optparse import OptionParser

parser = OptionParser(usage="%prog [options] [url]")

parser.add_option("-s", "--stop", dest="stop",
        action="store_true", default=False,
        help="Stop advertising")

parser.add_option("-v", "--verbose", dest="verbose",
        action="store_true", default=False,
        help="Print lots of debug output")

(options, args) = parser.parse_args()

# The default uri
uri = "https://goo.gl/SkcDTN"

if len(args) > 0:
    uri = args[0]

schemes = [
        "http://www.",
        "https://www.",
        "http://",
        "https://",
        ]

expansions = [
        ".com/", ".org/", ".edu/", ".net/", ".info/", ".biz/", ".gov/",
        ".com", ".org", ".edu", ".net", ".info", ".biz", ".gov",
        ]

def verboseOutput(text):
    if options.verbose:
        sys.stderr.write(text + "\n")

def encodeUri(uri):
    i = 0
    data = []

    for s in range(len(schemes)):
        scheme = schemes[s]
        if uri.startswith(scheme):
            data.append(s)
            i += len(scheme)
            break;
    else:
        raise Exception("Invalid uri scheme")

    while i < len(uri):
        if uri[i] == '.':
            for e in range(len(expansions)):
                expansion = expansions[e]
                if uri.startswith(expansion, i):
                    data.append(e)
                    i += len(expansion)
                    break
            else:
                data.append(0x2E)
                i += 1
        else:
            data.append(ord(uri[i]))
            i += 1

    return data

def encodeMessage(uri):
    encodedUri = encodeUri(uri)
    encodedUriLength = len(encodedUri)

    verboseOutput("Encoded uri length: " + str(encodedUriLength))

    if encodedUriLength > 18:
        raise Exception("Encoded url too long (max 18 bytes)")

    message = [
            0x02,   # Flags length
            0x01,   # Flags data type value
            0x1a,   # Flags data

            0x03,   # Service UUID length
            0x03,   # Service UUID data type value
            0xaa,   # 16-bit Eddystone UUID
            0xfe,   # 16-bit Eddystone UUID

            5 + len(encodedUri), # Service Data length
            0x16,   # Service Data data type value
            0xaa,   # 16-bit Eddystone UUID
            0xfe,   # 16-bit Eddystone UUID

            0x10,   # Eddystone-URL frame type
            0xed,   # txpower
            ]

    message += encodedUri

    return message

def systemCall(command):
    verboseOutput(command)
    child = subprocess.Popen(["-c", command],
            stdout = subprocess.PIPE,
            stderr = subprocess.PIPE,
            shell = True)
    child.communicate()

def advertise(uri):
    verboseOutput("Advertising: " + uri)
    message = encodeMessage(uri)

    # Prepend the length of the whole message
    message.insert(0, len(message))

    # Pad message to 32 bytes for hcitool
    while len(message) < 32: message.append(0x00)

    # Make a list of hex strings from the list of numbers
    message = map(lambda x: "%02x" % x, message)

    # Concatenate all the hex strings, separated by spaces
    message = " ".join(message)
    verboseOutput("Message: " + message)

    systemCall("sudo hciconfig hci0 up")
    # Stop advertising
    systemCall("sudo hcitool -i hci0 cmd 0x08 0x000a 00")

    # Set message
    systemCall("sudo hcitool -i hci0 cmd 0x08 0x0008 " + message)

    # Resume advertising
    systemCall("sudo hcitool -i hci0 cmd 0x08 0x000a 01")

def stopAdvertising():
    verboseOutput("Stopping advertising")
    systemCall("sudo hcitool -i hci0 cmd 0x08 0x000a 00")

try:
    if options.stop:
        stopAdvertising()
    else:
        advertise(uri)
except Exception as e:
    sys.stderr.write("Exception: " + str(e) + "\n")
    exit(1)
