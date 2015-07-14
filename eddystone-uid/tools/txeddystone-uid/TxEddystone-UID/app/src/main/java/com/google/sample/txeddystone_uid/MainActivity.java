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

package com.google.sample.txeddystone_uid;

import android.app.Activity;
import android.app.AlertDialog;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothManager;
import android.bluetooth.le.AdvertiseCallback;
import android.bluetooth.le.AdvertiseData;
import android.bluetooth.le.AdvertiseSettings;
import android.bluetooth.le.BluetoothLeAdvertiser;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.os.ParcelUuid;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.Switch;
import android.widget.Toast;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.util.Random;

/**
 * A simple app that can advertise Eddystone-UID frames. The namespace and instance parts of
 * the beacon ID are separately configurable.
 */
public class MainActivity extends Activity {
  private static final String TAG = "EddystoneAdvertiser";
  private static final int REQUEST_ENABLE_BLUETOOTH = 1;
  private static final String SHARED_PREFS_NAME = "txeddystone-uid-prefs";

  // The Eddystone Service UUID, 0xFEAA. See https://github.com/google/eddystone
  private static final ParcelUuid SERVICE_UUID =
      ParcelUuid.fromString("0000FEAA-0000-1000-8000-00805F9B34FB");

  // Broadcasts at medium power with minimum delay between advertisements.
  private static final AdvertiseSettings ADV_SETTINGS = new AdvertiseSettings.Builder()
      .setAdvertiseMode(AdvertiseSettings.ADVERTISE_MODE_LOW_LATENCY)
      .setTxPowerLevel(AdvertiseSettings.ADVERTISE_TX_POWER_MEDIUM)
      .setConnectable(true)
      .build();

  // Used to remember the most recently used namespace and instance ID values.
  private SharedPreferences sharedPreferences;

  private BluetoothLeAdvertiser adv;
  private AdvertiseCallback advertiseCallback;

