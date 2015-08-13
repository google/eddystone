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

import static com.google.sample.eddystonevalidator.Constants.TLM_FRAME_TYPE;
import static com.google.sample.eddystonevalidator.TestUtils.DEVICE_ADDRESS;
import static com.google.sample.eddystonevalidator.TestUtils.INITIAL_RSSI;

import android.test.AndroidTestCase;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.util.Arrays;

/**
 * Basic tests for the TlmValidator class.
 */
public class TlmValidatorTest extends AndroidTestCase {

  private static final byte TLM_VERSION = 0x00;
  private static final byte[] TLM_VOLTAGE = {
      0x13, (byte) 0x88  // 5000 millivolts
  };
  private static final byte[] TLM_TEMP = {
      0x15, 0x00  // 21 °C
  };
  private static final byte[] ADV_CNT = {
      0x00, 0x00, 0x00, 0x01
  };
  private static final byte[] SEC_CNT = {
      0x00, 0x00, 0x00, 0x01
  };

  private Beacon beacon;

  public TlmValidatorTest() {
    super();
  }

  @Override
  protected void setUp() throws Exception {
    super.setUp();
    beacon = new Beacon(DEVICE_ADDRESS, INITIAL_RSSI);
  }

  public void testTlmValidator_success() throws IOException {
    byte[] serviceData = tlmServiceData();
    TlmValidator.validate(DEVICE_ADDRESS, serviceData, beacon);

    // Only a TLM frame.
    assertTrue(beacon.hasTlmFrame);
    assertFalse(beacon.hasUidFrame);
    assertFalse(beacon.hasUrlFrame);

    // With no errors.
    assertTrue(Arrays.equals(serviceData, beacon.tlmServiceData));
    assertTrue(beacon.tlmStatus.getErrors().isEmpty());
  }

  public void testTlmValidator_failsBadVersion() throws IOException {
    byte[] serviceData = tlmServiceData();
    serviceData[1] = 0x10;
    TlmValidator.validate(DEVICE_ADDRESS, serviceData, beacon);

    assertTrue(beacon.hasTlmFrame);
    assertFalse(beacon.tlmStatus.getErrors().isEmpty());
  }

  public void testTlmValidator_passesVoltagePowered() throws Exception {
    // Devices that are powered should set the voltage to 0.
    byte[] serviceData = tlmServiceData();
    serviceData[2] = 0x00;
    serviceData[3] = 0x00;
    TlmValidator.validate(DEVICE_ADDRESS, serviceData, beacon);

    assertTrue(beacon.tlmStatus.getErrors().isEmpty());
  }

  public void testTlmValidator_failsVoltageTooLow() throws IOException {
    // 500 mV is certainly "valid" but let's flag that as an problem since
    // if it's not an encoding issue, the beacon is about to die anyway.
    byte[] serviceData = tlmServiceData();
    serviceData[2] = 0x01;
    serviceData[3] = (byte) 0xf3;  // 499 mV
    TlmValidator.validate(DEVICE_ADDRESS, serviceData, beacon);

    assertNotNull(beacon.tlmStatus.errVoltage);
    assertFalse(beacon.tlmStatus.getErrors().isEmpty());
  }

  public void testTlmValidator_failsVoltageTooHigh() throws IOException {
    byte[] serviceData = tlmServiceData();
    serviceData[2] = 0x27;
    serviceData[3] = 0x11;  // 10001 mV
    TlmValidator.validate(DEVICE_ADDRESS, serviceData, beacon);

    assertNotNull(beacon.tlmStatus.errVoltage);
    assertFalse(beacon.tlmStatus.getErrors().isEmpty());
  }

  public void testTlmValidator_failsTempTooLow() throws IOException {
    byte[] serviceData = tlmServiceData();
    // -1 °C. No doubt that's a valid operating temperature for most beacons
    // but are you really validating your beacons when it's this cold?
    serviceData[4] = (byte) 0xff;
    serviceData[5] = 0x00;
    TlmValidator.validate(DEVICE_ADDRESS, serviceData, beacon);

    assertNotNull(beacon.tlmStatus.errTemp);
    assertFalse(beacon.tlmStatus.getErrors().isEmpty());
  }

  public void testTlmValidator_failsTempTooHigh() throws IOException {
    byte[] serviceData = tlmServiceData();
    serviceData[4] = 0x3d;
    serviceData[5] = 0x00;  // 61 °C.
    TlmValidator.validate(DEVICE_ADDRESS, serviceData, beacon);

    assertNotNull(beacon.tlmStatus.errTemp);
    assertFalse(beacon.tlmStatus.getErrors().isEmpty());
  }

  public void testTlmValidator_passesTempNotSupported() throws IOException {
    byte[] serviceData = tlmServiceData();
    serviceData[4] = (byte) 0x80;
    serviceData[5] = 0x00;  // -128 °C.
    TlmValidator.validate(DEVICE_ADDRESS, serviceData, beacon);

    assertNull(beacon.tlmStatus.errTemp);
    assertTrue(beacon.tlmStatus.getErrors().isEmpty());
  }

