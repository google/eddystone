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

import android.util.Log;

import java.nio.ByteBuffer;
import java.util.Arrays;
import java.util.concurrent.TimeUnit;


/**
 * Basic validation of an Eddystone-TLM frame. <p>
 *
 * @see <a href="https://github.com/google/eddystone/eddystone-tlm">TLM frame specification</a>
 */
public class TlmValidator {

  private static final String TAG = TlmValidator.class.getSimpleName();

  // TODO: tests
  static final byte MIN_SERVICE_DATA_LEN = 14;

  // TLM frames only support version 0x00 for now.
  static final byte EXPECTED_VERSION = 0x00;

  // Minimum expected voltage value in beacon telemetry in millivolts.
  static final int MIN_EXPECTED_VOLTAGE = 500;

  // Maximum expected voltage value in beacon telemetry in millivolts.
  static final int MAX_EXPECTED_VOLTAGE = 10000;

  // Value indicating temperature not supported. temp[0] == 0x80, temp[1] == 0x00.
  static final float TEMPERATURE_NOT_SUPPORTED = -128.0f;

  // Minimum expected temperature value in beacon telemetry in degrees Celsius.
  static final float MIN_EXPECTED_TEMP = 0.0f;

  // Maximum expected temperature value in beacon telemetry in degrees Celsius.
  static final float MAX_EXPECTED_TEMP = 60.0f;

  // Maximum expected PDU count in beacon telemetry.
  // The fastest we'd expect to see a beacon transmitting would be about 10 Hz.
  // Given that and a lifetime of ~3 years, any value above this is suspicious.
  static final int MAX_EXPECTED_PDU_COUNT = 10 * 60 * 60 * 24 * 365 * 3;

  // Maximum expected time since boot in beacon telemetry.
  // Given that and a lifetime of ~3 years, any value above this is suspicious.
  static final int MAX_EXPECTED_SEC_COUNT = 10 * 60 * 60 * 24 * 365 * 3;

  // The service data for a TLM frame should vary with each broadcast, but depending on the
  // firmware implementation a couple of consecutive TLM frames may be broadcast. Store the
  // frame only if few seconds have passed since we last saw one.
  static final int STORE_NEXT_FRAME_DELTA_MS = 3000;

  private TlmValidator() {
  }

