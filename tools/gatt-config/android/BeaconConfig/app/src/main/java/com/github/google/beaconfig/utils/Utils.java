package com.github.google.beaconfig.utils;

import android.app.Activity;
import android.content.Context;
import android.support.annotation.NonNull;
import android.util.Log;
import android.view.View;
import android.view.inputmethod.InputMethodManager;

import com.github.google.beaconfig.BeaconConfigActivity;
import com.github.google.beaconfig.Constants;
import com.github.google.beaconfig.R;

import java.security.InvalidKeyException;
import java.security.NoSuchAlgorithmException;
import java.util.ArrayList;
import java.util.concurrent.TimeUnit;

import javax.crypto.BadPaddingException;
import javax.crypto.Cipher;
import javax.crypto.IllegalBlockSizeException;
import javax.crypto.NoSuchPaddingException;
import javax.crypto.spec.SecretKeySpec;

/**
 * Helper functions throughout the program.
 */

public class Utils {
    private static final String TAG = BeaconConfigActivity.class.getSimpleName();

    private static final char[] HEX = "0123456789ABCDEF".toCharArray();

    public static String byteArrayToHexString(byte[] bytes) {
        if (bytes == null) {
            Log.d(TAG, "Error. byteArrayToHexString() - input null");
            return "";
        }
        char[] charArray = new char[bytes.length * 2];
        int charIndex = 0;
        for (byte currentByte : bytes) {
            charArray[charIndex++] = HEX[(currentByte >> 4) & 0x0f];
            charArray[charIndex++] = HEX[currentByte & 0x0f];
        }
        return new String(charArray);
    }

    public static byte[] toByteArray(String s) {
        // s guaranteed valid by caller.
        int len = s.length();
        byte[] bytes = new byte[len / 2];
        for (int i = 0; i < len; i += 2) {
            bytes[i / 2] = (byte) ((Character.digit(s.charAt(i), 16) << 4)
                    + Character.digit(s.charAt(i + 1), 16));
        }
        return bytes;
    }

    public static byte[] aes128Encrypt(byte[] challenge, byte[] unlockCode) {
        Cipher cipher;
        try {
            cipher = Cipher.getInstance("AES/ECB/NoPadding");
        } catch (NoSuchAlgorithmException | NoSuchPaddingException e) {
            Log.e(TAG, "Error constructing cipher instance", e);
            return null;
        }
        try {
            SecretKeySpec keySpec = new SecretKeySpec(unlockCode, "AES");
            cipher.init(Cipher.ENCRYPT_MODE, keySpec);
        } catch (InvalidKeyException e) {
            Log.e(TAG, "Error initializing cipher instance", e);
            return null;
        }
        byte[] ret;
        try {
            ret = cipher.doFinal(challenge);
        } catch (IllegalBlockSizeException | BadPaddingException e) {
            Log.e(TAG, "Error executing cipher", e);
            return null;
        }
        return ret;
    }

    public static String toHexString(byte[] bytes) {
        if (bytes == null || bytes.length == 0) {
            return "";
        }
        char[] chars = new char[bytes.length * 2];
        for (int i = 0; i < bytes.length; i++) {
            int c = bytes[i] & 0xFF;
            chars[i * 2] = HEX[c >>> 4];
            chars[i * 2 + 1] = HEX[c & 0x0F];
        }
        return new String(chars).toLowerCase();
    }

    public static byte[] toTwoByteArray(int i) {
        byte[] out = new byte[2];
        out[0] = (byte) ((i >> 8) & 0xFF);
        out[1] = (byte) (i & 0xFF);
        return out;
    }

    public static void sleep(int millis) {
        try {
            Thread.sleep(millis);
        } catch (InterruptedException e) {
            Log.e(TAG, "sleep interruption", e);
        }
    }

    public static boolean isZeroed(byte[] bytes) {
        for (byte b : bytes) {
            if (b != 0x00) {
                return false;
            }
        }
        return true;
    }

    // Caller guarantees data is two bytes.
    public static int toInt(byte[] data) {
        return ((data[0] & 0xff) << 8 | data[1] & 0xff);
    }

    public static boolean isHex(char c) {
        return ((c >= '0') && (c <= '9'))
                || ((c >= 'a') && (c <= 'f'))
                || ((c >= 'A') && (c <= 'F'));
    }

