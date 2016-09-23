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

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothProfile;
import android.content.Context;
import android.util.Log;

import com.github.google.beaconfig.Constants;
import com.github.google.beaconfig.utils.Utils;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.ArrayList;
import java.util.List;
import java.util.Locale;
import java.util.UUID;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

/**
 * Gatt Client to take care of reading and writing operations using the Gatt service. It makes
 * calls to the GATT service synchronous.
 */
public class GattClient {
    private static final String TAG = GattClient.class.getSimpleName();

    // When asked to read, write, or discover services after connecting, we'll sleep the current
    // thread for this long. This is HIGHLY non-scientific. I have a lose theory that the BT stack
    // on some devices doesn't like being thrashed, and if the beacon responds very quickly to
    // requests we can get the phone into a bad state. This seems to help, but it's really just
    // cargo-cult programming.
    private static final int INTER_OP_SLEEP_MILLIS = 20;

    // The amount of time we allow reads/writes to return. It looks like Android L/M will wait
    // for up to 30 secs for completion, but my experience is that a disconnection happens if
    // the beacon is blocking the call and won't return.
    private static final int READ_WRITE_TIMEOUT_SECS = 20;
    private static final int MAX_EID_SLOT_READ_ATTEMPTS = 5;
    private static final int VALID_EID_SLOT_DATA_LENGTH = 14;
    private BluetoothGattCharacteristic cBroadcastCapabilities;
    private BluetoothGattCharacteristic cActiveSlot;
    private BluetoothGattCharacteristic cAdvertisingInterval;
    private BluetoothGattCharacteristic cRadioTxPower;
    private BluetoothGattCharacteristic cAdvertisedTxPower;
    private BluetoothGattCharacteristic cLockState;
    private BluetoothGattCharacteristic cUnlock;
    private BluetoothGattCharacteristic cPublicEcdhKey;
    private BluetoothGattCharacteristic cEidIdentityKey;
    private BluetoothGattCharacteristic cAdvSlotData;
    private BluetoothGattCharacteristic cFactoryReset;
    private BluetoothGattCharacteristic cRemainConnectable;

