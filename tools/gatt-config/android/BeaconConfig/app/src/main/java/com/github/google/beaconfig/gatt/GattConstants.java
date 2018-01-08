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

package com.github.google.beaconfig.gatt;

import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCharacteristic;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.UUID;
/**
 * Constants for the Eddystone GATT configuration service.
 */
public class GattConstants {
    private GattConstants() {}
    private static final String UNKNOWN_NAME = "Unknown characteristic";
    private static HashMap<String, String> attributes = new HashMap<>();
    public static final String ES_CONFIGURATION_SERVICE_UUID
            = "a3c87500-8ed3-4bdf-8a39-a01bebede295";
    public static final String CHAR_BROADCAST_CAPABILITIES = "a3c87501-8ed3-4bdf-8a39-a01bebede295";
    public static final String CHAR_ACTIVE_SLOT = "a3c87502-8ed3-4bdf-8a39-a01bebede295";
    public static final String CHAR_ADVERTISING_INTERVAL = "a3c87503-8ed3-4bdf-8a39-a01bebede295";
    public static final String CHAR_RADIO_TX_POWER = "a3c87504-8ed3-4bdf-8a39-a01bebede295";
    public static final String CHAR_ADVERTISED_TX_POWER = "a3c87505-8ed3-4bdf-8a39-a01bebede295";
    public static final String CHAR_LOCK_STATE = "a3c87506-8ed3-4bdf-8a39-a01bebede295";
    public static final String CHAR_UNLOCK = "a3c87507-8ed3-4bdf-8a39-a01bebede295";
    public static final String CHAR_PUBLIC_ECDH_KEY = "a3c87508-8ed3-4bdf-8a39-a01bebede295";
    public static final String CHAR_EID_IDENTITY_KEY = "a3c87509-8ed3-4bdf-8a39-a01bebede295";
    public static final String CHAR_ADV_SLOT_DATA = "a3c8750a-8ed3-4bdf-8a39-a01bebede295";
    public static final String CHAR_FACTORY_RESET = "a3c8750b-8ed3-4bdf-8a39-a01bebede295";
    public static final String CHAR_REMAIN_CONNECTABLE = "a3c8750c-8ed3-4bdf-8a39-a01bebede295";
    public static final List<UUID> CHAR_UUIDS = new ArrayList<>();
    static {
        CHAR_UUIDS.add(UUID.fromString(CHAR_BROADCAST_CAPABILITIES));
        CHAR_UUIDS.add(UUID.fromString(CHAR_ACTIVE_SLOT));
        CHAR_UUIDS.add(UUID.fromString(CHAR_ADVERTISING_INTERVAL));
        CHAR_UUIDS.add(UUID.fromString(CHAR_RADIO_TX_POWER));
        CHAR_UUIDS.add(UUID.fromString(CHAR_ADVERTISED_TX_POWER));
        CHAR_UUIDS.add(UUID.fromString(CHAR_LOCK_STATE));
        CHAR_UUIDS.add(UUID.fromString(CHAR_UNLOCK));
        CHAR_UUIDS.add(UUID.fromString(CHAR_PUBLIC_ECDH_KEY));
        CHAR_UUIDS.add(UUID.fromString(CHAR_EID_IDENTITY_KEY));
        CHAR_UUIDS.add(UUID.fromString(CHAR_ADV_SLOT_DATA));
        CHAR_UUIDS.add(UUID.fromString(CHAR_FACTORY_RESET));
        CHAR_UUIDS.add(UUID.fromString(CHAR_REMAIN_CONNECTABLE));
    }
    static {
        attributes.put(ES_CONFIGURATION_SERVICE_UUID, "Eddystone Configuration Service");
        attributes.put(CHAR_BROADCAST_CAPABILITIES, "Broadcast Capabilities");
        attributes.put(CHAR_ACTIVE_SLOT, "Active Slot");
        attributes.put(CHAR_ADVERTISING_INTERVAL, "Advertising Interval");
        attributes.put(CHAR_RADIO_TX_POWER, "Radio Tx Power");
        attributes.put(CHAR_ADVERTISED_TX_POWER, "(Advanced) Advertised Tx Power");
        attributes.put(CHAR_LOCK_STATE, "Lock State");
        attributes.put(CHAR_UNLOCK, "Unlock");
        attributes.put(CHAR_PUBLIC_ECDH_KEY, "Public ECDH Key");
        attributes.put(CHAR_EID_IDENTITY_KEY, "EID Identity Key");
        attributes.put(CHAR_ADV_SLOT_DATA, "ADV Slot Data");
        attributes.put(CHAR_FACTORY_RESET, "(Advanced) Factory reset");
        attributes.put(CHAR_REMAIN_CONNECTABLE, "(Advanced) Remain Connectable");
    }
    public static final byte CAPABILITIES_IS_VARIABLE_ADV_SUPPORTED = 0x01;
    public static final byte CAPABILITIES_IS_VARIABLE_TX_POWER_SUPPORTED = 0x02;
    public static final byte CAPABILITIES_IS_UID_FRAME_SUPPORTED = 0x0001;
    public static final byte CAPABILITIES_IS_URL_FRAME_SUPPORTED = 0x0002;
    public static final byte CAPABILITIES_IS_TLM_FRAME_SUPPORTED = 0x0004;
    public static final byte CAPABILITIES_IS_EID_FRAME_SUPPORTED = 0x0008;
    public static final byte LOCK_STATE_UNKNOWN = Byte.MIN_VALUE;
    public static final byte LOCK_STATE_LOCKED = 0x00;
    public static final byte LOCK_STATE_UNLOCKED = 0x01;
    public static final byte LOCK_STATE_UNLOCKED_AND_AUTOMATIC_RELOCK_DISABLED = 0x02;
    public static String lockStateAsString(byte lockState) {
        switch (lockState) {
            case LOCK_STATE_LOCKED:
                return "locked";
            case LOCK_STATE_UNLOCKED:
                return "unlocked";
            case LOCK_STATE_UNLOCKED_AND_AUTOMATIC_RELOCK_DISABLED:
                return "unlocked, autolock disabled";
            default: return "unknown lock state";
        }
    }
    public static final byte REMAIN_CONNECTABLE_NOT_SUPPORTED = 0x00;
    public static String getReadableName(String uuid) {
        String name = attributes.get(uuid);
        return name == null ? UNKNOWN_NAME : name;
    }
    public static String getReadableName(UUID uuid) {
        return getReadableName(uuid.toString());
    }
    public static String getReadableName(BluetoothGattCharacteristic characteristic) {
        return getReadableName(characteristic.getUuid());
    }
    public static String getStatusString(int status) {
        switch (status) {
            case BluetoothGatt.GATT_SUCCESS: return "GATT_SUCCESS";
            case BluetoothGatt.GATT_READ_NOT_PERMITTED: return "READ_NOT_PERMITTED";
            case BluetoothGatt.GATT_WRITE_NOT_PERMITTED: return "WRITE_NOT_PERMITTED";
            case BluetoothGatt.GATT_INSUFFICIENT_AUTHENTICATION:
                return "INSUFFICIENT_AUTHENTICATION";
            case BluetoothGatt.GATT_REQUEST_NOT_SUPPORTED: return "REQUEST_NOT_SUPPORTED";
            case BluetoothGatt.GATT_INVALID_OFFSET: return "INVALID_OFFSET";
            case BluetoothGatt.GATT_INVALID_ATTRIBUTE_LENGTH: return "INVALID_ATTRIBUTE_LENGTH";
            case BluetoothGatt.GATT_INSUFFICIENT_ENCRYPTION: return "INSUFFICIENT_ENCRYPTION";
            case BluetoothGatt.GATT_CONNECTION_CONGESTED: return "CONNECTION_CONGESTED";
            case BluetoothGatt.GATT_FAILURE: return "GATT_FAILURE";
            // Not first-class errors in Android, but iOS calls them out:
            // https://developer.apple.com/library/mac/documentation/CoreBluetooth/Reference/
            //   CoreBluetooth_Constants/#//apple_ref/c/tdef/CBATTError
            case 1: return "INVALID_HANDLE";
            case 4: return "INVALID_PDU";
            case 8: return "INSUFFICIENT_AUTHORIZATION";
            case 9: return "PREPARE_QUEUE_FULL";
            case 10: return "ATTRIBUTE_NOT_FOUND";
            case 11: return "ATTRIBUTE_NOT_LONG";
            case 12: return "INSUFFICIENT_ENCRYPTION_KEY_SIZE";
            case 14: return "UNLIKELY_ERROR";
            case 16: return "UNSUPPORTED_GROUP_TYPE";
            case 17: return "INSUFFICIENT_RESOURCES";
            default: return "unknown error status: " + status;
        }
    }
}