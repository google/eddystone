package com.github.google.beaconfig.utils;

import android.util.Log;

import java.util.Arrays;

import com.github.google.beaconfig.Constants;

/**
 * This class is composed of static methods to be used to either parse information from given
 * slotData or to build slotData from given strings and integers.
 */
public class SlotDataManager {
    private static final String TAG = SlotDataManager.class.getSimpleName();

    public static byte getFrameTypeFromSlotData(byte[] slotData) {
        return slotData[0];
    }

    /**
     * @param slotData has to be the slot data read from a UID frame of a beacon
     * @return namespace String parsed from the slot data byte array
     */
    public static String getNamespaceFromSlotData(byte[] slotData) {
        if (getFrameTypeFromSlotData(slotData) != Constants.UID_FRAME_TYPE) {
            Log.d(TAG, "Attempt to get namespace from a non-uid frame failed.");
            return "";
        }
        byte[] namespaceBytes = Arrays.copyOfRange(slotData, 2, 12);
        return Utils.byteArrayToHexString(namespaceBytes);
    }

    /**@param writeSlotData has to be the byte array to be written to a beacon slot in order to
     *                      configure it as a UID slot
     * @return namespace String parsed from the write byte array
     */
    public static String getNamespaceFromWriteByteArray(byte[] writeSlotData) {
        if (getFrameTypeFromSlotData(writeSlotData) != Constants.UID_FRAME_TYPE) {
            Log.d(TAG, "Attempt to get namespace from a non-uid frame failed.");
            return "";
        }
        byte[] namespaceBytes = Arrays.copyOfRange(writeSlotData, 1, 11);
        return Utils.byteArrayToHexString(namespaceBytes);
    }

    /**
     * @param slotData has to be the slot data of a UID configured slot of a beacon
     * @return instance String parsed from the slot data byte array
     */
    public static String getInstanceFromSlotData(byte[] slotData) {
        if (getFrameTypeFromSlotData(slotData) != Constants.UID_FRAME_TYPE) {
            Log.d(TAG, "Attempt to get instance from a non-uid frame failed.");
            return "";
        }
        byte[] instanceBytes = Arrays.copyOfRange(slotData, 12, 18);
        return Utils.byteArrayToHexString(instanceBytes);
    }

    /**
     * @param writeSlotData has to be the byte array to be written to a beacon slot in order to
     *                      configure it as a UID slot
     * @return namespace String parsed from the write byte array
     */
    public static String getInstanceFromWriteByteArray(byte[] writeSlotData) {
        if (getFrameTypeFromSlotData(writeSlotData) != Constants.UID_FRAME_TYPE) {
            Log.d(TAG, "Attempt to get instance from a non-uid frame failed.");
            return "";
        }
        byte[] instanceBytes = Arrays.copyOfRange(writeSlotData, 11, 17);
        return Utils.byteArrayToHexString(instanceBytes);
    }

    /**
     * @param slotData has to be the slot data of a URL configured slot of a beacon
     * @return url as a String parsed from the slot data byte array
     */
    public static String getUrlFromSlotData(byte[] slotData) {
        if (getFrameTypeFromSlotData(slotData) != Constants.URL_FRAME_TYPE) {
            Log.d(TAG, "Attempt to get URL from a non-url frame failed.");
            return "";
        }
        return URLEncodeDecoder.decodeUrlFromUrlBytes(
                Arrays.copyOfRange(slotData, 2, slotData.length));
    }

    /**
     * @param writeSlotData has to be the byte array to be written to a beacon slot in order to
     *                      configure it as a URL slot
     * @return url as a String parsed from the write byte array
     */
    public static String getUrlFromWriteByteArray(byte[] writeSlotData) {
        if (getFrameTypeFromSlotData(writeSlotData) != Constants.URL_FRAME_TYPE) {
            Log.d(TAG, "Attempt to get URL from a non-url frame failed.");
            return "";
        }
        return URLEncodeDecoder.decodeUrlFromUrlBytes(
                Arrays.copyOfRange(writeSlotData, 1, writeSlotData.length));
    }

    /**
     * @param slotData has to be the slot data of a TLM configured slot of a beacon
     * @return version parsed from the slot data byte array
     */
    public static byte getVersionFromSlotData(byte[] slotData) {
        if (getFrameTypeFromSlotData(slotData) != Constants.TLM_FRAME_TYPE) {
            Log.d(TAG, "Attempt to get version from a non-tlm frame failed.");
            return 0;
        }
        return slotData[1];
    }

