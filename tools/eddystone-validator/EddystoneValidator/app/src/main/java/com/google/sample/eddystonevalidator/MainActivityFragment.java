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

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Fragment;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothManager;
import android.bluetooth.le.BluetoothLeScanner;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanFilter;
import android.bluetooth.le.ScanRecord;
import android.bluetooth.le.ScanResult;
import android.bluetooth.le.ScanSettings;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.os.ParcelUuid;
import android.text.Editable;
import android.text.TextWatcher;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.Toast;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * Main UI and logic for scanning and validation of results.
 */
public class MainActivityFragment extends Fragment {
  private static final String TAG = "EddystoneValidator";
  private static final int REQUEST_ENABLE_BLUETOOTH = 1;

  // An aggressive scan for nearby devices that reports immediately.
  private static final ScanSettings SCAN_SETTINGS =
          new ScanSettings.Builder().setScanMode(ScanSettings.SCAN_MODE_LOW_LATENCY).setReportDelay(0)
                  .build();

  // The Eddystone Service UUID, 0xFEAA.
  private static final ParcelUuid EDDYSTONE_SERVICE_UUID =
          ParcelUuid.fromString("0000FEAA-0000-1000-8000-00805F9B34FB");

  private BluetoothLeScanner mScanner;
  private BeaconArrayAdapter mArrayAdapter;

  private List<ScanFilter> mScanFilters;
  private ScanCallback mScanCallback;

  private Map<String /* device address */, Beacon> mDeviceToBeaconMap = new HashMap<>();

  private EditText mFilter;

  @Override
  public void onCreate(final Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    init();
    ArrayList<Beacon> arrayList = new ArrayList<>();
    mArrayAdapter = new BeaconArrayAdapter(getActivity(), R.layout.beacon_list_item, arrayList);
    mScanFilters = new ArrayList<>();
    mScanFilters.add(new ScanFilter.Builder().setServiceUuid(EDDYSTONE_SERVICE_UUID).build());
    mScanCallback = new ScanCallback() {
      @Override
      public void onScanResult(int callbackType, ScanResult result) {
        ScanRecord scanRecord = result.getScanRecord();
        if (scanRecord == null) {
          return;
        }

        String deviceAddress = result.getDevice().getAddress();
        if (!mDeviceToBeaconMap.containsKey(deviceAddress)) {
          Beacon beacon = new Beacon(deviceAddress);
          mDeviceToBeaconMap.put(deviceAddress, beacon);
          mArrayAdapter.add(beacon);
        }

        byte[] serviceData = scanRecord.getServiceData(EDDYSTONE_SERVICE_UUID);
        Log.v(TAG, deviceAddress + " " + Utils.toHexString(serviceData));
        validateServiceData(deviceAddress, serviceData);
      }

      @Override
      public void onScanFailed(int errorCode) {
        switch (errorCode) {
          case SCAN_FAILED_ALREADY_STARTED:
            logErrorAndShowToast("SCAN_FAILED_ALREADY_STARTED");
            break;
          case SCAN_FAILED_APPLICATION_REGISTRATION_FAILED:
            logErrorAndShowToast("SCAN_FAILED_APPLICATION_REGISTRATION_FAILED");
            break;
          case SCAN_FAILED_FEATURE_UNSUPPORTED:
            logErrorAndShowToast("SCAN_FAILED_FEATURE_UNSUPPORTED");
            break;
          case SCAN_FAILED_INTERNAL_ERROR:
            logErrorAndShowToast("SCAN_FAILED_INTERNAL_ERROR");
            break;
          default:
            logErrorAndShowToast("Scan failed, unknown error code");
            break;
        }
      }
    };
  }

  @Override
  public View onCreateView(LayoutInflater inflater,
                           ViewGroup container,
                           Bundle savedInstanceState) {
    View view = inflater.inflate(R.layout.fragment_main, container, false);
    mFilter = (EditText) view.findViewById(R.id.filter);
    mFilter.addTextChangedListener(new TextWatcher() {
      @Override
      public void beforeTextChanged(CharSequence s, int start, int count, int after) {
        // NOP
      }

      @Override
      public void onTextChanged(CharSequence s, int start, int before, int count) {
        // NOP
      }

      @Override
      public void afterTextChanged(Editable s) {
        mArrayAdapter.getFilter().filter(mFilter.getText().toString());
      }
    });
    ListView listView = (ListView)view.findViewById(R.id.listView);
    listView.setAdapter(mArrayAdapter);
    listView.setEmptyView(view.findViewById(R.id.placeholder));
    return view;
  }

  @Override
  public void onPause() {
    super.onPause();
    if (mScanner != null) {
      mScanner.stopScan(mScanCallback);
    }
  }

  @Override
  public void onResume() {
    super.onResume();
    if (mScanner != null) {
      mScanner.startScan(mScanFilters, SCAN_SETTINGS, mScanCallback);
    }
  }

  @Override
  public void onActivityResult(int requestCode, int resultCode, Intent data) {
    super.onActivityResult(requestCode, resultCode, data);
    if (requestCode == REQUEST_ENABLE_BLUETOOTH) {
      if (resultCode == Activity.RESULT_OK) {
        init();
      }
      else {
        getActivity().finish();
      }
    }
  }

  // Attempts to create the mScanner.
  private void init() {
    BluetoothManager manager = (BluetoothManager)getActivity().getApplicationContext()
            .getSystemService(Context.BLUETOOTH_SERVICE);
    BluetoothAdapter btAdapter = manager.getAdapter();
    if (btAdapter == null) {
      showFinishingAlertDialog("Bluetooth Error", "Bluetooth not detected on device");
    }
    else if (!btAdapter.isEnabled()) {
      Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
      this.startActivityForResult(enableBtIntent, REQUEST_ENABLE_BLUETOOTH);
    }
    else {
      mScanner = btAdapter.getBluetoothLeScanner();
    }
  }

  // Pops an AlertDialog that quits the app on OK.
  private void showFinishingAlertDialog(String title, String message) {
    new AlertDialog.Builder(getActivity()).setTitle(title).setMessage(message)
            .setPositiveButton("OK", new DialogInterface.OnClickListener() {
              @Override
              public void onClick(DialogInterface dialogInterface, int i) {
                getActivity().finish();
              }
            }).show();
  }

  // Checks the frame type and hands off the service data to the validation module.
  private void validateServiceData(String deviceAddress, byte[] serviceData) {
    Beacon beacon = mDeviceToBeaconMap.get(deviceAddress);
    if (serviceData == null) {
      String err = "Null Eddystone service data";
      beacon.frameStatus.nullServiceData = err;
      logDeviceError(deviceAddress, err);
      return;
    }
    switch (serviceData[0]) {
      case Constants.UID_FRAME_TYPE:
        UidValidator.validate(deviceAddress, serviceData, beacon);
        break;
      case Constants.TLM_FRAME_TYPE:
        TlmValidator.validate(deviceAddress, serviceData, beacon);
        break;
      case Constants.URL_FRAME_TYPE:
        UrlValidator.validate(deviceAddress, serviceData, beacon);
        break;
      default:
        String err = String.format("Invalid frame type byte %2X %s", serviceData[0], deviceAddress);
        beacon.frameStatus.invalidFrameType = err;
        logDeviceError(deviceAddress, err);
        break;
    }
  }

  private void logErrorAndShowToast(String message) {
    Toast.makeText(getActivity(), message, Toast.LENGTH_SHORT).show();
    Log.e(TAG, message);
  }

  private void logDeviceError(String deviceAddress, String err) {
    Log.e(TAG, deviceAddress + ": " + err);
  }

}
