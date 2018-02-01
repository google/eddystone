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

package com.google.sample.txeddystone;

import static com.google.sample.txeddystone.Utils.isValidHex;
import static com.google.sample.txeddystone.Utils.setEnabledViews;
import static com.google.sample.txeddystone.Utils.toByteArray;

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
import android.os.Handler;
import android.os.Looper;
import android.os.ParcelUuid;
import android.support.design.widget.TabLayout;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentPagerAdapter;
import android.support.v4.view.ViewPager;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.Spinner;
import android.widget.Switch;
import android.widget.Toast;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;

/**
 * Implements a tabbed layout of 4 BLE advertisers, each capable of broadcasting
 * an Eddystone UID, TLM or URL frame, each with independent Tx power and mode.
 */
public class MainActivity extends AppCompatActivity {
  private static final int REQUEST_ENABLE_BLUETOOTH = 1;
  private static final int NUM_ADVERTISERS = 4;
  private static final String SHARED_PREFS_NAME = "txeddystone-prefs";
  private static final String PREF_FRAME_TYPE = "pref-frame-type";
  private static final String PREF_TX_POWER = "pref-tx-power";
  private static final String PREF_TX_MODE = "pref-tx-mode";
  private static final String PREF_NAMESPACE = "pref-namespace";
  private static final String PREF_INSTANCE = "pref-instance";
  private static final String PREF_VOLTAGE = "pref-voltage";
  private static final String PREF_TEMPERATURE = "pref-temperature";
  private static final String PREF_ADVCNT = "pref-advcnt";
  private static final String PREF_SECCNT = "pref-seccnt";
  private static final String PREF_URL = "pref-url";
  private static final byte FRAME_TYPE_UID = 0x00;
  private static final byte FRAME_TYPE_URL = 0x10;
  private static final byte FRAME_TYPE_TLM = 0x20;
  private static final ParcelUuid SERVICE_UUID =
      ParcelUuid.fromString("0000FEAA-0000-1000-8000-00805F9B34FB");

  /**
   * The {@link android.support.v4.view.PagerAdapter} that will provide fragments for each of the
   * sections. We use a {@link FragmentPagerAdapter} derivative, which will keep every loaded
   * fragment in memory. If this becomes too memory intensive, it may be best to switch to a {@link
   * android.support.v4.app.FragmentStatePagerAdapter}.
   */
  private SectionsPagerAdapter mSectionsPagerAdapter;

