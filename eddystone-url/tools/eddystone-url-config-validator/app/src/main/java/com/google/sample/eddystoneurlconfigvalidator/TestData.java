/*
 * Copyright 2015 Google Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.google.sample.eddystoneurlconfigvalidator;

import android.bluetooth.BluetoothGatt;

import org.uribeacon.beacon.ConfigUriBeacon;

class TestData {

  public static final byte[] SHORT_LOCK_KEY = new byte[15];
  public static final byte[] BASIC_LOCK_KEY = new byte[16];
  public static final byte[] WRONG_LOCK_KEY = createWrongKey();

  private static byte[] createWrongKey() {
    byte[] wrongKey = new byte[16];
    wrongKey[0] = 1;
    return wrongKey;
  }

  public static final byte[] LONG_LOCK_KEY = new byte[17];
  public static final byte[] UNLOCKED_STATE = new byte[]{0};
  public static final byte[] LOCKED_STATE = new byte[]{1};
  public static final byte[] BASIC_GENERAL_DATA = new byte[]{1};
  public static final byte[] BASIC_GENERAL_DATA_2 = new byte[]{2};
  public static final byte[][] MULTIPLE_GENERAL_DATA = new byte[][]{BASIC_GENERAL_DATA_2,
      BASIC_GENERAL_DATA};
  public static final byte[] SHORT_FLAGS = new byte[]{};
  public static final byte[] LONG_FLAGS = new byte[2];
  public static final byte[] LONG_URI = new byte[19];
  public static final byte[] SHORT_TX_POWER_LEVELS = new byte[3];
  public static final byte[] BASIC_TX_POWER_LEVELS = new byte[4];
  public static final byte[] BASIC_TX_POWER_LEVELS_2 = new byte[]{1, 1, 1, 1};
  public static final byte[][] MULTIPLE_TX_POWER_LEVELS = new byte[][]{BASIC_TX_POWER_LEVELS_2,
      BASIC_TX_POWER_LEVELS};
  public static final byte[] LONG_TX_POWER_LEVELS = new byte[5];
  public static final byte[] SHORT_POWER_MODE = new byte[0];
  public static final byte[] LONG_POWER_MODE = new byte[2];
  public static final byte[] INVALID_POWER_MODE = new byte[]{5};
  public static final byte[] SHORT_PERIOD = new byte[1];
  // period = 999
  public static final byte[] BASIC_PERIOD = new byte[]{(byte) 0xE7, 0x03};
  // period = 1001
  public static final byte[] BASIC_PERIOD_2 = new byte[]{(byte) 0xE9, 0x03};
  // period = 1
  public static final byte[] LOW_PERIOD = new byte[]{1, 0};
  // period = 0
  public static final byte[] ZERO_PERIOD = new byte[]{0, 0};
  public static final byte[][] MULTIPLE_BASIC_PERIOD = new byte[][]{BASIC_PERIOD_2, BASIC_PERIOD};
  public static final byte[] LONG_PERIOD = new byte[3];
  public static final byte[] SHORT_RESET = new byte[0];
  public static final byte[] LONG_RESET = new byte[2];
  public static final int[] VALID_LENGTH_AUTHORIZATION_ERRORS = new int[]{ConfigUriBeacon.INSUFFICIENT_AUTHORIZATION, BluetoothGatt.GATT_INVALID_ATTRIBUTE_LENGTH};
}
