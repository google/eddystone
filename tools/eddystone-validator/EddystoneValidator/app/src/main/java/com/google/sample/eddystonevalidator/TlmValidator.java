// Copyright 2015 Google Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

package com.google.sample.eddystonevalidator;

import static com.google.sample.eddystonevalidator.Constants.MAX_EXPECTED_TEMP;
import static com.google.sample.eddystonevalidator.Constants.MIN_EXPECTED_TEMP;

import android.util.Log;

import java.nio.ByteBuffer;
import java.util.Arrays;


/**
 * Basic validation of an Eddystone-TLM frame.
 * <p>
 * @see <a href="https://github.com/google/eddystone/eddystone-tlm">TLM frame specification</a>
 */
public class TlmValidator {
  private static final String TAG = TlmValidator.class.getSimpleName();
  private TlmValidator() {}

  static void validate(String deviceAddress, byte[] serviceData, Beacon beacon) {
    beacon.hasTlmFrame = true;

    // The service data for a TLM frame should vary with each broadcast, but depending on the
    // firmware implementation a couple of consecutive TLM frames may be broadcast. Store the
    // frame only if few seconds have passed since we last saw one.
    byte[] previousTlm = null;
    if (beacon.tlmServiceData == null) {
      beacon.tlmServiceData = serviceData;
      beacon.timestamp = System.currentTimeMillis();
    }
    else if (System.currentTimeMillis() - beacon.timestamp > 3000) {
      beacon.timestamp = System.currentTimeMillis();
      previousTlm = beacon.tlmServiceData.clone();
      if (Arrays.equals(beacon.tlmServiceData, serviceData)) {
        String err =
          "TLM service data was identical to recent TLM frame:\n" + Utils.toHexString(serviceData);
        beacon.tlmStatus.errors.add(err);
        logDeviceError(deviceAddress, err);
        beacon.tlmServiceData = serviceData;
        return;
      }
    }

    ByteBuffer buf = ByteBuffer.wrap(serviceData);
    buf.get();  // We already know the frame type byte is 0x20.

    // The version should be zero.
    byte version = buf.get();
    beacon.tlmStatus.version = String.format("0x%02X", version);
    if (version != 0x00) {
      String err = String.format("Bad TLM version, expected 0x00, got %02X", version);
      beacon.tlmStatus.version = err;
      logDeviceError(deviceAddress, err);
    }

    // Battery voltage should be sane. Zero is fine if the device is externally powered, but
    // it shouldn't be negative or unreasonably high.
    short voltage = buf.getShort();
    beacon.tlmStatus.voltage = String.valueOf(voltage);
    if (voltage < 0 || voltage > 10000) {
      String err = "Expected TLM voltage to be between 0 and 10000, got " + voltage;
      beacon.tlmStatus.voltage = err;
      logDeviceError(deviceAddress, err);
    }

    // Temp varies a lot with the hardware and the margins appear to be very wide. USB beacons
    // in particular can report quite high temps. Let's at least check they're partially sane.
    byte tempIntegral = buf.get();
    int tempFractional = (buf.get() & 0xff);
    float temp = tempIntegral + (tempFractional / 256.0f);
    beacon.tlmStatus.temp = String.valueOf(temp);
    if (temp < MIN_EXPECTED_TEMP || temp > MAX_EXPECTED_TEMP) {
      String err = String.format("Expected TLM temperature to be between %d and %d, got %.2f",
                                 MIN_EXPECTED_TEMP, MAX_EXPECTED_TEMP, temp);
      beacon.tlmStatus.errors.add(err);
      logDeviceError(deviceAddress, err);
    }

    // Check the PDU count is increasing from frame to frame.
    int advCnt = buf.getInt();
    beacon.tlmStatus.advCnt = String.valueOf(advCnt);
    if (advCnt < 0) {
      String err = "Expected TLM ADV count to be positive, got " + advCnt;
      beacon.tlmStatus.errors.add(err);
      logDeviceError(deviceAddress, err);
    }
    if (previousTlm != null) {
      int previousAdvCnt = ByteBuffer.wrap(previousTlm, 6, 4).getInt();
      if (previousAdvCnt == advCnt) {
        String err = "Expected increasing TLM PDU count but unchanged from " + advCnt;
        beacon.tlmStatus.errors.add(err);
        logDeviceError(deviceAddress, err);
      }
    }

    // Check that the time since boot is increasing.
    int uptime = buf.getInt();
    beacon.tlmStatus.secCnt = String.valueOf(uptime);
    if (uptime < 0) {
      String err = "Expected TLM time since boot to be positive, got " + uptime;
      beacon.tlmStatus.errors.add(err);
      logDeviceError(deviceAddress, err);
    }
    if (previousTlm != null) {
      int previousUptime = ByteBuffer.wrap(previousTlm, 10, 4).getInt();
      if (previousUptime == uptime) {
        String err = "Expected increasing TLM time since boot but unchanged from " + uptime;
        beacon.tlmStatus.errors.add(err);
        logDeviceError(deviceAddress, err);
      }
    }

    byte[] rfu = Arrays.copyOfRange(serviceData, 14, 20);
    for (byte b : rfu) {
      if (b != 0x00) {
        String err = "Expected TLM RFU bytes to be 0x00, were " + Utils.toHexString(rfu);
        beacon.tlmStatus.errors.add(err);
        logDeviceError(deviceAddress, err);
        break;
      }
    }
  }

  private static void logDeviceError(String deviceAddress, String err) {
    Log.e(TAG, deviceAddress + ": " + err);
  }
}
