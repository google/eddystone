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

import java.util.Locale;
import java.util.UUID;
/**
 * Basic wrapper for an error in the GattClient.
 */
public class GattClientException extends Exception {
    // Supported operations.
    public static final int OP_READ = 1;
    public static final int OP_WRITE = 2;
    // Errors beyond the canonical errors in the BluetoothGatt class.
    public static final int ERROR_INTERRUPTED = -1;
    public static final int ERROR_EXECUTION = -2;
    public static final int ERROR_TIMEOUT = -3;
    public static final int ERROR_GATT_DISCONNECTED = -4;
    public final UUID uuid;
    public final int op;
    public final int error;
    public final String customMessage;
    /**
     * Wraps a GATT client error that prevented the GATT operation from completing.
     *
     * @param uuid the UUID of the characteristic that triggered the error.
     * @param op the operation that triggered the error, defined by one of the constants in
     *        the GattOperation class.
     * @param error maps to one of the ERROR_XXX constants in this class. The data field will be
     *        null since the GATT operation did not complete.
     * @param customMessage if not null, provides a custom message to return in getMessage()
     */
    public GattClientException(UUID uuid, int op, int error, String customMessage) {
        this.uuid = uuid;
        this.op = op;
        this.error = error;
        this.customMessage = customMessage;
    }
    /**
     * Wraps a GATT client error that prevented the GATT operation from completing.
     *
     * @param uuid the UUID of the characteristic that triggered the error.
     * @param op the operation that triggered the error, defined by one of the constants in
     *        the GattOperation class.
     * @param error maps to one of the ERROR_XXX constants in this class. The data field will be
     *        null since the GATT operation did not complete.
     */
    public GattClientException(UUID uuid, int op, int error) {
        this(uuid, op, error, null);
    }
    @Override
    public String getMessage() {
        String errStr = error == ERROR_INTERRUPTED
                ? "InterruptionException" : error == ERROR_EXECUTION
                ? "ExecutionException" : "TimeoutException";
        String opStr = op == GattOperation.OP_READ ? "reading" : "writing";
        String message = String.format(Locale.UK,
                "Client error %s when %s characteristic %s",
                errStr,
                opStr,
                GattConstants.getReadableName(uuid));
        if (customMessage != null) {
            message += "Detail: " + customMessage;
        }
        return message;
    }
}