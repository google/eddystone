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

import com.google.sample.eddystoneurlconfigvalidator.TestHelper.TestCallback;

import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.le.ScanResult;
import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;

import java.util.ArrayList;
import java.util.ListIterator;
import java.util.concurrent.TimeUnit;

/**
 * The purpose of the TestRunner is to hold the test and dictate what test to run next or to retry a test.
 * It also communicates the tests' state changes to the activity.
 * The TestRunner contains an ArrayList of TestHelpers. Each TestHelper represents a single test to run.
 * A TestHelper consist of a set of actions to be carried out e.g. connect to beacon. A TestHelper also
 * contains the state of the Test e.g. test started.
 * Finally a TestAction represent an interaction with the beacon. The TestAction also holds the whether or not
 * the action was successful.
 */
class TestRunner {

  private static final String TAG = TestRunner.class.getCanonicalName();

  // States for the runner
  //TODO: don't need so many variables. Define states.
  private boolean mStopped;
  private boolean mFailed = false;
  private boolean mRestartedCompleted = false;
  private boolean mRestarted = false;

  private final ArrayList<TestHelper> mUriBeaconTests;
  private TestHelper mLatestTest;
  private final ListIterator<TestHelper> mTestIterator;
  private final DataCallback mDataCallback;
  private final Handler mHandler;
  // To keep the connection to the beacon alive the same gatt object
  // must be passed around. But since gatt is attached to a callback a single super callback
  // is needed for all tests to share.
  private final BluetoothGattCallback superBluetoothScanCallback = new BluetoothGattCallback() {
    @Override
    public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
      Log.d(TAG, "RESTARTED COMPLETED: " + mRestartedCompleted);
      if (!mRestartedCompleted) {
        mLatestTest.mGattCallback.onConnectionStateChange(gatt, status, newState);
      }
    }

    @Override
    public void onServicesDiscovered(BluetoothGatt gatt, int status) {
      mLatestTest.mGattCallback.onServicesDiscovered(gatt, status);
    }

    @Override
    public void onCharacteristicRead(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic,
        int status) {
      mLatestTest.mGattCallback.onCharacteristicRead(gatt, characteristic, status);
    }

    @Override
    public void onCharacteristicWrite(BluetoothGatt gatt,
        BluetoothGattCharacteristic characteristic,
        int status) {
      mLatestTest.mGattCallback.onCharacteristicWrite(gatt, characteristic, status);
    }
  };

  private final TestCallback mTestCallback = new TestCallback() {
    @Override
    public void testStarted() {
      mDataCallback.dataUpdated();
    }

    @Override
    public void testCompleted(final BluetoothDevice bluetoothDevice, final BluetoothGatt gatt) {
      Log.d(TAG, "Test Completed. Failed: " + mLatestTest.isFailed());
      // If the latest test failed set the runner to failed
      if (mLatestTest.isFailed()) {
        mFailed = true;
      }
      // If a test was restarted. TODO: Should probably be a state of the test not the runner.
      if (mRestarted) {
        mRestarted = false;
        mRestartedCompleted = true;
      }
      mDataCallback.dataUpdated();
      // if the test haven't been stopped and a single test retry hasn't been done then run next test
      if (!mStopped && !mRestartedCompleted) {
        // If we just disconnected from the beacon we want to wait a second before the next test
        // Otherwise the connection to the beacon might fail.
        if (gatt == null) {
          mHandler.postDelayed(new Runnable() {
            @Override
            public void run() {
              start(bluetoothDevice, gatt);
            }
          }, TimeUnit.SECONDS.toMillis(1));
        } else {
          start(bluetoothDevice, gatt);
        }
      } else {
        // disconnect if done with tests and gatt connection still on
        if (gatt != null) {
          gatt.disconnect();
        }
        // if tests have been stopped, you don't want to run any more tests
        // TODO: check if this is necessary. It seems that the conditions could be enough for the next test not to run.
        mHandler.removeCallbacksAndMessages(null);
      }
    }

    @Override
    public void waitingForConfigMode() {
      mDataCallback.waitingForConfigMode();
    }

    @Override
    public void connectedToBeacon() {
      mDataCallback.connectedToBeacon();
    }

    @Override
    public void multipleBeacons(ArrayList<ScanResult> scanResults) {
      mDataCallback.multipleConfigModeBeacons(scanResults);
    }

  };

  /**
   * Class to execute the tests
   * @param context To be used by the bluetooth stack
   * @param dataCallback callback to communicate with the activity
   * @param testType type of the tests to run
   * @param optionalImplemented if the optional characteristics have been implemented
   */
  public TestRunner(Context context, DataCallback dataCallback,
      String testType, boolean optionalImplemented) {
    mStopped = false;
    mDataCallback = dataCallback;
    if (SpecEddystoneURLTests.class.getName().equals(testType)){
      mUriBeaconTests = SpecEddystoneURLTests
          .initializeTests(context, mTestCallback, optionalImplemented);
    } else {
      mUriBeaconTests = CoreEddystoneURLTests
          .initializeTests(context, mTestCallback, optionalImplemented);
    }
    mTestIterator = mUriBeaconTests.listIterator();
    mHandler = new Handler(Looper.myLooper());
  }

  /**
   * Start next test
   * @param bluetoothDevice
   * @param gatt
   */
  public void start(BluetoothDevice bluetoothDevice, BluetoothGatt gatt) {
    Log.d(TAG, "Starting tests");
    if (mTestIterator.hasNext() && !mStopped) {
      mLatestTest = mTestIterator.next();
      mLatestTest.run(bluetoothDevice, gatt, superBluetoothScanCallback);
    } else {
      mDataCallback.testsCompleted(mFailed);
    }
  }

  /**
   * @return an ArrayList of the tests that are gonna be executed
   */
  public ArrayList<TestHelper> getUriBeaconTests() {
    return mUriBeaconTests;
  }

  /**
   * Stops the current test and don't execute more tests
   */
  public void stop() {
    mStopped = true;
    mLatestTest.stopTest();
  }

  /**
   * Used for when a test requires the user to specify which beacon to interact with
   * @param which index of the beacon the test should interact with
   */
  public void continueTest(int which) {
    mLatestTest.continueTest(which);
  }

  /**
   * Restart a test
   * @param testPosition index of the test to restart
   */
  public void restart(int testPosition) {
    mRestarted = true;
    mRestartedCompleted = false;
    mLatestTest = mUriBeaconTests.get(testPosition);
    mLatestTest.repeat(superBluetoothScanCallback);
  }

  /**
   * Callback to communicate the state of the tests to the activity
   */
  public interface DataCallback {

    /**
     * A test has changed its state
     */
    public void dataUpdated();

    /**
     * A test is looking for a beacon in config mode
     */
    public void waitingForConfigMode();

    /**
     * A test has successfully connected to a beacon
     */
    public void connectedToBeacon();

    /**
     * All the tests have been completed
     * @param failed true if a test has failed false otherwise
     */
    public void testsCompleted(boolean failed);

    /**
     * The are multiple possible beacons to test, the user should pick one
     * @param scanResults ArrayList of the possible beacons to test
     */
    public void multipleConfigModeBeacons(ArrayList<ScanResult> scanResults);
  }
}
