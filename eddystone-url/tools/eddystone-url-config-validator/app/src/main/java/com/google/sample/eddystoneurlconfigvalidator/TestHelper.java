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

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothProfile;
import android.bluetooth.le.BluetoothLeScanner;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanFilter;
import android.bluetooth.le.ScanResult;
import android.bluetooth.le.ScanSettings;
import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.os.ParcelUuid;
import android.util.Log;

import org.uribeacon.beacon.UriBeacon;
import org.uribeacon.config.ProtocolV2;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.List;
import java.util.UUID;
import java.util.concurrent.TimeUnit;

public class TestHelper {

  // Constants
  private static final String TAG = TestHelper.class.getCanonicalName();
  private final long SCAN_TIMEOUT = TimeUnit.SECONDS.toMillis(5);

  // BLE Scan callback
  private final ScanCallback mScanCallback = new ScanCallback() {
    @Override
    public void onScanResult(int callbackType, ScanResult result) {
      super.onScanResult(callbackType, result);
      if (mBluetoothDevice != null) {
        Log.d(TAG, "Looking for: " + mBluetoothDevice.getAddress());
      }
      Log.d(TAG, "On scan Result: " + result.getDevice().getAddress());
      // First time we see the beacon
      if(mScanResultSet.add(result.getDevice())) {
        mScanResults.add(result);
      }
    }
  };

