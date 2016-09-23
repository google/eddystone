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

import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.le.BluetoothLeScanner;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanFilter;
import android.bluetooth.le.ScanResult;
import android.util.Log;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

import com.github.google.beaconfig.utils.UiUtils;

/**
 * Scanner class using the BluetoothLeScanner to scan for beacons and convoy the result in a
 * ready-to-use way to the scanningActivity.
 *
 * It uses a list of scan filters to filter through the scan results and saves each good scan result
 * in a BeaconScanData structure. All the results are stored in a List which is sent back to the
 * ScanningActivity to display in the UI.
 *
 * ScanResults from the same beacon are used to update the same BeaconScanData object, so that in
 * the end each BeaconScanData corresponds to a unique beacon.
 */
class BeaconScanner {
    private static final String TAG = BeaconScanner.class.getSimpleName();

    private Activity currentActivity;
    private BluetoothLeScanner scanner;
    private ScanCallback scanCallback;
    private List<ScanFilter> scanFilters;
    final private List<BeaconScanData> scanDataList;
    final private Map<String, BeaconScanData> addrToBeacons;

    public BeaconScanner(final Activity currentActivity, BluetoothAdapter bluetoothAdapter) {
        this.currentActivity = currentActivity;
        scanner = bluetoothAdapter.getBluetoothLeScanner();

        scanCallback = new ScanCallback() {
            @Override
            public void onScanResult(int callbackType, ScanResult result) {
                BeaconScanData beaconScanData;
                String address = result.getDevice().getAddress();
                synchronized (addrToBeacons) {
                    if (addrToBeacons.isEmpty() || !addrToBeacons.containsKey(address)) {
                        beaconScanData = new BeaconScanData(result);

                        addrToBeacons.put(address, beaconScanData);
                        scanDataList.add(beaconScanData);
                    } else {
                        beaconScanData = addrToBeacons.get(address);
                        beaconScanData.update(result);
                    }
                }
            }

            @Override
            public void onScanFailed(int errorCode) {
                switch (errorCode) {
                    case SCAN_FAILED_ALREADY_STARTED:
                        Log.d(TAG, "SCAN_FAILED_ALREADY_STARTED");
                        break;
                    case SCAN_FAILED_APPLICATION_REGISTRATION_FAILED:
                        Log.d(TAG, "SCAN_FAILED_APPLICATION_REGISTRATION_FAILED");
                        UiUtils.showToast(currentActivity, "Scan failed. Try to turn bluetooth " +
                                "OFF and ON again");
                        break;
                    case SCAN_FAILED_FEATURE_UNSUPPORTED:
                        Log.d(TAG, "SCAN_FAILED_FEATURE_UNSUPPORTED");
                        break;
                    case SCAN_FAILED_INTERNAL_ERROR:
                        Log.d(TAG, "SCAN_FAILED_INTERNAL_ERROR");
                        break;
                    default:
                        Log.d(TAG, "Scan failed, unknown error code");
                        break;
                }
            }
        };

        scanFilters = new ArrayList<>();
        scanFilters.add(new ScanFilter.Builder().setServiceUuid
                (Constants.EDDYSTONE_SERVICE_UUID).build());
        scanFilters.add(new ScanFilter.Builder().setServiceUuid
                (Constants.EDDYSTONE_CONFIGURATION_UUID).build());
        addrToBeacons = new HashMap<>();
        scanDataList = new ArrayList<>();
    }

    private boolean isScanning = false;
    public void scan() {
        if (isScanning) {
            return;
        }
        isScanning = true;
        scanDataList.clear();
        addrToBeacons.clear();
        scanner.startScan(scanFilters, Constants.SCAN_SETTINGS, scanCallback);
        final ScheduledExecutorService worker =
                Executors.newSingleThreadScheduledExecutor();
        Runnable stop = new Runnable() {
            public void run() {
                synchronized (scanDataList) {
                    scanner.stopScan(scanCallback);
                    isScanning = false;
                    ((ScanningActivity) currentActivity).scanComplete(scanDataList);
                }
            }
        };

                worker.schedule(stop, Constants.SCAN_TIME_SECS, TimeUnit.SECONDS);
    }

}
