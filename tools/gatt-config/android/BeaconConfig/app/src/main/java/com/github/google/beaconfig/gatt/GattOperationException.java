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

import com.github.google.beaconfig.utils.Utils;

import java.util.Locale;
import java.util.UUID;

/**
 * Basic wrapper for an error during a GATT operation.
 */
public class GattOperationException extends Exception {
    // Errors beyond the canonical errors in the BluetoothGatt class.
    public static final int ERROR_INTERRUPTED = -1;
    public static final int ERROR_EXECUTION = -2;
    public static final int ERROR_TIMEOUT = -3;
    public final UUID uuid;
    public final int op;
    public final int status;
    public final byte[] data;
    /**
     * Wraps a GATT error from a completed operation that returned a canonical error code.
     * @param uuid the UUID of the characteristic that triggered the error.
     * @param op the operation that triggered the error, defined by one of the constants in
     *        the GattOperation class.
     * @param status maps to one of the constants defined in the platform's BluetoothGatt class.
     * @param data the data returned by the operation.
     */
    public GattOperationException(UUID uuid, int op, int status, byte[] data) {
        this.uuid = uuid;
        this.op = op;
        this.status = status;
        this.data = data;
    }
    @Override
    public String getMessage() {
        String opStr = op == GattOperation.OP_READ ? "reading" : "writing";
        String dataStr
                = data == null ? "null" : data.length == 0 ? "[empty]" : Utils.toHexString(data);
        String message = String.format(Locale.UK,
                "GATT Error %s when %s characteristic %s, data: %s",
                GattConstants.getStatusString(status),
                opStr,
                GattConstants.getReadableName(uuid),
                dataStr);
        return message;
    }
}