    /**
     * @param slotData has to be the slot data of a TLM configured slot of a beacon
     * @return voltage parsed from the slot data byte array
     */
    public static short getVoltageFromSlotData(byte[] slotData) {
        if (getFrameTypeFromSlotData(slotData) != Constants.TLM_FRAME_TYPE) {
            Log.d(TAG, "Attempt to get voltage from a non-tlm frame failed.");
            return 0;
        }
        return (short) (((slotData[2] & 0xff) << 8) | (slotData[3] & 0xff));
    }

    /**
     * @param slotData has to be the slot data of a TLM configured slot of a beacon
     * @return temperature parsed from the slot data byte array
     */
    public static float getTemperatureFromSlotData(byte[] slotData) {
        if (getFrameTypeFromSlotData(slotData) != Constants.TLM_FRAME_TYPE) {
            Log.d(TAG, "Attempt to get temperature from a non-tlm frame failed.");
            return 0;
        }
        byte tempIntegral = slotData[4];
        int tempFractional = (slotData[5] & 0xff);
        return tempIntegral + (tempFractional / 256.0f);
    }

    /**
     * @param slotData has to be the slot data of a TLM configured slot of a beacon
     * @return advertising PDU count parsed from the slot data byte array
     */
    public static int getAdvertisingPDUCountFromSlotData(byte[] slotData) {
        if (getFrameTypeFromSlotData(slotData) != Constants.TLM_FRAME_TYPE) {
            Log.d(TAG, "Attempt to get advertising PDU count from a non-tlm frame failed.");
            return 0;
        }
        return ((slotData[6] & 0xff) << 24)
                | ((slotData[7] & 0xff) << 16)
                | ((slotData[8] & 0xff) << 8)
                | (slotData[9] & 0xff);
    }

    /**
     * @param slotData TLM slot data read from beacon
     * @return time since on in milliseconds
     */
    public static int getTimeSinceOnFromSlotData(byte[] slotData) {
        if (getFrameTypeFromSlotData(slotData) != Constants.TLM_FRAME_TYPE) {
            Log.d(TAG, "Attempt to get time since on from a non-tlm frame failed.");
            return 0;
        }
        return ((slotData[10] & 0xff) << 24)
                | ((slotData[11] & 0xff) << 16)
                | ((slotData[12] & 0xff) << 8)
                | (slotData[13] & 0xff);
    }

    public static String getEphemeralIdFromSlotData(byte[] slotData) {
        if (getFrameTypeFromSlotData(slotData) != Constants.EID_FRAME_TYPE) {
            Log.d(TAG, "Attempt to get ephemeral id from a non-eid frame failed.");
            return "";
        }
        return Utils.byteArrayToHexString(Arrays.copyOfRange(slotData, 2, 10));
    }


    public static byte[] buildNewUidSlotData(String namespace, String instance) {
        byte[] newUidSlotData = new byte[17];

        newUidSlotData[0] = Constants.UID_FRAME_TYPE;

        if (namespace != null && namespace.length() == 20) {
            byte[] namespaceBytes = Utils.toByteArray(namespace);
            newUidSlotData = Utils.rewriteBytes(newUidSlotData, 1, 10, namespaceBytes);
        }

        if (instance != null && instance.length() == 12) {
            byte[] instanceBytes = Utils.toByteArray(instance);
            newUidSlotData = Utils.rewriteBytes(newUidSlotData, 11, 6, instanceBytes);
        }

        return newUidSlotData;
    }

    public static byte[] buildNewUrlSlotData(String newUrl) {
        byte[] urlBytes = URLEncodeDecoder.encodeUri(newUrl);
        byte[] newSlotData = new byte[urlBytes.length + 1];
        newSlotData[0] = Constants.URL_FRAME_TYPE;
        newSlotData = Utils.rewriteBytes(newSlotData, 1, urlBytes.length, urlBytes);
        return newSlotData;
    }

    public static byte[] buildNewTlmSlotData() {
        byte[] newSlotData = new byte[1];
        newSlotData[0] = Constants.TLM_FRAME_TYPE;
        return newSlotData;
    }

    public static byte[] buildNewEidSlotData() {
        return new byte[0];
    }
}