  private Switch txSwitch;
  private EditText namespace;
  private Button rndNamespace;
  private EditText instance;
  private Button rndInstance;

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.activity_main);
    sharedPreferences = getSharedPreferences(SHARED_PREFS_NAME, 0);
    init();
  }

  @Override
  protected void onActivityResult(int requestCode, int resultCode, Intent data) {
    super.onActivityResult(requestCode, resultCode, data);
    if (requestCode == REQUEST_ENABLE_BLUETOOTH) {
      if (resultCode == Activity.RESULT_OK) {
        init();
      } else {
        finish();
      }
    }
  }

  @Override
  protected void onPause() {
    super.onPause();
    if (namespace != null && instance != null) {
      SharedPreferences.Editor editor = sharedPreferences.edit();
      editor.putString("namespace", namespace.getText().toString());
      editor.putString("instance", instance.getText().toString());
      editor.apply();
    }
  }

  // Checks if Bluetooth advertising is supported on the device and requests enabling if necessary.
  private void init() {
    BluetoothManager manager = (BluetoothManager) getApplicationContext().getSystemService(
        Context.BLUETOOTH_SERVICE);
    BluetoothAdapter btAdapter = manager.getAdapter();
    if (btAdapter == null) {
      showFinishingAlertDialog("Bluetooth Error", "Bluetooth not detected on device");
    } else if (!btAdapter.isEnabled()) {
      Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
      this.startActivityForResult(enableBtIntent, REQUEST_ENABLE_BLUETOOTH);
    } else if (!btAdapter.isMultipleAdvertisementSupported()) {
      showFinishingAlertDialog("Not supported", "BLE advertising not supported on this device");
    } else {
      adv = btAdapter.getBluetoothLeAdvertiser();
      advertiseCallback = createAdvertiseCallback();
      buildUi();
    }
  }

  // Pops an AlertDialog that quits the app on OK.
  private void showFinishingAlertDialog(String title, String message) {
    new AlertDialog.Builder(this)
        .setTitle(title)
        .setMessage(message)
        .setPositiveButton("OK", new DialogInterface.OnClickListener() {
          @Override
          public void onClick(DialogInterface dialogInterface, int i) {
            finish();
          }
        }).show();
  }

  private AdvertiseCallback createAdvertiseCallback() {
    return new AdvertiseCallback() {
      @Override
      public void onStartFailure(int errorCode) {
        switch (errorCode) {
          case ADVERTISE_FAILED_DATA_TOO_LARGE:
            showToastAndLogError("ADVERTISE_FAILED_DATA_TOO_LARGE");
            break;
          case ADVERTISE_FAILED_TOO_MANY_ADVERTISERS:
            showToastAndLogError("ADVERTISE_FAILED_TOO_MANY_ADVERTISERS");
            break;
          case ADVERTISE_FAILED_ALREADY_STARTED:
            showToastAndLogError("ADVERTISE_FAILED_ALREADY_STARTED");
            break;
          case ADVERTISE_FAILED_INTERNAL_ERROR:
            showToastAndLogError("ADVERTISE_FAILED_INTERNAL_ERROR");
            break;
          case ADVERTISE_FAILED_FEATURE_UNSUPPORTED:
            showToastAndLogError("ADVERTISE_FAILED_FEATURE_UNSUPPORTED");
            break;
          default:
            showToastAndLogError("startAdvertising failed with unknown error " + errorCode);
            break;
        }
      }
    };
  }

  private void buildUi() {
    txSwitch = (Switch) findViewById(R.id.txSwitch);
    txSwitch.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
      @Override
      public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
        if (isChecked) {
          startAdvertising();
        } else {
          stopAdvertising();
        }
      }
    });
    namespace = (EditText) findViewById(R.id.namespace);
    namespace.setText(sharedPreferences.getString("namespace", "01020304050607080910"));
    instance = (EditText) findViewById(R.id.instance);
    instance.setText(sharedPreferences.getString("instance", "AABBCCDDEEFF"));
    rndNamespace = (Button) findViewById(R.id.randomizeNamespace);
    rndNamespace.setOnClickListener(new View.OnClickListener() {
      @Override
      public void onClick(View v) {
        namespace.setText(randomHexString(10));
      }
    });
    rndInstance = (Button) findViewById(R.id.randomizeInstance);
    rndInstance.setOnClickListener(new View.OnClickListener() {
      @Override
      public void onClick(View v) {
        instance.setText(randomHexString(6));
      }
    });
  }

  private void startAdvertising() {
    Log.i(TAG, "Starting ADV");
    if (!isValidHex(namespace.getText().toString(), 10)) {
      namespace.setError("not 10-byte hex");
      txSwitch.setChecked(false);
      return;
    }
    if (!isValidHex(instance.getText().toString(), 6)) {
      instance.setError("not 6-byte hex");
      txSwitch.setChecked(false);
      return;
    }

    byte[] serviceData = null;
    try {
      serviceData = buildServiceData();
    } catch (IOException e) {
      Log.e(TAG, e.toString());
      Toast.makeText(this, "failed to build service data", Toast.LENGTH_SHORT).show();
      txSwitch.setChecked(false);
    }

    AdvertiseData advertiseData = new AdvertiseData.Builder()
        .addServiceData(SERVICE_UUID, serviceData)
        .addServiceUuid(SERVICE_UUID)
        .setIncludeTxPowerLevel(false)
        .setIncludeDeviceName(false)
        .build();

    namespace.setError(null);
    instance.setError(null);
    setEnabledViews(false, namespace, instance, rndNamespace, rndInstance);
    adv.startAdvertising(ADV_SETTINGS, advertiseData, advertiseCallback);
  }

  private void stopAdvertising() {
    Log.i(TAG, "Stopping ADV");
    adv.stopAdvertising(advertiseCallback);
    setEnabledViews(true, namespace, instance, rndNamespace, rndInstance);
  }

  private void setEnabledViews(boolean enabled, View... views) {
    for (View v : views) {
      v.setEnabled(enabled);
    }
  }

  private byte[] buildServiceData() throws IOException {
    byte[] namespaceBytes = toByteArray(namespace.getText().toString());
    byte[] instanceBytes = toByteArray(instance.getText().toString());
    ByteArrayOutputStream os = new ByteArrayOutputStream();
    os.write(new byte[]{(byte) 0x00 /* UID frame type */, (byte) 0x10 /* Tx power */});
    os.write(namespaceBytes);
    os.write(instanceBytes);
    return os.toByteArray();
  }

  private boolean isValidHex(String s, int len) {
    return !(s == null || s.isEmpty()) && (s.length() / 2) == len && s.matches("[0-9A-F]+");
  }

  private byte[] toByteArray(String hexString) {
    // hexString guaranteed valid.
    int len = hexString.length();
    byte[] bytes = new byte[len / 2];
    for (int i = 0; i < len; i += 2) {
      bytes[i / 2] = (byte) ((Character.digit(hexString.charAt(i), 16) << 4)
          + Character.digit(hexString.charAt(i + 1), 16));
    }
    return bytes;
  }

  private String randomHexString(int length) {
    byte[] buf = new byte[length];
    new Random().nextBytes(buf);
    StringBuilder stringBuilder = new StringBuilder();
    for (int i = 0; i < length; i++) {
      stringBuilder.append(String.format("%02X", buf[i]));
    }
    return stringBuilder.toString();
  }

  private void showToast(String message) {
    Toast.makeText(this, message, Toast.LENGTH_LONG).show();
  }

  private void showToastAndLogError(String message) {
    showToast(message);
    Log.e(TAG, message);
  }

}