    // Callbacks in BluetoothGattCallback are delivered on a thread pool that the system owns.
    // Don't try to use a newSingleThreadExecutor service here -- trying to read the Future<>
    // from the executor will likely be blocked on the callback trying to use the same thread.
    private ExecutorService executor = Executors.newFixedThreadPool(3);
    private final BluetoothGattCallback gattCallback = new BluetoothGattCallback() {

        @Override
        public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
            if (newState == BluetoothProfile.STATE_CONNECTED) {
                listener.onGattConnected();
            } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                gatt.close();
                listener.onGattDisconnected();
            }
        }

        @Override
        public void onServicesDiscovered(BluetoothGatt gatt, int status) {
            executor.execute(new Runnable() {
                @Override
                public void run() {
                    handleServicesDiscovered();
                    listener.onGattServicesDiscovered();
                }
            });
        }

        @Override
        public void onCharacteristicRead(final BluetoothGatt gatt,
                                         final BluetoothGattCharacteristic characteristic,
                                         final int status) {
            executor.execute(new Runnable() {
                @Override
                public void run() {
                    byte[] data = characteristic.getValue();
                    Log.d(TAG, "onCharacteristicRead for "
                            + GattConstants.getReadableName(characteristic)
                            + ", data: " + Utils.toHexString(data));

                    resultQueue.add(new GattResult(status, data));
                }
            });
        }

        @Override
        public void onCharacteristicWrite(final BluetoothGatt gatt,
                                          final BluetoothGattCharacteristic characteristic,
                                          final int status) {
            executor.execute(new Runnable() {
                @Override
                public void run() {
                    byte[] data = characteristic.getValue();
                    Log.d(TAG, "onCharacteristicWrite for "
                            + GattConstants.getReadableName(characteristic)
                            + ", data: " + Utils.toHexString(data));

                    resultQueue.add(new GattResult(status, data));
                }
            });
        }
    };

    private final Context context;
    private final String deviceAddress;
    private final GattListener listener;
    private BluetoothAdapter bluetoothAdapter;
    private BluetoothGatt bluetoothGatt;
    private boolean isEddystoneGattServicePresent = false;
    private List<UUID> missingCharacteristics = new ArrayList<>();

    // Basic blocking queue of size 1. Every operation through this client is synchronous. It's the
    // caller's job to chain together operations based on the success of each function implemented
    // by this client.
    private BlockingQueue<GattResult> resultQueue = new ArrayBlockingQueue<>(1);

    /**
     * The activity handles the connection and disconnection events.
     */
    public interface GattListener {
        void onGattConnected();
        void onGattServicesDiscovered();
        void onGattDisconnected();
    }

    private class GattResult {
        public final int status;
        public final byte[] data;
        public GattResult(int status, byte[] data) {
            this.status = status;
            this.data = data;
        }
    }

    public GattClient(Context context, String deviceAddress, GattListener listener) {
        this.context = context;
        this.deviceAddress = deviceAddress;
        this.listener = listener;
        BluetoothManager bluetoothManager = (BluetoothManager) context
                .getSystemService(Context.BLUETOOTH_SERVICE);
        bluetoothAdapter = bluetoothManager.getAdapter();
    }

    public boolean connect() {
        if (bluetoothAdapter == null) {
            Log.e(TAG, "Unable to obtain a BluetoothAdapter");
            return false;
        }

        BluetoothDevice bluetoothDevice = bluetoothAdapter.getRemoteDevice(deviceAddress);
        if (bluetoothDevice == null) {
            Log.e(TAG, "Device " + deviceAddress + " not found, unable to connect");
            return false;
        }

        bluetoothGatt = bluetoothDevice.connectGatt(context, false, gattCallback);
        Log.d(TAG, "Trying to create a new connection");
        return true;
    }

    public void discoverServices() {
        if (bluetoothGatt != null) {
            Utils.sleep(INTER_OP_SLEEP_MILLIS);
            bluetoothGatt.discoverServices();
        }
    }

    private void handleServicesDiscovered() {
        for (BluetoothGattService service : bluetoothGatt.getServices()) {
            Log.d(TAG, "discovered service " + service.getUuid());

            if (service.getUuid().equals(Constants.EDDYSTONE_CONFIGURATION_UUID.getUuid())) {
                isEddystoneGattServicePresent = true;
                Log.d(TAG, "this is the eddystone service");

                for (BluetoothGattCharacteristic c : service.getCharacteristics()) {
                    String uuid = c.getUuid().toString();
                    if (uuid.equalsIgnoreCase(GattConstants.CHAR_BROADCAST_CAPABILITIES)) {
                        cBroadcastCapabilities = c;
                    } else if (uuid.equalsIgnoreCase(GattConstants.CHAR_ACTIVE_SLOT)) {
                        cActiveSlot = c;
                    } else if (uuid.equalsIgnoreCase(GattConstants.CHAR_ADVERTISING_INTERVAL)) {
                        cAdvertisingInterval = c;
                    } else if (uuid.equalsIgnoreCase(GattConstants.CHAR_RADIO_TX_POWER)) {
                        cRadioTxPower = c;
                    } else if (uuid.equalsIgnoreCase(GattConstants.CHAR_ADVERTISED_TX_POWER)) {
                        cAdvertisedTxPower = c;
                    } else if (uuid.equalsIgnoreCase(GattConstants.CHAR_LOCK_STATE)) {
                        cLockState = c;
                    } else if (uuid.equalsIgnoreCase(GattConstants.CHAR_UNLOCK)) {
                        cUnlock = c;
                    } else if (uuid.equalsIgnoreCase(GattConstants.CHAR_PUBLIC_ECDH_KEY)) {
                        cPublicEcdhKey = c;
                    } else if (uuid.equalsIgnoreCase(GattConstants.CHAR_EID_IDENTITY_KEY)) {
                        cEidIdentityKey = c;
                    } else if (uuid.equalsIgnoreCase(GattConstants.CHAR_ADV_SLOT_DATA)) {
                        cAdvSlotData = c;
                    } else if (uuid.equalsIgnoreCase(GattConstants.CHAR_FACTORY_RESET)) {
                        cFactoryReset = c;
                    } else if (uuid.equalsIgnoreCase(GattConstants.CHAR_REMAIN_CONNECTABLE)) {
                        cRemainConnectable = c;
                    }
                }

                Log.d(TAG, cUnlock.toString());
                Log.d(TAG, "Checking characteristics");

                for (UUID uuid : GattConstants.CHAR_UUIDS) {
                    if (service.getCharacteristic(uuid) == null) {
                        missingCharacteristics.add(uuid);
                        Log.d(TAG, GattConstants.getReadableName(uuid) + " missing");
                    }
                }
                break;
            }
        }
    }

    public void disconnect() {
        if (bluetoothGatt == null) {
            return;
        }
        bluetoothGatt.close();
        bluetoothGatt.disconnect();
        Log.d(TAG, "Just disconnected.");
        bluetoothGatt = null;
    }

    public boolean isEddystoneGattServicePresent() {
        return isEddystoneGattServicePresent;
    }

    public List<UUID> getMissingCharacteristics() {
        return missingCharacteristics;
    }

    private class GattCallable implements Callable<GattResult> {
        @Override
        public GattResult call() throws Exception {
            try {
                return resultQueue.take();
            } catch (InterruptedException e) {
                Log.d(TAG, "InterruptedException, returning error result");
                return new GattResult(-1, null);
            }
        }
    }

    // Read and writes of raw data for each characteristic.
    public byte[] readBroadcastCapabilities() throws GattClientException, GattOperationException {
        return read(cBroadcastCapabilities);
    }

    public byte[] readActiveSlot() throws GattClientException, GattOperationException {
        return read(cActiveSlot);
    }

    public byte[] writeActiveSlot(int slot) throws GattClientException, GattOperationException {
        byte[] data = new byte[] {(byte) slot};
        return write(cActiveSlot, data);
    }

    public byte[] readAdvertisingInterval() throws GattClientException, GattOperationException {
        return read(cAdvertisingInterval);
    }

    public byte[] writeAdvertisingInterval(int millis)
            throws GattClientException, GattOperationException {
        byte[] data = Utils.toTwoByteArray(millis);
        return write(cAdvertisingInterval, data);
    }

    public byte[] readRadioTxPower() throws GattClientException, GattOperationException {
        return read(cRadioTxPower);
    }

    public byte[] writeRadioTxPower(int dbm) throws GattClientException, GattOperationException {
        byte[] data = new byte[] {(byte) dbm};
        return write(cRadioTxPower, data);
    }

    public byte[] readAdvertisedTxPower() throws GattClientException, GattOperationException {
        return read(cAdvertisedTxPower);
    }

    public byte[] writeAdvertisedTxPower(int dbm)
            throws GattClientException, GattOperationException {
        byte[] data = new byte[] {(byte) dbm};
        return write(cAdvertisedTxPower, data);
    }

    public byte[] readLockState() throws GattClientException, GattOperationException {
        return read(cLockState);
    }

    public byte[] writeLockState(byte[] data) throws GattClientException, GattOperationException {
        return write(cLockState, data);
    }

    public byte[] readUnlock() throws GattClientException, GattOperationException {
        return read(cUnlock);
    }

    public byte[] writeUnlock(byte[] data) throws GattClientException, GattOperationException {
        return write(cUnlock, data);
    }

    public byte[] readPublicEcdhKey() throws GattClientException, GattOperationException {
        return read(cPublicEcdhKey);
    }

    public byte[] readEidIdentityKey() throws GattClientException, GattOperationException {
        return read(cEidIdentityKey);
    }

    public byte[] writeEidIdentityKey(byte[] data)
            throws GattClientException, GattOperationException {
        return write(cEidIdentityKey, data);
    }

    public byte[] readAdvSlotData() throws GattClientException, GattOperationException {
        byte[] result = read(cAdvSlotData);
        return result;
    }

    public byte[] writeAdvSlotData(byte[] data) throws GattClientException, GattOperationException {
        return write(cAdvSlotData, data);
    }

    public byte[] writeFactoryReset(byte[] data)
            throws GattClientException, GattOperationException {
        return write(cFactoryReset, data);
    }

    public byte[] readRemainConnectable() throws GattClientException, GattOperationException {
        return read(cRemainConnectable);
    }

    public byte[] writeRemainConnectable(byte[] data)
            throws GattClientException, GattOperationException {
        return write(cRemainConnectable, data);
    }

    private byte[] read(BluetoothGattCharacteristic characteristic)
            throws GattClientException, GattOperationException {
        Utils.sleep(INTER_OP_SLEEP_MILLIS);
        Log.d(TAG, "reading " + GattConstants.getReadableName(characteristic));
        if (bluetoothGatt != null) {
            bluetoothGatt.readCharacteristic(characteristic);
            return getResult(characteristic, GattOperation.OP_READ);
        } else {
            throw new GattClientException(characteristic.getUuid(), GattOperation.OP_READ,
                    GattClientException.ERROR_GATT_DISCONNECTED,
                    "Bluetooth Gatt disconnected unexpectedly");
        }
    }

    private byte[] write(BluetoothGattCharacteristic characteristic, byte[] data)
            throws GattClientException, GattOperationException {
        Utils.sleep(INTER_OP_SLEEP_MILLIS);
        Log.d(TAG, "writing " + GattConstants.getReadableName(characteristic)
                + ", data: " + Utils.toHexString(data));
        characteristic.setValue(data);

        if (bluetoothGatt != null) {
            bluetoothGatt.writeCharacteristic(characteristic);
            return getResult(characteristic, GattOperation.OP_WRITE);
        } else {
            throw new GattClientException(characteristic.getUuid(), GattOperation.OP_WRITE,
                    GattClientException.ERROR_GATT_DISCONNECTED,
                    "Bluetooth Gatt disconnected unexpectedly");
        }
    }

    private byte[] getResult(BluetoothGattCharacteristic c, int op)
            throws GattClientException, GattOperationException {
        Future<GattResult> future = executor.submit(new GattCallable());
        try {
            GattResult result = future.get(READ_WRITE_TIMEOUT_SECS, TimeUnit.SECONDS);
            Log.d(TAG, "future returned status " + result.status
                    + ", data " + Utils.toHexString(result.data));

            if (result.status != BluetoothGatt.GATT_SUCCESS) {
                throw new GattOperationException(c.getUuid(), op, result.status, result.data);
            }

            return result.data;
        } catch (InterruptedException e) {
            Log.d(TAG, "InterruptedException in future.get, returning error result");
            throw new GattClientException(c.getUuid(), op,
                    GattOperationException.ERROR_INTERRUPTED);
        } catch (ExecutionException e) {
            Log.d(TAG, "ExecutionException in future.get, returning error result");
            throw new GattClientException(c.getUuid(), op, GattOperationException.ERROR_EXECUTION);
        } catch (TimeoutException e) {
            Log.d(TAG, "TimeoutException in future.get, returning error result");
            throw new GattClientException(c.getUuid(), op, GattOperationException.ERROR_TIMEOUT);
        }
    }

    /**
     * Convenience methods.
     * Simple struct representing a parsed slot data byte array for an EID slot.
     */
    public class EidSlotData {
        public final byte exponent;
        public final long clock;
        public final byte[] eid;
        public EidSlotData(final byte exponent, final long clock, final byte[] eid) {
            this.exponent = exponent;
            this.clock = clock;
            this.eid = eid;
        }

        @Override
        public String toString() {
            return String.format(Locale.UK, "exponent=%d, clock=%d, eid=%s",
                    (int) exponent, (int) clock, Utils.toHexString(eid));
        }
    }

    /**
     * This is a special-case of readAdvSlotData(). Some devices are slow about their cryptographic
     * computations and can take a while to return the EID data. Instead of blocking on the read
     * call, they either return a zeroed EID value, or an incorrect length. We deal with these cases
     * here by checking and re-reading after a pause, giving up if necessary.
     */
    public EidSlotData readEidSlotData() throws GattClientException, GattOperationException {
        int attempt = 1;
        Log.d(TAG, "readEidSlotData attempt " + attempt);
        byte[] data = readAdvSlotData();
        while (data.length != VALID_EID_SLOT_DATA_LENGTH || isEidValueZeroed(data)) {
            if (attempt == MAX_EID_SLOT_READ_ATTEMPTS) {
                Log.d(TAG, "readEidSlotData max attempts of "
                        + MAX_EID_SLOT_READ_ATTEMPTS
                        + " reached, giving up");
                throw new GattClientException(cAdvSlotData.getUuid(), GattClientException.OP_READ,
                        -1, "Repeated reads from slot data characteristic returned invalid data. "
                        + "Expected 14 bytes, last value read was " + Utils.toHexString(data));
            }
            attempt++;
            Log.d(TAG, "readEidSlotData, data is wrong length or zeroed, "
                    + "retrying after 2 secs, attempt " + attempt);
            Utils.sleep(2000);
            data = readAdvSlotData();
        }

        ByteBuffer buf = ByteBuffer.wrap(data).order(ByteOrder.BIG_ENDIAN);
        byte frameType = buf.get();

        if (frameType != Constants.EID_FRAME_TYPE) {
            throw new GattClientException(cAdvSlotData.getUuid(), GattClientException.OP_READ, -1,
                    "Reading EID slot returned invalid frame type. "
                            + "Expected 0x30, got " + frameType);
        }
        byte exponent = buf.get();

        // We want an unsigned int, so get the long and mask it.
        long clock = (long) buf.getInt() & 0xffffffffL;
        byte[] eidValue = new byte[8];
        buf.get(eidValue);
        return new EidSlotData(exponent, clock, eidValue);
    }

    // Caller guarantees length is 14.
    private boolean isEidValueZeroed(final byte[] eidSlotData) {
        byte[] value = new byte[8];
        System.arraycopy(eidSlotData, 6, value, 0, 8);
        return Utils.isZeroed(value);
    }

    // Convenience methods.
    public void performFactoryReset() throws GattClientException, GattOperationException {
        writeFactoryReset(new byte[]{0x0B});
    }

    public boolean unlock(byte[] unlockCode) {
        Log.d(TAG, "Starting unlocking...");
        try {
            byte[] readUnlockResult = readUnlock();
            byte[] encrypted = Utils.aes128Encrypt(readUnlockResult, unlockCode);
            if (encrypted == null) {
                return false;
            }

            writeUnlock(encrypted);
            byte[] lockState = readLockState();
            return lockState != null
                    && lockState.length > 0
                    && lockState[0] == GattConstants.LOCK_STATE_UNLOCKED;
        } catch (GattClientException | GattOperationException e) {
            Log.e(TAG, "Failed to unlock beacon", e);
            return false;
        }
    }
}