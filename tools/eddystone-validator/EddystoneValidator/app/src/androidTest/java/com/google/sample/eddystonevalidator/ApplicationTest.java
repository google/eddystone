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

import android.test.AndroidTestCase;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.util.Arrays;

/**
 * <a href="http://d.android.com/tools/testing/testing_android.html">Testing Fundamentals</a>
 */
public class ApplicationTest extends AndroidTestCase {

  private static final String DEVICE_ADDRESS = "00:01:02:03:04:05";
  private static final int INITIAL_RSSI = -100;
  private static final byte FRAME_TYPE_UID = 0x00;
  private static final byte FRAME_TYPE_URL = 0x10;
  private static final byte FRAME_TYPE_TLM = 0x20;
  private static final byte TX_POWER_LOW = (byte) -20;

  private static final byte[] UID_NAMESPACE = {
      0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09  // 10 bytes
  };

  private static final byte[] UID_INSTANCE = {
      (byte) 0xAA, (byte) 0xBB,(byte) 0xCC,(byte) 0xDD,(byte) 0xEE,(byte) 0xFF  // 6 bytes
  };

  private static final byte[] UID_RFU = {
      0x00, 0x00
  };

  private Beacon beacon;

  public ApplicationTest() {
    super();
  }

  @Override
  protected void setUp() throws Exception {
    super.setUp();
    beacon = new Beacon(DEVICE_ADDRESS, INITIAL_RSSI);
  }

  public void testUidValidator_success() throws IOException {
    byte[] serviceData = uidServiceData();
    UidValidator.validate(DEVICE_ADDRESS, serviceData, beacon);

    // Only a UID frame.
    assertTrue(beacon.hasUidFrame);
    assertFalse(beacon.hasTlmFrame);
    assertFalse(beacon.hasUrlFrame);

    // With no errors.
    assertTrue(Arrays.equals(serviceData, beacon.uidServiceData));
    assertTrue(beacon.uidStatus.getErrors().isEmpty());
  }

  // Tx power should be between -100 and 20.
  public void testUidValidator_failsTxPowerBelowMin() throws IOException {
    byte[] serviceData = uidServiceData();
    serviceData[1] = (byte) Constants.MIN_EXPECTED_TX_POWER - 1;
    UidValidator.validate(DEVICE_ADDRESS, serviceData, beacon);

    assertFalse(beacon.uidStatus.getErrors().isEmpty());
    assertEquals(1, beacon.uidStatus.errors.size());
  }

  // Tx power should be between -100 and 20.
  public void testUidValidator_failsTxPowerAboveMax() throws IOException {
    byte[] serviceData = uidServiceData();
    serviceData[1] = (byte) Constants.MAX_EXPECTED_TX_POWER + 1;
    UidValidator.validate(DEVICE_ADDRESS, serviceData, beacon);

    assertFalse(beacon.uidStatus.getErrors().isEmpty());
    assertEquals(1, beacon.uidStatus.errors.size());
  }

  // An ID of all zeroes is certainly "valid" but probably indicates garbage.
  public void testUidValidator_failsZeroedId() {
    byte[] serviceData = new byte[]{
        FRAME_TYPE_UID, TX_POWER_LOW,
        // 10-byte namespace
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        // 6-byte instance
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        // RFU
        0x00, 0x00
    };
    UidValidator.validate(DEVICE_ADDRESS, serviceData, beacon);

    assertFalse(beacon.uidStatus.getErrors().isEmpty());
  }

  // The ID can't change between frames.
  public void testUidValidator_failsIdChanges() throws IOException {
    byte[] serviceData = uidServiceData();
    UidValidator.validate(DEVICE_ADDRESS, serviceData, beacon);

    assertTrue(beacon.uidStatus.getErrors().isEmpty());

    serviceData[2] += 1;
    UidValidator.validate(DEVICE_ADDRESS, serviceData, beacon);

    assertFalse(beacon.uidStatus.getErrors().isEmpty());
    assertEquals(1, beacon.uidStatus.errors.size());
  }

  // RFU bytes should be zeroed.
  public void testUidValidator_failsNonZeroRfu() throws IOException {
    byte[] serviceData = uidServiceData();
    serviceData[19] = 0x01;
    UidValidator.validate(DEVICE_ADDRESS, serviceData, beacon);

    assertFalse(beacon.uidStatus.getErrors().isEmpty());
    assertEquals(1, beacon.uidStatus.errors.size());
  }

  private byte[] uidServiceData() throws IOException {
    ByteArrayOutputStream os = new ByteArrayOutputStream();
    os.write(FRAME_TYPE_UID);
    os.write(TX_POWER_LOW);
    os.write(UID_NAMESPACE);
    os.write(UID_INSTANCE);
    os.write(UID_RFU);
    return os.toByteArray();
  }

}
