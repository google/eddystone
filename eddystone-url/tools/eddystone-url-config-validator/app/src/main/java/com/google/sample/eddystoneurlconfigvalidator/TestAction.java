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

import android.os.ParcelUuid;

import java.util.UUID;

/**
 * Class that represents a test action. Each test action contains a type, characteristic UUID,
 * expected return code(s), transmitted value (value to write or expected to read), status (whether
 * the action failed) and the reason the action failed. It's used by the test helper to know what
 */
class TestAction {

  // Different types of action
  public final static int CONNECT = 0;
  public final static int WRITE = 1;
  public final static int ASSERT = 2;
  public final static int ASSERT_NOT_EQUALS = 3;
  public final static int MULTIPLE_VALID_RETURN_CODES = 4;
  public final static int DISCONNECT = 5;
  public final static int ADV_TX_POWER = 6;
  public final static int ADV_FLAGS = 7;
  public final static int ADV_URI = 8;
  public final static int ADV_PACKET = 9;
  public final static int LAST = 10;


  public final int actionType;
  public UUID characteristicUuid;
  public ParcelUuid serviceUuid;
  public int expectedReturnCode;
  public byte[] transmittedValue;
  public int[] expectedReturnCodes;
  public boolean failed;
  public String reason;

  /**
   * Simplest action. Intended for Connect, disconnect, adv packet and last actions.
   * @param type of action to run
   */
  public TestAction(int type) {
    //TODO: throw illegal argument for any other action
    this.actionType = type;
  }

  /**
   * Intended for a Write, Assert and Assert Not Equals actions
   * @param actionType type of action
   * @param characteristicUuid characteristic uuid to use in the action
   * @param expectedReturnCode expected return code from the beacon
   * @param transmittedValue value to write to the beacon or value expected to receive
   */
  public TestAction(int actionType, UUID characteristicUuid, int expectedReturnCode,
      byte[] transmittedValue) {
    //TODO: throw illegal argument for any other action
    this.actionType = actionType;
    this.characteristicUuid = characteristicUuid;
    this.expectedReturnCode = expectedReturnCode;
    this.transmittedValue = transmittedValue;
  }

  /**
   * Intended for Assert Adv Flags, Assert Tx Power and Assert Adv Uri
   * @param actionType type of action
   * @param transmittedValue Value to expect from the beacon
   */
  public TestAction(int actionType, byte[] transmittedValue) {
    //TODO: throw illegal argument for any other action
    this.actionType = actionType;
    this.transmittedValue = transmittedValue;
  }

  /**
   * Action to use when there are multiple valid return codes for a specific action,
   * e.g. writting a long URI when the beacon is locked could return invalid lenght or insufficient
   * authorization.
   * @param actionType Type of action
   * @param characteristicUuid Characterisitc to use
   * @param transmittedValue Value to read or write
   * @param expectedReturnCodes expected return codes from the beacon
   */
  public TestAction(int actionType, UUID characteristicUuid, byte[] transmittedValue, int[] expectedReturnCodes) {
    //TODO: throw illegal argument for any other action
    this.actionType = actionType;
    this.characteristicUuid = characteristicUuid;
    this.transmittedValue = transmittedValue;
    this.expectedReturnCodes = expectedReturnCodes;
  }

  public TestAction(int actionType, ParcelUuid serviceUuid) {
    this.actionType = actionType;
    this.serviceUuid = serviceUuid;
  }

  public TestAction(int actionType, ParcelUuid serviceUuid, byte[] transmittedValue) {
    this.actionType = actionType;
    this.serviceUuid = serviceUuid;
    this.transmittedValue = transmittedValue;
  }
}