  static void validate(String deviceAddress, byte[] serviceData, Beacon beacon) {
    beacon.hasTlmFrame = true;

    byte[] previousTlm = null;
    if (beacon.tlmServiceData == null) {
      beacon.tlmServiceData = serviceData;
      beacon.timestamp = System.currentTimeMillis();
    } else if (System.currentTimeMillis() - beacon.timestamp > STORE_NEXT_FRAME_DELTA_MS) {
      beacon.timestamp = System.currentTimeMillis();
      previousTlm = beacon.tlmServiceData.clone();
      if (Arrays.equals(beacon.tlmServiceData, serviceData)) {
        String err =
            "TLM service data was identical to recent TLM frame:\n" + Utils
                .toHexString(serviceData);
        beacon.tlmStatus.errIdentialFrame = err;
        logDeviceError(deviceAddress, err);
        beacon.tlmServiceData = serviceData;
      }
    }

    if (serviceData.length < MIN_SERVICE_DATA_LEN) {
      String err = String.format("TLM frame too short, needs at least %d bytes, got %d",
          MIN_SERVICE_DATA_LEN, serviceData.length);
      beacon.frameStatus.tooShortServiceData = err;
      logDeviceError(deviceAddress, err);
      return;
    }

    ByteBuffer buf = ByteBuffer.wrap(serviceData);
    buf.get();  // We already know the frame type byte is 0x20.

    // The version should be zero.
    byte version = buf.get();
    beacon.tlmStatus.version = String.format("0x%02X", version);
    if (version != EXPECTED_VERSION) {
      String err = String.format("Bad TLM version, expected 0x%02X, got %02X",
          EXPECTED_VERSION, version);
      beacon.tlmStatus.errVersion = err;
      logDeviceError(deviceAddress, err);
    }

    // Battery voltage should be sane. Zero is fine if the device is externally powered, but
    // it shouldn't be negative or unreasonably high.
    short voltage = buf.getShort();
    beacon.tlmStatus.voltage = String.valueOf(voltage);
    if (voltage != 0 && (voltage < MIN_EXPECTED_VOLTAGE || voltage > MAX_EXPECTED_VOLTAGE)) {
      String err = String.format("Expected TLM voltage to be between %d and %d, got %d",
          MIN_EXPECTED_VOLTAGE, MAX_EXPECTED_VOLTAGE, voltage);
      beacon.tlmStatus.errVoltage = err;
      logDeviceError(deviceAddress, err);
    }

    // Temp varies a lot with the hardware and the margins appear to be very wide. USB beacons
    // in particular can report quite high temps. Let's at least check they're partially sane.
    byte tempIntegral = buf.get();
    int tempFractional = (buf.get() & 0xff);
    float temp = tempIntegral + (tempFractional / 256.0f);
    beacon.tlmStatus.temp = String.valueOf(temp);
    if (temp != TEMPERATURE_NOT_SUPPORTED) {
      if (temp < MIN_EXPECTED_TEMP || temp > MAX_EXPECTED_TEMP) {
        String err = String.format("Expected TLM temperature to be between %.2f and %.2f, got %.2f",
            MIN_EXPECTED_TEMP, MAX_EXPECTED_TEMP, temp);
        beacon.tlmStatus.errTemp = err;
        logDeviceError(deviceAddress, err);
      }
    }

    // Check the PDU count is increasing from frame to frame and is neither too low or too high.
    int advCnt = buf.getInt();
    beacon.tlmStatus.advCnt = String.valueOf(advCnt);
    if (advCnt <= 0) {
      String err = "Expected TLM ADV count to be positive, got " + advCnt;
      beacon.tlmStatus.errPduCnt = err;
      logDeviceError(deviceAddress, err);
    }
    if (advCnt > MAX_EXPECTED_PDU_COUNT) {
      String err = String.format("TLM ADV count %d is higher than expected max of %d",
          advCnt, MAX_EXPECTED_PDU_COUNT);
      beacon.tlmStatus.errPduCnt = err;
      logDeviceError(deviceAddress, err);
    }
    if (previousTlm != null) {
      int previousAdvCnt = ByteBuffer.wrap(previousTlm, 6, 4).getInt();
      if (previousAdvCnt == advCnt) {
        String err = "Expected increasing TLM PDU count but unchanged from " + advCnt;
        beacon.tlmStatus.errPduCnt = err;
        logDeviceError(deviceAddress, err);
      }
    }

    // Check that the time since boot is increasing and is neither too low nor too high.
    int uptime = buf.getInt();
    beacon.tlmStatus.secCnt = String.format("%d (%d days)", uptime, TimeUnit.SECONDS.toDays(uptime / 10));
    if (uptime <= 0) {
      String err = "Expected TLM time since boot to be positive, got " + uptime;
      beacon.tlmStatus.errSecCnt = err;
      logDeviceError(deviceAddress, err);
    }
    if (uptime > MAX_EXPECTED_SEC_COUNT) {
      String err = String.format("TLM time since boot %d is higher than expected max of %d",
          uptime, MAX_EXPECTED_SEC_COUNT);
      beacon.tlmStatus.errSecCnt = err;
      logDeviceError(deviceAddress, err);
    }
    if (previousTlm != null) {
      int previousUptime = ByteBuffer.wrap(previousTlm, 10, 4).getInt();
      if (previousUptime == uptime) {
        String err = "Expected increasing TLM time since boot but unchanged from " + uptime;
        beacon.tlmStatus.errSecCnt = err;
        logDeviceError(deviceAddress, err);
      }
    }

    byte[] rfu = Arrays.copyOfRange(serviceData, 14, 20);
    for (byte b : rfu) {
      if (b != 0x00) {
        String err = "Expected TLM RFU bytes to be 0x00, were " + Utils.toHexString(rfu);
        beacon.tlmStatus.errRfu = err;
        logDeviceError(deviceAddress, err);
        break;
      }
    }
  }

  private static void logDeviceError(String deviceAddress, String err) {
    Log.e(TAG, deviceAddress + ": " + err);
  }
}