  /**
   * The {@link ViewPager} that will host the section contents.
   */
  private ViewPager mViewPager;

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.activity_main);

    init();

    Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
    setSupportActionBar(toolbar);
    // Create the adapter that will return a fragment for each of the primary sections of the
    // activity.
    mSectionsPagerAdapter = new SectionsPagerAdapter(getSupportFragmentManager());

    // Set up the ViewPager with the sections adapter and hold three of them in the pager's
    // cache so the view state is easy to manage.
    mViewPager = (ViewPager) findViewById(R.id.container);
    mViewPager.setAdapter(mSectionsPagerAdapter);
    mViewPager.setOffscreenPageLimit(NUM_ADVERTISERS - 1);

    TabLayout tabLayout = (TabLayout) findViewById(R.id.tabs);
    tabLayout.setupWithViewPager(mViewPager);
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

  /**
   * A {@link FragmentPagerAdapter} that returns a fragment corresponding to one of the
   * sections/tabs/pages.
   */
  public class SectionsPagerAdapter extends FragmentPagerAdapter {

    public SectionsPagerAdapter(FragmentManager fm) {
      super(fm);
    }

    @Override
    public Fragment getItem(int position) {
      return AdvertiserFragment.newInstance(position + 1);
    }

    @Override
    public int getCount() {
      return NUM_ADVERTISERS;
    }

    @Override
    public CharSequence getPageTitle(int position) {
      switch (position) {
        case 0:
          return "ADV 1";
        case 1:
          return "ADV 2";
        case 2:
          return "ADV 3";
        case 3:
          return "ADV 4";
      }
      return null;
    }
  }

  /**
   * A Fragment containing all the UI and logic to advertise one of the Eddystone frame types.
   */
  public static class AdvertiserFragment extends Fragment {
    private static final String TAG = AdvertiserFragment.class.getSimpleName();

    // The fragment argument representing the section number for this fragment.
    private static final String ADV_NUMBER = "adv_number";

    // Each fragment has its own shared prefs where we store the most recently used values.
    private SharedPreferences sharedPreferences;

    private static final Handler handler = new Handler(Looper.getMainLooper());

    // The layouts for each frame type.
    private LinearLayout uidLayout;
    private LinearLayout tlmLayout;
    private LinearLayout urlLayout;

    // Frame type selector.
    private Spinner frameType;

    // Elements common to all frame types.
    private Switch txSwitch;
    private Spinner txPower;
    private Spinner txMode;

    // UID fields.
    private EditText namespace;
    private Button rndNamespace;
    private EditText instance;
    private Button rndInstance;

    // TLM fields.
    private EditText voltage;
    private EditText temperature;
    private EditText advCount;
    private EditText secCount;
    private CheckBox rotateCounters;

    // URL fields.
    private EditText url;

    private int advNumber;

    // Used to set the spinners from shared preferences.
    private String frameTypeValue;
    private String txPowerValue;
    private String txModeValue;

    private BluetoothLeAdvertiser advertiser;
    private AdvertiseCallback advertiseCallback;
    private boolean isAdvertising = false;

    /**
     * Returns a new instance of this fragment for the given section number.
     */
    public static AdvertiserFragment newInstance(int sectionNumber) {
      AdvertiserFragment fragment = new AdvertiserFragment();
      Bundle args = new Bundle();
      args.putInt(ADV_NUMBER, sectionNumber);
      fragment.setArguments(args);
      return fragment;
    }

    private String getPrefString(String key, int defaultResId) {
      return sharedPreferences.getString(key, getString(defaultResId));
    }

    @Override
    public void onSaveInstanceState(Bundle outState) {
      super.onSaveInstanceState(outState);
      stopAdvertising();
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
      super.onCreate(savedInstanceState);
      advNumber = getArguments().getInt(ADV_NUMBER);
      sharedPreferences = getContext().getSharedPreferences(SHARED_PREFS_NAME + advNumber, 0);

      frameTypeValue = getPrefString(PREF_FRAME_TYPE, R.string.frame_type_uid);
      txPowerValue = getPrefString(PREF_TX_POWER, R.string.tx_power_high);
      txModeValue = getPrefString(PREF_TX_MODE, R.string.tx_mode_low_latency);

      BluetoothManager manager = (BluetoothManager) getContext().getSystemService(
          Context.BLUETOOTH_SERVICE);
      BluetoothAdapter btAdapter = manager.getAdapter();
      advertiser = btAdapter.getBluetoothLeAdvertiser();

      advertiseCallback = new AdvertiseCallback() {
        @Override
        public void onStartSuccess(AdvertiseSettings settingsInEffect) {
          isAdvertising = true;
        }

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

    @Override
    public void onPause() {
      super.onPause();

      SharedPreferences.Editor editor = sharedPreferences.edit();
      editor.putString(PREF_FRAME_TYPE, frameTypeValue);
      editor.putString(PREF_TX_POWER, txPowerValue);
      editor.putString(PREF_TX_MODE, txModeValue);
      editor.putString(PREF_NAMESPACE, namespace.getText().toString());
      editor.putString(PREF_INSTANCE, instance.getText().toString());
      editor.putString(PREF_VOLTAGE, voltage.getText().toString());
      editor.putString(PREF_TEMPERATURE, temperature.getText().toString());
      editor.putString(PREF_ADVCNT, advCount.getText().toString());
      editor.putString(PREF_SECCNT, secCount.getText().toString());
      editor.putString(PREF_URL, url.getText().toString());
      editor.apply();
    }

    @Override
    public void onResume() {
      super.onResume();
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
        Bundle savedInstanceState) {
      View rootView = inflater.inflate(R.layout.fragment_main, container, false);

      uidLayout = (LinearLayout) rootView.findViewById(R.id.uidLayout);
      tlmLayout = (LinearLayout) rootView.findViewById(R.id.tlmLayout);
      urlLayout = (LinearLayout) rootView.findViewById(R.id.urlLayout);

      frameType = (Spinner) rootView.findViewById(R.id.frameTypeSpinner);
      ArrayAdapter<CharSequence> frameTypeAdapter = ArrayAdapter.createFromResource(
          AdvertiserFragment.this.getContext(), R.array.frame_type_array,
          android.R.layout.simple_spinner_dropdown_item);
      frameTypeAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
      frameType.setAdapter(frameTypeAdapter);
      for (int i = 0; i < frameType.getCount(); i++) {
        if (frameType.getItemAtPosition(i).equals(frameTypeValue)) {
          frameType.setSelection(i);
        }
      }
      setFrameTypeSelectionListener();

      txSwitch = (Switch) rootView.findViewById(R.id.txSwitch);
      txSwitch.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
        @Override
        public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
          if (isChecked) {
            startAdvertising();
          } else {
            stopAdvertising();
            handler.removeCallbacksAndMessages(null);
          }
        }
      });

      txPower = (Spinner) rootView.findViewById(R.id.txPower);
      ArrayAdapter<CharSequence> txPowerAdapter = ArrayAdapter.createFromResource(
          AdvertiserFragment.this.getContext(), R.array.tx_power_array,
          android.R.layout.simple_spinner_dropdown_item);
      txPowerAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
      txPower.setAdapter(txPowerAdapter);
      for (int i = 0; i < txPower.getCount(); i++) {
        if (txPower.getItemAtPosition(i).equals(txPowerValue)) {
          txPower.setSelection(i);
        }
      }
      setTxPowerSelectionListener();

      txMode = (Spinner) rootView.findViewById(R.id.txMode);
      ArrayAdapter<CharSequence> txModeAdapter = ArrayAdapter.createFromResource(
          AdvertiserFragment.this.getContext(), R.array.tx_mode_array,
          android.R.layout.simple_spinner_dropdown_item);
      txModeAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
      txMode.setAdapter(txModeAdapter);
      for (int i = 0; i < txMode.getCount(); i++) {
        if (txMode.getItemAtPosition(i).equals(txModeValue)) {
          txMode.setSelection(i);
        }
      }
      setTxModeSelectionListener();

      namespace = (EditText) rootView.findViewById(R.id.namespace);
      namespace.setText(sharedPreferences.getString(PREF_NAMESPACE, "00000000000000000000"));

      rndNamespace = (Button) rootView.findViewById(R.id.randomizeNamespace);
      rndNamespace.setOnClickListener(new OnClickListener() {
        @Override
        public void onClick(View view) {
          namespace.setText(Utils.randomHexString(10));
        }
      });

      instance = (EditText) rootView.findViewById(R.id.instance);
      instance.setText(sharedPreferences.getString(PREF_INSTANCE, "AAAAAAAAAAAA"));

      rndInstance = (Button) rootView.findViewById(R.id.randomizeInstance);
      rndInstance.setOnClickListener(new OnClickListener() {
        @Override
        public void onClick(View view) {
          instance.setText(Utils.randomHexString(6));
        }
      });

      voltage = (EditText) rootView.findViewById(R.id.voltage);
      voltage.setText(sharedPreferences.getString(PREF_VOLTAGE, "2987"));

      temperature = (EditText) rootView.findViewById(R.id.temperature);
      temperature.setText(sharedPreferences.getString(PREF_TEMPERATURE, "23.4"));

      advCount = (EditText) rootView.findViewById(R.id.advcnt);
      advCount.setText(sharedPreferences.getString(PREF_ADVCNT, "123"));

      secCount = (EditText) rootView.findViewById(R.id.seccnt);
      secCount.setText(sharedPreferences.getString(PREF_SECCNT, "456"));

      rotateCounters = (CheckBox) rootView.findViewById(R.id.rotateCounters);

      url = (EditText) rootView.findViewById(R.id.url);
      url.setText(sharedPreferences.getString(PREF_URL, "https://www.google.co.uk"));

      return rootView;
    }

    private void setFrameTypeSelectionListener() {
      frameType.setOnItemSelectedListener(new OnItemSelectedListener() {
        @Override
        public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
          String selected = (String) parent.getItemAtPosition(position);
          if (!selected.equals(frameTypeValue) && txSwitch.isChecked()) {
            txSwitch.setChecked(false);
          }
          if (selected.equals(getString(R.string.frame_type_uid))) {
            tlmLayout.setVisibility(View.GONE);
            urlLayout.setVisibility(View.GONE);
            uidLayout.setVisibility(View.VISIBLE);
          } else if (selected.equals(getString(R.string.frame_type_tlm))) {
            uidLayout.setVisibility(View.GONE);
            urlLayout.setVisibility(View.GONE);
            tlmLayout.setVisibility(View.VISIBLE);
          } else if (selected.equals(getString(R.string.frame_type_url))) {
            tlmLayout.setVisibility(View.GONE);
            uidLayout.setVisibility(View.GONE);
            urlLayout.setVisibility(View.VISIBLE);
          }
          frameTypeValue = selected;
        }

        @Override
        public void onNothingSelected(AdapterView<?> adapterView) {
          // NOP
        }
      });
    }

    private void setTxPowerSelectionListener() {
      txPower.setOnItemSelectedListener(new OnItemSelectedListener() {
        @Override
        public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
          txPowerValue = (String) parent.getItemAtPosition(position);
        }

        @Override
        public void onNothingSelected(AdapterView<?> parent) {
          // NOP
        }
      });
    }

    private void setTxModeSelectionListener() {
      txMode.setOnItemSelectedListener(new OnItemSelectedListener() {
        @Override
        public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
          txModeValue = (String) parent.getItemAtPosition(position);
        }

        @Override
        public void onNothingSelected(AdapterView<?> parent) {
          // NOP
        }
      });
    }

    private int txModeValueToSetting() {
      if (txModeValue.equals(getString(R.string.tx_mode_low_latency))) {
        return AdvertiseSettings.ADVERTISE_MODE_LOW_LATENCY;
      } else if (txModeValue.equals(getString(R.string.tx_mode_balanced))) {
        return AdvertiseSettings.ADVERTISE_MODE_BALANCED;
      } else {
        return AdvertiseSettings.ADVERTISE_MODE_LOW_POWER;
      }
    }

    // Returns a very rough approximation of the Tx mode's frequency.
    private int txModeValueToHertz() {
      if (txModeValue.equals(getString(R.string.tx_mode_low_latency))) {
        return 7;
      } else if (txModeValue.equals(getString(R.string.tx_mode_balanced))) {
        return 3;
      } else {
        return 1;
      }
    }

    private int txPowerValueToSetting() {
      if (txPowerValue.equals(getString(R.string.tx_power_high))) {
        return AdvertiseSettings.ADVERTISE_TX_POWER_HIGH;
      } else if (txPowerValue.equals(getString(R.string.tx_power_medium))) {
        return AdvertiseSettings.ADVERTISE_TX_POWER_MEDIUM;
      } else if (txPowerValue.equals(getString(R.string.tx_power_low))) {
        return AdvertiseSettings.ADVERTISE_TX_POWER_LOW;
      } else {
        return AdvertiseSettings.ADVERTISE_TX_POWER_ULTRA_LOW;
      }
    }

    private void startAdvertising() {
      if (isAdvertising) {
        return;
      }
      AdvertiseSettings advertiseSettings = new AdvertiseSettings.Builder()
          .setAdvertiseMode(txModeValueToSetting())
          .setTxPowerLevel(txPowerValueToSetting())
          .setConnectable(true)
          .build();
      if (frameTypeValue.equals(getString(R.string.frame_type_uid))) {
        startAdvertisingUri(advertiseSettings);
      } else if (frameTypeValue.equals(getString(R.string.frame_type_tlm))) {
        startAdvertisingTlm(advertiseSettings);
      } else if (frameTypeValue.equals(getString(R.string.frame_type_url))) {
        startAdvertisingUrl(advertiseSettings);
      } else {
        showToastAndLogError("Illegal frame type value " + frameTypeValue);
      }
    }

    private void startAdvertisingUri(AdvertiseSettings settings) {
      Log.i(TAG, String.format("Start URI frame on ADV %d, Tx power %s, mode %s",
          advNumber, txPower.getSelectedItem(), txMode.getSelectedItem()));
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
      byte[] serviceData;
      try {
        serviceData = buildUidServiceData();
      } catch (IOException e) {
        Log.e(TAG, e.toString());
        Toast.makeText(getContext(), "Failed to build service data", Toast.LENGTH_SHORT).show();
        txSwitch.setChecked(false);
        return;
      }
      AdvertiseData advertiseData = new AdvertiseData.Builder()
          .addServiceData(SERVICE_UUID, serviceData)
          .addServiceUuid(SERVICE_UUID)
          .setIncludeTxPowerLevel(false)
          .setIncludeDeviceName(false)
          .build();
      namespace.setError(null);
      instance.setError(null);
      toggleInputViews(false);
      advertiser.startAdvertising(settings, advertiseData, advertiseCallback);
    }

    // Converts the current Tx power level value to the byte value for that power
    // in dBm at 0 meters.
    //
    // Note that this will vary by device and the values are only roughly accurate.
    // The measurements were taken with a Nexus 6.
    private byte txPowerValueToByte() {
      if (txPowerValue.equals(getString(R.string.tx_power_high))) {
        return (byte) -16;
      } else if (txPowerValue.equals(getString(R.string.tx_power_medium))) {
        return (byte) -26;
      } else if (txPowerValue.equals(getString(R.string.tx_power_low))) {
        return (byte) -35;
      } else {
        return (byte) -59;
      }
    }

    private byte[] buildUidServiceData() throws IOException {
      byte[] namespaceBytes = toByteArray(namespace.getText().toString());
      byte[] instanceBytes = toByteArray(instance.getText().toString());
      ByteArrayOutputStream os = new ByteArrayOutputStream();
      os.write(new byte[]{FRAME_TYPE_UID, txPowerValueToByte()});
      os.write(namespaceBytes);
      os.write(instanceBytes);
      return os.toByteArray();
    }

    // TODO: for the TLM frame to be fully valid the advcnt and seccnt values should increment
    // on every broadcast. The validation app will flag the current state as a bad frame.
    private byte[] buildTlmServiceData(int millivolts, double temp, int advcnt, int seccnt)
        throws IOException {
      ByteBuffer buf = ByteBuffer.allocate(14);
      buf.put(FRAME_TYPE_TLM);
      buf.put((byte) 0x00);
      buf.putShort((short) millivolts);
      // Fixed-point 8.8 format.
      short t = (short) (temp * 256.0f);
      buf.put((byte) (t >> 8));
      buf.put((byte) (t & 0xff));
      buf.putInt(advcnt);
      buf.putInt(seccnt);
      return buf.array();
    }

    private byte[] buildUrlServiceData(String uri) throws IOException {
      ByteArrayOutputStream os = new ByteArrayOutputStream();
      os.write(new byte[]{FRAME_TYPE_URL, txPowerValueToByte()});
      byte[] urlData = UrlUtils.encodeUri(uri);
      if (urlData == null) {
        showToastAndLogError("Could not encode URI " + uri);
      } else {
        os.write(urlData);
      }
      return os.toByteArray();
    }

    private void startAdvertisingTlm(final AdvertiseSettings settings) {
      Log.i(TAG, String.format("Start TLM frame on ADV %d, Tx power %s, mode %s",
          advNumber, txPower.getSelectedItem(), txMode.getSelectedItem()));

      final int millivolts;
      try {
        millivolts = Integer.parseInt(voltage.getText().toString());
      } catch (NumberFormatException e) {
        Log.e(TAG, "Error parsing voltage to int", e);
        voltage.setError("not an integer between 0 and 10000");
        txSwitch.setChecked(false);
        return;
      }

      final double temp;
      try {
        temp = Double.parseDouble(temperature.getText().toString());
      } catch (NumberFormatException e) {
        Log.e(TAG, "Error parsing temperature to float", e);
        temperature.setError("not a float between 0.0 and 60.0");
        txSwitch.setChecked(false);
        return;
      }

      final int advcnt;
      try {
        advcnt = Integer.parseInt(advCount.getText().toString());
      } catch (NumberFormatException e) {
        Log.e(TAG, "Error parsing advCount to int", e);
        advCount.setError("Not an integer");
        txSwitch.setChecked(false);
        return;
      }

      final int seccnt;
      try {
        seccnt = Integer.parseInt(secCount.getText().toString());
      } catch (NumberFormatException e) {
        Log.e(TAG, "Error parsing secCount to int", e);
        secCount.setError("Not an integer");
        txSwitch.setChecked(false);
        return;
      }

      byte[] serviceData;
      try {
        serviceData = buildTlmServiceData(millivolts, temp, advcnt, seccnt);
      } catch (IOException e) {
        Log.e(TAG, e.toString());
        Toast.makeText(getContext(), "Failed to build service data", Toast.LENGTH_SHORT).show();
        txSwitch.setChecked(false);
        return;
      }

      AdvertiseData advertiseData = new AdvertiseData.Builder()
          .addServiceData(SERVICE_UUID, serviceData)
          .addServiceUuid(SERVICE_UUID)
          .setIncludeTxPowerLevel(false)
          .setIncludeDeviceName(false)
          .build();

      voltage.setError(null);
      temperature.setError(null);
      advCount.setError(null);
      secCount.setError(null);
      toggleInputViews(false);

      // If checked we fake the stop/start so we can adjust the broadcast service data. We try to
      // rotate at roughly the same rate as is selected in Tx Mode,
      if (rotateCounters.isChecked()) {
        final long delayMillis = 1000 / txModeValueToHertz();
        Runnable tlmRotation = new Runnable() {
          @Override
          public void run() {
            stopAdvertising();
            byte[] newServiceData = null;
            int adv = Integer.parseInt(advCount.getText().toString()) + 1;
            int sec = Integer.parseInt(secCount.getText().toString()) + 1;
            advCount.setText(Integer.toString(adv));
            secCount.setText(Integer.toString(sec));
            try {
              newServiceData = buildTlmServiceData(millivolts, temp, adv, sec);
            } catch (IOException e) {
              showToastAndLogError(e.getMessage());
            }
            Log.i(TAG, "restarting TLM ADV in loop");
            AdvertiseData newAdvertiseData = new AdvertiseData.Builder()
                .addServiceData(SERVICE_UUID, newServiceData)
                .addServiceUuid(SERVICE_UUID)
                .setIncludeTxPowerLevel(false)
                .setIncludeDeviceName(false)
                .build();
            toggleInputViews(false);
            advertiser.startAdvertising(settings, newAdvertiseData, advertiseCallback);
            handler.postDelayed(this, delayMillis);
          }
        };
        handler.postDelayed(tlmRotation, delayMillis);
      } else {
        advertiser.startAdvertising(settings, advertiseData, advertiseCallback);
      }
    }

    private void startAdvertisingUrl(AdvertiseSettings settings) {
      Log.i(TAG, String.format("Start URL frame %s on ADV %d, Tx power %s, mode %s",
          url.getText(), advNumber, txPower.getSelectedItem(), txMode.getSelectedItem()));

      String urlString = url.getText().toString();
      byte[] uri = UrlUtils.encodeUri(urlString);
      if (uri == null) {
        url.setError("Invalid URL");
        txSwitch.setChecked(false);
        return;
      }

      if (uri.length > 18) {
        url.setError("URL encodes to " + uri.length + " bytes, max is 18");
        txSwitch.setChecked(false);
        return;
      }

      byte[] serviceData;
      try {
        serviceData = buildUrlServiceData(urlString);
      } catch (IOException e) {
        Log.e(TAG, e.toString());
        Toast.makeText(getContext(), "Failed to build service data", Toast.LENGTH_SHORT).show();
        txSwitch.setChecked(false);
        return;
      }

      AdvertiseData advertiseData = new AdvertiseData.Builder()
          .addServiceData(SERVICE_UUID, serviceData)
          .addServiceUuid(SERVICE_UUID)
          .setIncludeTxPowerLevel(false)
          .setIncludeDeviceName(false)
          .build();

      url.setError(null);
      toggleInputViews(false);

      advertiser.startAdvertising(settings, advertiseData, advertiseCallback);
    }

    private void stopAdvertising() {
      Log.i(TAG, String.format("Stop ADV %d", advNumber));
      advertiser.stopAdvertising(advertiseCallback);
      isAdvertising = false;
      toggleInputViews(true);
    }

    private void toggleInputViews(boolean enable) {
      setEnabledViews(enable, frameType, namespace, instance, rndNamespace, rndInstance, txPower,
          txMode, voltage, temperature, advCount, secCount, url, rotateCounters);
    }

    private void showToast(String message) {
      Toast.makeText(getContext(), message, Toast.LENGTH_LONG).show();
    }

    private void showToastAndLogError(String message) {
      showToast(message);
      Log.e(TAG, message);
    }
  }
}