  // Gatt Variables
  private BluetoothGatt mGatt;
  private BluetoothGattService mService;
  private BluetoothDevice mBluetoothDevice;
  private final UUID mServiceUuid;
  //TODO: It seems that to maintain connection between tests a better idea would be to extract the Gatt service to the Test Runner
  private BluetoothGattCallback mOutSideGattCallback;
  private final BluetoothAdapter mBluetoothAdapter;
  public final BluetoothGattCallback mGattCallback = new BluetoothGattCallback() {
    @Override
    public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
      super.onConnectionStateChange(gatt, status, newState);
      Log.d(TAG, "Status: " + status + "; New State: " + newState);
      mGatt = gatt;
      if (status == BluetoothGatt.GATT_SUCCESS) {
        if (newState == BluetoothProfile.STATE_CONNECTED) {
          mGatt.discoverServices();
        } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
          if (!failed) {
            mTestActions.remove();
            disconnected = true;
            mGatt = null;
            dispatch();
          }
        }
      } else {
        if (newState == BluetoothProfile.STATE_DISCONNECTED) {
          mGatt = null;
        }
        fail("Failed. Status: " + status + ". New State: " + newState);
      }
    }

    @Override
    public void onServicesDiscovered(BluetoothGatt gatt, int status) {
      super.onServicesDiscovered(gatt, status);
      Log.d(TAG, "On services discovered");
      mGatt = gatt;
      mService = mGatt.getService(mServiceUuid);
      if (mTestActions.peek().actionType == TestAction.CONNECT) {
        mTestActions.remove();
      }
      mTestCallback.connectedToBeacon();
      dispatch();
    }

    @Override
    public void onCharacteristicRead(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic,
        int status) {
      super.onCharacteristicRead(gatt, characteristic, status);
      Log.d(TAG, "On characteristic read");
      mGatt = gatt;
      TestAction readTest = mTestActions.peek();
      byte[] value = characteristic.getValue();
      int actionType = readTest.actionType;
      if (readTest.expectedReturnCode != status) {
        fail("Incorrect status code: " + status + ". Expected: " + readTest.expectedReturnCode);
      } else if (actionType == TestAction.ASSERT_NOT_EQUALS
          && Arrays.equals(readTest.transmittedValue, value)) {
        fail("Values read are the same: " + Arrays.toString(value));
      } else if (actionType == TestAction.ASSERT
          && !Arrays.equals(readTest.transmittedValue, value)) {
        fail("Result not the same. Expected: " + Arrays.toString(readTest.transmittedValue)
            + ". Received: " + Arrays.toString(value));
      } else {
        mTestActions.remove();
        dispatch();
      }
    }


    @Override
    public void onCharacteristicWrite(BluetoothGatt gatt,
        BluetoothGattCharacteristic characteristic,
        int status) {
      super.onCharacteristicWrite(gatt, characteristic, status);
      Log.d(TAG, "On write");
      mGatt = gatt;
      TestAction writeTest = mTestActions.peek();
      int actionType = writeTest.actionType;
      if (actionType == TestAction.WRITE &&  writeTest.expectedReturnCode != status) {
        fail("Incorrect status code: " + status + ". Expected: " + writeTest.expectedReturnCode);
      } else if (actionType == TestAction.MULTIPLE_VALID_RETURN_CODES) {
        boolean match = false;
        for (int expected : writeTest.expectedReturnCodes) {
          if (expected == status) {
            match = true;
          }
        }
        if (!match) {
          fail("Error code didn't match any of the expected error codes.");
        } else {
          mTestActions.remove();
          dispatch();
        }
      } else {
        mTestActions.remove();
        dispatch();
      }
    }

  };

  // Test Information
  private final String mName;
  private final String mReference;
  private final Context mContext;
  private HashSet<BluetoothDevice> mScanResultSet;
  private ArrayList<ScanResult> mScanResults;
  private LinkedList<TestAction> mTestActions;
  private final LinkedList<TestAction> mTestSteps;

  // Test State
  //TODO: there is probably no need to have so many variables. Defining states would be better.
  private boolean started;
  private boolean failed;
  private boolean finished;
  private boolean disconnected;
  private boolean stopped;

  private final TestCallback mTestCallback;
  private final Handler mHandler;


  private TestHelper(
      String name, String reference, Context context, UUID serviceUuid,
      TestCallback testCallback, LinkedList<TestAction> testActions,
      LinkedList<TestAction> testSteps) {
    mName = name;
    mReference = reference;
    mContext = context;
    mServiceUuid = serviceUuid;
    mTestCallback = testCallback;
    mTestActions = testActions;
    mTestSteps = testSteps;
    final BluetoothManager bluetoothManager =
        (BluetoothManager) context.getSystemService(Context.BLUETOOTH_SERVICE);
    mBluetoothAdapter = bluetoothManager.getAdapter();
    mHandler = new Handler(Looper.myLooper());
  }

  /**
   * @return name of the test
   */
  public String getName() {
    return mName;
  }

  /**
   * @return A link to the reference in the spec regarding the correct behavior
   */
  public String getReference() {
    return mReference;
  }

  /**
   * @return a linked list containing the tests
   */
  public LinkedList<TestAction> getTestSteps() {
    return mTestSteps;
  }

  /**
   * @return whether the test failed or not
   */
  public boolean isFailed() {
    return failed;
  }

  /**
   * @return whether or not the test has started
   */
  public boolean isStarted() {
    return started;
  }

  /**
   * Start running the test
   * @param bluetoothDevice device to connect for the test. If no device is present the TestHelper
   * will scan for nearby beacons in config mode
   * @param gatt gatt connection if available. If no gatt connection is available the TestHelper
   * will try to connect to the bluetoothDevice
   * @param outsideCallback Outside callback for the gatt connection
   */
  public void run(BluetoothDevice bluetoothDevice, BluetoothGatt gatt,
      BluetoothGattCallback outsideCallback) {
    Log.d(TAG, "Run Called for: " + getName());
    // Reset Test state
    started = true;
    failed = false;
    finished = false;
    disconnected = false;
    stopped = false;
    // Initialize new list for results
    mScanResults = new ArrayList<>();
    mScanResultSet = new HashSet<>();
    mBluetoothDevice = bluetoothDevice;
    mGatt = gatt;
    if (mGatt != null) {
      mService = gatt.getService(mServiceUuid);
    }
    mOutSideGattCallback = outsideCallback;

    // Notify test started
    mTestCallback.testStarted();
    dispatch();
  }

  private void connectToGatt() {
    Log.d(TAG, "Connecting");
    // If the test just disconnected from the beacon, wait a second
    // otherwise the connection will fail.
    if (disconnected) {
      try {
        disconnected = false;
        TimeUnit.SECONDS.sleep(1);
      } catch (InterruptedException e) {
        e.printStackTrace();
      }
    }
    // Notify runner that the test is currently waiting for a beacon
    mTestCallback.waitingForConfigMode();
    // If there is no device defined scan for a beacon to connect to
    if (mBluetoothDevice == null) {
      scanForConfigBeacon();
    } else {
      mBluetoothDevice.connectGatt(mContext, false, mOutSideGattCallback);
    }
  }

  private void readFromGatt() {
    Log.d(TAG, "reading");
    TestAction readTest = mTestActions.peek();
    BluetoothGattCharacteristic characteristic = mService
        .getCharacteristic(readTest.characteristicUuid);
    mGatt.readCharacteristic(characteristic);
  }

  private void writeToGatt() {
    Log.d(TAG, "Writing");
    TestAction writeTest = mTestActions.peek();
    BluetoothGattCharacteristic characteristic = mService
        .getCharacteristic(writeTest.characteristicUuid);
    // WriteType is WRITE_TYPE_NO_RESPONSE even though the one that requests a response
    // is called WRITE_TYPE_DEFAULT!
    if (characteristic.getWriteType() != BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT) {
      Log.w(TAG, "writeCharacteristic default WriteType is being forced to WRITE_TYPE_DEFAULT");
      characteristic.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT);
    }
    characteristic.setValue(writeTest.transmittedValue);
    mGatt.writeCharacteristic(characteristic);
  }

  private void disconnectFromGatt() {
    Log.d(TAG, "Disconnecting");
    mGatt.disconnect();
  }

  /**
   * This function is in charge of dispatching the next action in the test
   */
  private void dispatch() {
    Log.d(TAG, "Dispatching");
    int actionType = mTestActions.peek().actionType;
    // If the test is stopped and connected to the beacon
    // disconnect from the beacon
    if (stopped) {
      if (mGatt != null) {
        disconnectFromGatt();
      }
    } else if (actionType == TestAction.LAST) {
      Log.d(TAG, "Last");
      finished = true;
      // Tell the test runner that the test has been completed.
      // Pass both the bluetoohDevice and the gatt connection for the next test to use.
      mTestCallback.testCompleted(mBluetoothDevice, mGatt);
    } else if (actionType == TestAction.CONNECT) {
      Log.d(TAG, "Connect");
      connectToGatt();
    } else if (actionType == TestAction.ADV_FLAGS) {
      Log.d(TAG, "ADV FLAGS");
      lookForAdv();
    } else if (actionType == TestAction.ADV_TX_POWER) {
      Log.d(TAG, "ADV TX POWER");
      lookForAdv();
    } else if (actionType == TestAction.ADV_URI) {
      Log.d(TAG, "ADV uri");
      lookForAdv();
    } else if (actionType == TestAction.ADV_PACKET) {
      Log.d(TAG, "ADV packet");
      lookForAdv();
      // There was no previous test or the previous test disconnected from the beacon so need to connect again
    } else if (mGatt == null) {
      Log.d(TAG, "no gatt. connecting");
      connectToGatt();
    } else if (actionType == TestAction.ASSERT || actionType == TestAction.ASSERT_NOT_EQUALS) {
      Log.d(TAG, "Read");
      readFromGatt();
    } else if (actionType == TestAction.WRITE || actionType == TestAction.MULTIPLE_VALID_RETURN_CODES) {
      Log.d(TAG, "Write");
      writeToGatt();
    } else if (actionType == TestAction.DISCONNECT) {
      Log.d(TAG, "Disconenct");
      disconnectFromGatt();
    }
  }

  /**
   * Function to start a scan for config beacon
   */
  private void scanForConfigBeacon() {
    ScanSettings settings = new ScanSettings.Builder()
        .setScanMode(ScanSettings.SCAN_MODE_LOW_LATENCY)
        .build();
    List<ScanFilter> filters = new ArrayList<>();

    ScanFilter filter = new ScanFilter.Builder()
        .setServiceUuid(ProtocolV2.CONFIG_SERVICE_UUID)
        .build();
    filters.add(filter);
    getLeScanner().startScan(filters, settings, mScanCallback);
    Log.d(TAG, "Looking for new config beacons");
    // Stop the test after a while
    mHandler.postDelayed(new Runnable() {
      @Override
      public void run() {
        stopSearchingForBeacons();
        if (mScanResults.size() == 0) {
          fail("No beacons in Config Mode found");
        } else if (mScanResults.size() == 1) {
          continueTest(0);
        } else {
          // Tell the runner that there are multiple beacons available
          mTestCallback.multipleBeacons(mScanResults);
        }
      }
    }, SCAN_TIMEOUT);
  }

  /**
   * Function to scan for broadcasting beacons (not configurable beacons).
   */
  private void lookForAdv() {
    ScanSettings settings = new ScanSettings.Builder()
        .setScanMode(ScanSettings.SCAN_MODE_LOW_LATENCY)
        .build();
    List<ScanFilter> filters = new ArrayList<>();

    ScanFilter filter = new ScanFilter.Builder()
        .setServiceUuid(mTestActions.peek().serviceUuid)
        .build();
    filters.add(filter);
    getLeScanner().startScan(filters, settings, mScanCallback);
    // Stop after a while
    mHandler.postDelayed(new Runnable() {
      @Override
      public void run() {
        stopSearchingForBeacons();
        // If a device is defined check its packet.
        // Otherwise notify runner that there are multiple beacons available to test.
        if (mBluetoothDevice != null) {
          for (ScanResult scanResult : mScanResults) {
            if (scanResult.getDevice().getAddress().equals(mBluetoothDevice.getAddress())) {
              checkPacket(scanResult);
              return;
            }
          }
          fail("Could not find adv packet. Looking for address: " + mBluetoothDevice.getAddress());
        } else if (mScanResults.size() == 1) {
          Log.d(TAG, "Continuing test");
          continueTest(0);
        } else if (mScanResults.size() > 1) {
          Log.d(TAG, "Multiple beacons");
          mTestCallback.multipleBeacons(mScanResults);
        } else {
          fail("Could not find adv packet");
        }
      }
    }, SCAN_TIMEOUT);
  }
  // Stop BLE scanner
  private void stopSearchingForBeacons() {
    getLeScanner().stopScan(mScanCallback);
  }

  /**
   * Function that checks that the packet has the correct value depending on the action.
   * @param result ScanResult from the scanner
   */
  private void checkPacket(ScanResult result) {
    mHandler.removeCallbacksAndMessages(null);
    stopSearchingForBeacons();
    Log.d(TAG, "Found beacon");
    TestAction action = mTestActions.peek();
    if (action.serviceUuid == CoreEddystoneURLTests.TEST_UUID) {
      checkEddystoneURLPacket(result, action);

    } else {
      checkUriBeaconPacket(result, action);
    }
  }

  private void checkUriBeaconPacket(ScanResult result, TestAction action) {
    if (action.actionType == TestAction.ADV_PACKET) {
      if (getAdvPacket(result, action.serviceUuid).length < 2) {
        fail("Invalid Adv Packet");
      } else {
        mTestActions.remove();
        dispatch();
      }
    } else if (action.actionType == TestAction.ADV_FLAGS) {
      byte flags = getFlags(result);
      byte expectedFlags = action.transmittedValue[0];
      if (expectedFlags != flags) {
        fail("Received: " + flags + ". Expected: " + expectedFlags);
      } else {
        mTestActions.remove();
        dispatch();
      }
    } else if (action.actionType == TestAction.ADV_TX_POWER) {
      byte txPowerLevel = getTxPowerLevel(result);
      byte expectedTxPowerLevel = action.transmittedValue[0];
      if (expectedTxPowerLevel != txPowerLevel) {
        fail("Received: " + txPowerLevel + ". Expected: " + expectedTxPowerLevel);
      } else {
        mTestActions.remove();
        dispatch();
      }
    } else if (action.actionType == TestAction.ADV_URI) {
      byte[] uri = getUri(result);
      if (!Arrays.equals(action.transmittedValue, uri)) {
        fail("Received: " + Arrays.toString(uri)
            + ". Expected: " + Arrays.toString(action.transmittedValue));
      } else {
        mTestActions.remove();
        dispatch();
      }
    }
  }

  private void checkEddystoneURLPacket(ScanResult result, TestAction action) {
    Log.d(TAG, "Address: " + result.getDevice().getAddress());
    List<ParcelUuid> uuids = result.getScanRecord().getServiceUuids();
    Log.d(TAG, "Packet: " + Arrays.toString(getAdvPacket(result, action.serviceUuid)));
    if (uuids == null || !uuids.contains(action.serviceUuid)) {
      fail("Eddystone-URL should contain service uuid: " + action.serviceUuid);
      return;
    }
    byte[] advPacket = getAdvPacket(result, action.serviceUuid);
    if (advPacket.length <  1 || advPacket[0] != 0x10) {
      fail("Invalid Adv Packet: " + Arrays.toString(advPacket));
      return;
    }
    if (action.actionType == TestAction.ADV_URI) {
      byte[] url = Arrays.copyOfRange(advPacket, 2, advPacket.length);
      if (!Arrays.equals(action.transmittedValue, url)) {
        fail("Received: " + Arrays.toString(url)
            + ". Expected: " + Arrays.toString(action.transmittedValue));
      }
    } else if (action.actionType == TestAction.ADV_TX_POWER) {
      byte txPowerLevel = advPacket[1];
      byte expectedTxPowerLevel = action.transmittedValue[0];
      if (expectedTxPowerLevel != txPowerLevel) {
        fail("Received: " + txPowerLevel + ". Expected: " + expectedTxPowerLevel);
      }
    }
    mTestActions.remove();
    dispatch();
  }

  private BluetoothLeScanner getLeScanner() {
    return mBluetoothAdapter.getBluetoothLeScanner();
  }

  private byte getFlags(ScanResult result) {
    byte[] serviceData = result.getScanRecord().getServiceData(UriBeacon.URI_SERVICE_UUID);
    return serviceData[0];
  }

  private byte getTxPowerLevel(ScanResult result) {
    byte[] serviceData = result.getScanRecord().getServiceData(UriBeacon.URI_SERVICE_UUID);
    return serviceData[1];
  }

  private byte[] getUri(ScanResult result) {
    byte[] serviceData = result.getScanRecord().getServiceData(UriBeacon.URI_SERVICE_UUID);
    // Get the uri which should start at position 2
    return Arrays.copyOfRange(serviceData, 2, serviceData.length);
  }

  private byte[] getAdvPacket(ScanResult result, ParcelUuid serviceUuid) {
    return result.getScanRecord().getServiceData(serviceUuid);
  }

  /**
   * Called when a test fails
   * @param reason
   */
  private void fail(String reason) {
    Log.d(TAG, "Failing because: " + reason);
    mHandler.removeCallbacksAndMessages(null);
    failed = true;
    finished = true;
    // Update action to failed
    mTestActions.peek().failed = true;
    mTestActions.peek().reason = reason;
    mTestCallback.testCompleted(mBluetoothDevice, mGatt);
  }

  public boolean isFinished() {
    return finished;
  }

  // Stops the current running test
  public void stopTest() {
    stopped = true;
    stopSearchingForBeacons();
    fail("Stopped by user");
  }

  /**
   * Set the new bluetooth device to run the test on
   * @param which
   */
  public void continueTest(int which) {
    //TODO: should probably have a better name
    mBluetoothDevice = mScanResults.get(which).getDevice();
    dispatch();
  }

  /**
   * Function to repeat a test
   * @param outSideGattCallback gatt callback to use
   */
  public void repeat(BluetoothGattCallback outSideGattCallback) {
    Log.d(TAG, "Repeating");
    mTestActions = new LinkedList<>(mTestSteps);
    // The test should use a new gatt connection but use the previous bluetooth device if available
    run(mBluetoothDevice, null, outSideGattCallback);
  }

  /**
   * Interface to communicate the test state to the runner
   */
  public interface TestCallback {

    /**
     * Indicate the test started
     */
    public void testStarted();

    /**
     * Indicate the test completed
     * @param deviceBeingTested device to use for the next test
     * @param gatt gatt connection to use for the next test
     */
    public void testCompleted(BluetoothDevice deviceBeingTested, BluetoothGatt gatt);

    /**
     * Indicate that the test is waiting for a connectable beacon
     */
    public void waitingForConfigMode();

    /**
     * Indicate that the test has connected to a beacon
     */
    public void connectedToBeacon();

    /**
     * Indicate that there are multiple beacons available to test
     * @param scanResults ArrayList containing the scan results of the beacons found
     */
    public void multipleBeacons(ArrayList<ScanResult> scanResults);
  }

  /**
   * Builder class to build a test
   */
  public static class Builder {

    private String mName;
    private String mReference = "";
    private Context mContext;
    private UUID mServiceUuid;
    private TestCallback mTestCallback;
    private final LinkedList<TestAction> mTestActions = new LinkedList<>();

    /**
     * Set the name of the test to show in the test adapter
     * @param name Name of the test
     * @return this builder
     */
    public Builder name(String name) {
      mName = name;
      return this;
    }

    /**
     * Set the reference in the spec that shows the correct behavior
     * @param reference
     * @return
     */
    public Builder reference(String reference) {
      mReference = reference;
      return this;
    }

    /**
     * Sets necessary parameters for the test
     * @param context used for bluetooth stack
     * @param serviceUuid service id of the config service
     * @param testCallback callback to communicate back with the runner
     * @return this builder
     */
    public Builder setUp(Context context, ParcelUuid serviceUuid,
        TestCallback testCallback) {
      //TODO: since all the tests share these parameters it seems unnecessary to have to define them for each test.
      mContext = context;
      mServiceUuid = serviceUuid.getUuid();
      mTestCallback = testCallback;
      return this;
    }

    /**
     * Adds a Connect action to the queue of actions
     * @return this builder
     */
    public Builder connect() {
      mTestActions.add(new TestAction(TestAction.CONNECT));
      return this;
    }

    /**
     * Adds a Disconnect action to the queue of actions
     * @return this builder
     */
    public Builder disconnect() {
      mTestActions.add(new TestAction(TestAction.DISCONNECT));
      return this;
    }

    /**
     * Adds a Assert action to the queue of actions. Reads a value from the beacon and compares it
     * to an expected value.
     * @param characteristicUuid UUID of the characteristic to read
     * @param expectedValue Expected value to read from the beacon
     * @param expectedReturnCode Expected return code from the beacon
     * @return this builder
     */
    public Builder assertEquals(UUID characteristicUuid, byte[] expectedValue,
        int expectedReturnCode) {
      mTestActions.add(
          new TestAction(TestAction.ASSERT, characteristicUuid, expectedReturnCode, expectedValue));
      return this;
    }

    /**
     * Adds a Assert Not Equal action to the queue of actions. Read a value from the beacon and
     * compares it to the not expected value.
     * @param characteristicUuid UUID of the characteristic to read
     * @param expectedValue Value that should not be returned from the beacon
     * @param expectedReturnCode Expected return code from the beacon
     * @return this builder
     */
    public Builder assertNotEquals(UUID characteristicUuid, byte[] expectedValue,
        int expectedReturnCode) {
      mTestActions.add(
          new TestAction(TestAction.ASSERT_NOT_EQUALS, characteristicUuid, expectedReturnCode, expectedValue));
      return this;
    }
    //TODO: The following actions share the same serviceUuid. It should be defined globally.
    /**
     * Adds an Assert Adv Flags action to the queue of actions. Assert the flags in the
     * Advertisement Packet are the expected ones.
     * @param expectedValue Expected value of the flags
     * @return this builder
     */
    public Builder assertAdvFlags(ParcelUuid serviceUuid, byte expectedValue) {
      mTestActions.add(new TestAction(TestAction.ADV_FLAGS, serviceUuid, new byte[]{expectedValue}));
      return this;
    }

    /**
     * Adds an Assert Adv Tx Power action to the queue of actions. Assert the tx power in the
     * Advertisement packet is the expected one.
     * @param expectedValue Expected value of the tx power.
     * @return this builder
     */
    public Builder assertAdvTxPower(ParcelUuid serviceUuid, byte expectedValue) {
      mTestActions.add(new TestAction(TestAction.ADV_TX_POWER, serviceUuid, new byte[]{expectedValue}));
      return this;
    }

    /**
     * Adds an Assert Adv URI actions to the queue of actions. Assert the URI in the Advertisement
     * packet is the expected one.
     * @param expectedValue Expected value of the URI
     * @return this builder
     */
    public Builder assertAdvUri(ParcelUuid serviceUuid, byte[] expectedValue) {
      mTestActions.add(new TestAction(TestAction.ADV_URI, serviceUuid, expectedValue));
      return this;
    }

    /**
     * Adds an Assert Adv Packet action to the queue of actions. Assert that the UriBeacon has a
     * valid advertisement packet according to spec.
     * @return this builder
     */
    public Builder checkAdvPacket(ParcelUuid serviceUuid) {
      mTestActions.add(new TestAction(TestAction.ADV_PACKET, serviceUuid));
      return this;
    }


    /**
     * Adds a Write action to the queue of actions
     * @param characteristicUuid UUID of the characteristic to write
     * @param value Value to write to the UriBeacon
     * @param expectedReturnCode Expected return code from the beacon
     * @return this builder
     */
    public Builder write(UUID characteristicUuid, byte[] value, int expectedReturnCode) {
      mTestActions.add(
          new TestAction(TestAction.WRITE, characteristicUuid, expectedReturnCode, value));
      return this;
    }

    /**
     * Adds a Write action to the queue of actions
     * @param characteristicUuid UUID of the characteristic to write
     * @param value Value to write to the UriBeacon
     * @param expectedReturnCodes Valid possible return codes from the beacon
     * @return this builder
     */
    public Builder write(UUID characteristicUuid, byte[] value,
        int[] expectedReturnCodes) {
      mTestActions.add(
          new TestAction(TestAction.MULTIPLE_VALID_RETURN_CODES, characteristicUuid, value, expectedReturnCodes)
      );
      return this;
    }

    /**
     * Adds a Write and then a Read action for each of the values.
     * @param characteristicUuid UUID of the characteristic to write and read
     * @param values array of values to write and read
     * @return this builder
     */
    public Builder writeAndRead(UUID characteristicUuid, byte[][] values) {
      for (byte[] value : values) {
        writeAndRead(characteristicUuid, value);
      }
      return this;
    }

    /**
     * Adds a Write and then a Read action to the queue of actions.
     * @param characteristicUuid UUID of the characteristic to write and read
     * @param value value to write and expected to read
     * @return this builder
     */
    public Builder writeAndRead(UUID characteristicUuid, byte[] value) {
      mTestActions.add(
          new TestAction(TestAction.WRITE, characteristicUuid, BluetoothGatt.GATT_SUCCESS,
              value));
      mTestActions.add(
          new TestAction(TestAction.ASSERT, characteristicUuid, BluetoothGatt.GATT_SUCCESS,
              value));
      return this;
    }

    /**
     * Adds the actions inside the provided Builder to the current test.
     * @param builder Builder to get the actions from
     * @return this builder
     */
    public Builder insertActions(Builder builder) {
      for (TestAction action : builder.mTestActions) {
        mTestActions.add(action);
      }
      return this;
    }

    /**
     * Builds the test
     * @return this builder
     */
    public TestHelper build() {
      mTestActions.add(new TestAction(TestAction.LAST));
      // Keep a copy of the steps to show in the UI
      LinkedList<TestAction> testSteps = new LinkedList<>(mTestActions);
      return new TestHelper(mName, mReference, mContext, mServiceUuid, mTestCallback,
          mTestActions, testSteps);
    }
  }
}
