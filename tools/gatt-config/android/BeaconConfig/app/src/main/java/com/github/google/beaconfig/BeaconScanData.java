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

import android.bluetooth.le.ScanResult;
import android.util.Log;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import com.github.google.beaconfig.utils.Utils;

/**
 * Holder for all the data that the scanner from the Scanning Activity receives about a particular
 * beacon. This data will then be displayed in the recyclerView.
 */

public class BeaconScanData {
    public static final String TAG = BeaconScanData.class.getSimpleName();

    final String deviceAddress;
    int rssi;
    String name;
    boolean connectable = false;

    final List<byte[]> uidFrameTypes;
    byte[] tlmFrameType;
    final List<byte[]> urlFrameTypes;
    byte[] eidFrameType;

    BeaconScanData(ScanResult sr) {
        this.deviceAddress = sr.getDevice().getAddress();
        if (sr.getDevice().getName() != null) {
            name = sr.getDevice().getName();
        } else {
            name = "[no name]";
        }
        uidFrameTypes = new ArrayList<>();
        urlFrameTypes = new ArrayList<>();

        update(sr);
    }

    public void update(ScanResult sr) {
        rssi = sr.getRssi();
        byte[] serviceData = sr.getScanRecord().getServiceData(Constants.EDDYSTONE_SERVICE_UUID);
        if (serviceData == null || serviceData.length == 0) {
            serviceData = sr.getScanRecord().getServiceData(Constants.EDDYSTONE_CONFIGURATION_UUID);
            if (serviceData == null || Utils.slotIsEmpty(serviceData)) {
                connectable = true;
                return;
            } else {
                String err = "No suitable service data.";
                Log.d(TAG, err);
            }
        }

        switch (serviceData[0]) {
            case Constants.UID_FRAME_TYPE:
                synchronized (uidFrameTypes) {
                    for (byte[] uidServiceData : uidFrameTypes) {
                        if (Arrays.equals(serviceData, uidServiceData)) {
                            return;
                        }
                    }
                    uidFrameTypes.add(serviceData);
                }
                break;
            case Constants.URL_FRAME_TYPE:
                synchronized (urlFrameTypes) {
                    for (byte[] urlServiceData : urlFrameTypes) {
                        if (Arrays.equals(serviceData, urlServiceData)) {
                            return;
                        }
                    }
                    urlFrameTypes.add(serviceData);
                }
                break;
            case Constants.TLM_FRAME_TYPE:
                tlmFrameType = serviceData;
                break;
            case Constants.EID_FRAME_TYPE:
                eidFrameType = serviceData;
                break;
            default:
                String err = String.format("Invalid frame type byte %02X", serviceData[0]);
                Log.d(TAG, err);
        }
    }
}