    public static byte findMaxValue(byte[] array) {
        byte max = array[0];

        for (byte a : array) {
            if (a > max) {
                max = a;
            }
        }
        return max;
    }

    public static byte findMinValue(byte[] array) {
        byte max = array[0];

        for (byte a : array) {
            if (a < max) {
                max = a;
            }
        }
        return max;
    }

    public static int findValueClosestTo(int value, byte[] allowedValues) {
        int smallestDistance = Math.abs(allowedValues[0] - value);
        int index = 0;
        for (int i = 1; i < allowedValues.length; i++) {
            int currDistance = Math.abs(allowedValues[i] - value);
            if (currDistance < smallestDistance) {
                index = i;
                smallestDistance = currDistance;
            }
        }
        return allowedValues[index];
    }

    public static byte[] rewriteBytes(byte[] original, int beginIndex,
                                      int howMany, byte[] replaceWith) {
        for (int i = beginIndex; i < beginIndex + howMany; i++) {
            original[i] = replaceWith[i - beginIndex];
        }

        return original;
    }

    public static String getStringFromFrameType(byte frameType) {
        switch (frameType) {
            case Constants.UID_FRAME_TYPE:
                return Constants.UID;

            case Constants.URL_FRAME_TYPE:
                return Constants.URL;

            case Constants.TLM_FRAME_TYPE:
                return Constants.TLM;

            case Constants.EID_FRAME_TYPE:
                return Constants.EID;

            default:
                return "--";
        }
    }

    public static byte getFrameTypeFromString(String frameString) {
        switch (frameString) {
            case Constants.UID:
                return Constants.UID_FRAME_TYPE;

            case Constants.URL:
                return Constants.URL_FRAME_TYPE;

            case Constants.TLM:
                return Constants.TLM_FRAME_TYPE;

            case Constants.EID:
                return Constants.EID_FRAME_TYPE;

            default:
                return Constants.EMPTY_FRAME_TYPE;
        }
    }

    public static String getFrameNameFromSlotData(byte[] slotData) {
        if (slotIsEmpty(slotData)) {
            return "--";
        } else {
            return getStringFromFrameType(slotData[0]);
        }
    }

    public static boolean slotIsEmpty(byte[] slotData) {
        return slotData.length < 2;
    }

    /**
     * @param uptime has to be in milliseconds
     * @return String representation rounded down to seconds, minutes, hours or days
     */
    @NonNull
    public static String getTimeString(int uptime) {
        int uptimeSecs = uptime / 10;
        String timeOn;
        if (TimeUnit.SECONDS.toMinutes(uptimeSecs) == 0) {
            timeOn = uptimeSecs + (uptimeSecs == 1 ? " sec" : " secs");
        } else if (TimeUnit.SECONDS.toHours(uptimeSecs) == 0) {
            long uptimeMins = TimeUnit.SECONDS.toMinutes(uptimeSecs);
            timeOn = uptimeMins + (uptimeMins == 1 ? " min" : " mins");
        } else if (TimeUnit.SECONDS.toDays(uptimeSecs) == 0) {
            long uptimeHours = TimeUnit.SECONDS.toHours(uptimeSecs);
            timeOn = uptimeHours + (uptimeHours == 1 ? " hour" : " hours");
        } else {
            long uptimeDays = TimeUnit.SECONDS.toDays(uptimeSecs);
            timeOn = uptimeDays + (uptimeDays == 1 ? " day" : " days");
        }
        return timeOn;
    }

    public static void hideKeyboard(Activity activity, View view) {
        InputMethodManager imm
                = (InputMethodManager) activity.getSystemService(Context.INPUT_METHOD_SERVICE);
        imm.hideSoftInputFromWindow(view.getWindowToken(), 0);
    }

    public static String[] generateArrayWithStringValuesInRange(int min, int max) {
        ArrayList<String> arrayList = new ArrayList<>();
        for (int i = min; i < max; i++) {
            arrayList.add(Integer.toString(i));
        }
        String[] stringArray = new String[arrayList.size()];
        for (int i = 0; i < arrayList.size(); i++) {
            stringArray[i] = arrayList.get(i);
        }
        return stringArray;
    }
}
