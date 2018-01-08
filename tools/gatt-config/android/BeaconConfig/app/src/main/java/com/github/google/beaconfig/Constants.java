// Copyright 2016 Google Inc. All rights reserved.
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

package com.github.google.beaconfig;

import android.bluetooth.le.ScanSettings;
import android.os.ParcelUuid;

/**
 * Constants used by other classes.
 */

public class Constants {
    static final int SCAN_TIME_SECS = 4;
    static final int REQUEST_ENABLE_BLUETOOTH = 1;
    static final ScanSettings SCAN_SETTINGS =
            new ScanSettings.Builder()
                    .setScanMode(ScanSettings.SCAN_MODE_LOW_LATENCY)
                    .setReportDelay(0)
                    .build();
    static final ParcelUuid EDDYSTONE_SERVICE_UUID =
            ParcelUuid.fromString("0000FEAA-0000-1000-8000-00805F9B34FB");
    public static final ParcelUuid EDDYSTONE_CONFIGURATION_UUID =
    ParcelUuid.fromString("a3c87500-8ed3-4bdf-8a39-a01bebede295");

    /**
     * Eddystone-UID frame type value.
     */
    public static final byte UID_FRAME_TYPE = 0x00;

    /**
     * Eddystone-URL frame type value.
     */
    public static final byte URL_FRAME_TYPE = 0x10;

    /**
     * Eddystone-TLM frame type value.
     */
    public static final byte TLM_FRAME_TYPE = 0x20;

    /**
     * Eddystone-EID frame type value.
     */
    public static final byte EID_FRAME_TYPE = 0x30;
    public static final byte EMPTY_FRAME_TYPE = 0x40;

    public static final String SLOT_DATA = "slot_data";
    public static final String TX_POWER = "tx_power";
    public static final String ADV_POWER = "adv_power";
    public static final String UID = "UID";
    public static final String URL = "URL";
    public static final String TLM = "TLM";
    public static final String EID = "EID";
    public static final String ADV_INTERVAL = "advertising_interval";
    public static final String BEACON_ADDRESS = "beacon_address";
    public static final String BEACON_NAME = "beacon_name";
    public static final String BROADCAST_CAPABILITIES = "BROADCAST_CAPABILITIES";
    public static final String SLOT_NUMBER = "SLOT_NUMBER";
    public static final String REMAIN_CONNECTABLE = "REMAIN_CONNECTABLE";

    public static final String CONFIG_NAMES = "CONFIG_NAMES";
    public static final String SAVED_CONFIGURATIONS = "SAVED_CONFIGURATIONS";
}
