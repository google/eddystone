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

import static com.google.sample.eddystonevalidator.Constants.MAX_EXPECTED_TX_POWER;
import static com.google.sample.eddystonevalidator.Constants.MIN_EXPECTED_TX_POWER;

import android.util.Log;

import java.util.Arrays;


/**
 * Basic validation of an Eddystone-UID frame. <p>
 *
 * @see <a href="https://github.com/google/eddystone/eddystone-uid">UID frame specification</a>
 */
public class UidValidator {

  private static final String TAG = UidValidator.class.getSimpleName();

  private UidValidator() {
  }

  static void validate(String deviceAddress, byte[] serviceData, Beacon beacon) {
    beacon.hasUidFrame = true;

    // Tx power should have reasonable values.
    int txPower = (int) serviceData[1];
    beacon.uidStatus.txPower = txPower;
    if (txPower < MIN_EXPECTED_TX_POWER || txPower > MAX_EXPECTED_TX_POWER) {
      String err = String
          .format("Expected UID Tx power between %d and %d, got %d", MIN_EXPECTED_TX_POWER,
              MAX_EXPECTED_TX_POWER, txPower);
      beacon.uidStatus.errTx = err;
      logDeviceError(deviceAddress, err);
    }

    // The namespace and instance bytes should not be all zeroes.
    byte[] uidBytes = Arrays.copyOfRange(serviceData, 2, 18);
    beacon.uidStatus.uidValue = Utils.toHexString(uidBytes);
    if (Utils.isZeroed(uidBytes)) {
      String err = "UID bytes are all 0x00";
      beacon.uidStatus.errUid = err;
      logDeviceError(deviceAddress, err);
    }

    // If we have a previous frame, verify the ID isn't changing.
    if (beacon.uidServiceData == null) {
      beacon.uidServiceData = serviceData.clone();
    } else {
      byte[] previousUidBytes = Arrays.copyOfRange(beacon.uidServiceData, 2, 18);
      if (!Arrays.equals(uidBytes, previousUidBytes)) {
        String err = String.format("UID should be invariant.\nLast: %s\nthis: %s",
            Utils.toHexString(previousUidBytes),
            Utils.toHexString(uidBytes));
        beacon.uidStatus.errUid = err;
        logDeviceError(deviceAddress, err);
        beacon.uidServiceData = serviceData.clone();
      }
    }

    // Last two bytes in frame are RFU and should be zeroed.
    byte[] rfu = Arrays.copyOfRange(serviceData, 18, 20);
    if (rfu[0] != 0x00 || rfu[1] != 0x00) {
      String err = "Expected UID RFU bytes to be 0x00, were " + Utils.toHexString(rfu);
      beacon.uidStatus.errRfu = err;
      logDeviceError(deviceAddress, err);
    }
  }

  private static void logDeviceError(String deviceAddress, String err) {
    Log.e(TAG, deviceAddress + ": " + err);
  }
}