  public void testTlmValidator_failsPduCountTooLow() throws IOException {
    // If it's just been started then 0 is fine, but otherwise it's a bug.
    byte[] serviceData = tlmServiceData();
    serviceData[6] = serviceData[7] = serviceData[8] = serviceData[9] = 0x00;
    TlmValidator.validate(DEVICE_ADDRESS, serviceData, beacon);

    assertNotNull(beacon.tlmStatus.errPduCnt);
    assertFalse(beacon.tlmStatus.getErrors().isEmpty());
  }

  public void testTlmValidator_failsPduCountTooHigh() throws IOException {
    // Highest expected value is 946080000, or 0x38640900.
    // See Constants.MAX_EXPECTED_PDU_COUNT.
    byte[] serviceData = tlmServiceData();
    serviceData[6] = 0x38;
    serviceData[7] = 0x64;
    serviceData[8] = 0x09;
    serviceData[9] = 0x01;
    TlmValidator.validate(DEVICE_ADDRESS, serviceData, beacon);

    assertNotNull(beacon.tlmStatus.errPduCnt);
    assertFalse(beacon.tlmStatus.getErrors().isEmpty());
  }

  public void testTlmValidator_failsPduCountNotIncreasing() throws IOException {
    byte[] t1 = tlmServiceData();
    TlmValidator.validate(DEVICE_ADDRESS, t1, beacon);

    // Rewind the timestamp so the next frame is stored and compared.
    beacon.timestamp -= (TlmValidator.STORE_NEXT_FRAME_DELTA_MS + 1);

    byte[] t2 = tlmServiceData();
    TlmValidator.validate(DEVICE_ADDRESS, t2, beacon);

    // Advance the boot counter so it's valid.
    t2[13] += 1;

    assertNotNull(beacon.tlmStatus.errPduCnt);
    assertFalse(beacon.tlmStatus.getErrors().isEmpty());
  }

  public void testTlmValidator_failsSecCountTooLow() throws IOException {
    // If it's just been started then 0 is fine, but otherwise it's a bug.
    byte[] serviceData = tlmServiceData();
    serviceData[10] = serviceData[11] = serviceData[12] = serviceData[13] = 0x00;
    TlmValidator.validate(DEVICE_ADDRESS, serviceData, beacon);

    assertNotNull(beacon.tlmStatus.errSecCnt);
    assertFalse(beacon.tlmStatus.getErrors().isEmpty());
  }

  public void testTlmValidator_failsSecCountTooHigh() throws IOException {
    // Highest expected value is 946080000, or 0x38640900 (yes, same as PduCnt).
    // See TlmValidator.MAX_EXPECTED_PDU_COUNT.
    byte[] serviceData = tlmServiceData();
    serviceData[10] = 0x38;
    serviceData[11] = 0x64;
    serviceData[12] = 0x09;
    serviceData[13] = 0x01;
    TlmValidator.validate(DEVICE_ADDRESS, serviceData, beacon);

    assertNotNull(beacon.tlmStatus.errSecCnt);
    assertFalse(beacon.tlmStatus.getErrors().isEmpty());
  }

  public void testTlmValidator_failsSecCountNotIncreasing() throws IOException {
    byte[] t1 = tlmServiceData();
    TlmValidator.validate(DEVICE_ADDRESS, t1, beacon);

    // Rewind the timestamp so the next frame is stored and compared.
    beacon.timestamp -= (TlmValidator.STORE_NEXT_FRAME_DELTA_MS + 1);

    byte[] t2 = tlmServiceData();
    TlmValidator.validate(DEVICE_ADDRESS, t2, beacon);

    // Advance the PDU counter so it's valid.
    t2[9] += 1;

    assertNotNull(beacon.tlmStatus.errSecCnt);
    assertFalse(beacon.tlmStatus.getErrors().isEmpty());
  }

  public void testTlmValidator_succeedsWhenPduAndSecCountIncreasing() throws IOException {
    byte[] t1 = tlmServiceData();
    TlmValidator.validate(DEVICE_ADDRESS, t1, beacon);

    // Rewind the timestamp so the next frame is stored and compared.
    beacon.timestamp -= (TlmValidator.STORE_NEXT_FRAME_DELTA_MS + 1);

    byte[] t2 = tlmServiceData();
    t2[9] += 1;   // Advance PDU count.
    t2[13] += 1;  // Advance SEC count.
    TlmValidator.validate(DEVICE_ADDRESS, t2, beacon);

    assertNull(beacon.tlmStatus.errPduCnt);
    assertNull(beacon.tlmStatus.errSecCnt);
    assertTrue(beacon.tlmStatus.getErrors().isEmpty());
  }

  private byte[] tlmServiceData() throws IOException {
    ByteArrayOutputStream os = new ByteArrayOutputStream();
    os.write(TLM_FRAME_TYPE);
    os.write(TLM_VERSION);
    os.write(TLM_VOLTAGE);
    os.write(TLM_TEMP);
    os.write(ADV_CNT);
    os.write(SEC_CNT);
    return os.toByteArray();
  }

}
