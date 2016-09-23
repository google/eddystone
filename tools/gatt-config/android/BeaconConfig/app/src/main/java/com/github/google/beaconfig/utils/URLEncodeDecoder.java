package com.github.google.beaconfig.utils;

import android.util.Log;
import android.util.SparseArray;
import android.webkit.URLUtil;
import java.nio.BufferUnderflowException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.Locale;
import java.util.UUID;

/**
 * Manages decoding of URL from a url service data.
 */
public class URLEncodeDecoder {
    private static final String TAG = URLEncodeDecoder.class.getSimpleName();

    private static final SparseArray<String> URI_SCHEMES = new SparseArray<String>() {{
        put((byte) 0, "http://www.");
        put((byte) 1, "https://www.");
        put((byte) 2, "http://");
        put((byte) 3, "https://");
        put((byte) 4, "urn:uuid:");
    }};

    private static final SparseArray<String> URL_CODES = new SparseArray<String>() {{
        put((byte) 0, ".com/");
        put((byte) 1, ".org/");
        put((byte) 2, ".edu/");
        put((byte) 3, ".net/");
        put((byte) 4, ".info/");
        put((byte) 5, ".biz/");
        put((byte) 6, ".gov/");
        put((byte) 7, ".com");
        put((byte) 8, ".org");
        put((byte) 9, ".edu");
        put((byte) 10, ".net");
        put((byte) 11, ".info");
        put((byte) 12, ".biz");
        put((byte) 13, ".gov");
    }};

    public static String decodeUrlFromUrlBytes(byte[] serviceData) {
        if (serviceData == null || (serviceData != null && serviceData.length == 0)) {
            return null;
        }
        StringBuilder url = new StringBuilder();
        int offset = 0;
        byte b = serviceData[offset++];
        String scheme = URI_SCHEMES.get(b);
        if (scheme != null) {
            url.append(scheme); // too cramped in UI to show scheme.
            if (URLUtil.isNetworkUrl(scheme)) {
                return decodeUrl(serviceData, offset, url);
            } else if ("urn:uuid:".equals(scheme)) {
                return decodeUrnUuid(serviceData, offset, url);
            }
        }
        return url.toString();
    }

    public static String decodeUrl(byte[] serviceData, int offset, StringBuilder urlBuilder) {
        while (offset < serviceData.length) {
            byte b = serviceData[offset++];
            String code = URL_CODES.get(b);
            if (code != null) {
                urlBuilder.append(code);
            } else {
                urlBuilder.append((char) b);
            }
        }
        return urlBuilder.toString();
    }

    static String decodeUrnUuid(byte[] serviceData, int offset, StringBuilder urnBuilder) {
        ByteBuffer bb = ByteBuffer.wrap(serviceData);
        // UUIDs are ordered as byte array, which means most significant first
        bb.order(ByteOrder.BIG_ENDIAN);
        long mostSignificantBytes, leastSignificantBytes;
        try {
            bb.position(offset);
            mostSignificantBytes = bb.getLong();
            leastSignificantBytes = bb.getLong();
        } catch (BufferUnderflowException e) {
            Log.d(TAG, "Decoding UrnUuid failed.");
            return null;
        }
        UUID uuid = new UUID(mostSignificantBytes, leastSignificantBytes);
        urnBuilder.append(uuid.toString());
        return urnBuilder.toString();
    }

    /**
     * Creates the Uri string with embedded expansion codes.
     *
     * @param uri to be encoded
     * @return the Uri string with expansion codes.
     */
    public static byte[] encodeUri(String uri) {
        if (uri.length() == 0) {
            return new byte[0];
        }
        ByteBuffer bb = ByteBuffer.allocate(uri.length());
        // UUIDs are ordered as byte array, which means most significant first
        bb.order(ByteOrder.BIG_ENDIAN);
        int position = 0;

        // Add the byte code for the scheme or return null if none
        Byte schemeCode = encodeUriScheme(uri);
        if (schemeCode == null) {
            return null;
        }
        String scheme = URI_SCHEMES.get(schemeCode);
        bb.put(schemeCode);
        position += scheme.length();

        if (URLUtil.isNetworkUrl(scheme)) {
            return encodeUrl(uri, position, bb);
        } else if ("urn:uuid:".equals(scheme)) {
            return encodeUrnUuid(uri, position, bb);
        }
        return null;
    }

    private static Byte encodeUriScheme(String uri) {
        String lowerCaseUri = uri.toLowerCase(Locale.ENGLISH);
        for (int i = 0; i < URI_SCHEMES.size(); i++) {
            // get the key and value.
            int key = URI_SCHEMES.keyAt(i);
            String value = URI_SCHEMES.valueAt(i);
            if (lowerCaseUri.startsWith(value)) {
                return (byte) key;
            }
        }
        return null;
    }

    private static byte[] encodeUrl(String url, int position, ByteBuffer bb) {
        while (position < url.length()) {
            byte expansion = findLongestExpansion(url, position);
            if (expansion >= 0) {
                bb.put(expansion);
                position += URL_CODES.get(expansion).length();
            } else {
                bb.put((byte) url.charAt(position++));
            }
        }
        return byteBufferToArray(bb);
    }

    private static byte[] encodeUrnUuid(String urn, int position, ByteBuffer bb) {
        String uuidString = urn.substring(position, urn.length());
        UUID uuid;
        try {
            uuid = UUID.fromString(uuidString);
        } catch (IllegalArgumentException e) {
            Log.w("TAG", "encodeUrnUuid invalid urn:uuid format - " + urn);
            return null;
        }
        // UUIDs are ordered as byte array, which means most significant first
        bb.order(ByteOrder.BIG_ENDIAN);
        bb.putLong(uuid.getMostSignificantBits());
        bb.putLong(uuid.getLeastSignificantBits());
        return byteBufferToArray(bb);
    }

    /**
     * Finds the longest expansion from the uri at the current position.
     *
     * @param uriString the Uri
     * @param pos start position
     * @return an index in URI_MAP or 0 if none.
     */
    private static byte findLongestExpansion(String uriString, int pos) {
        byte expansion = -1;
        int expansionLength = 0;
        for (int i = 0; i < URL_CODES.size(); i++) {
            // get the key and value.
            int key = URL_CODES.keyAt(i);
            String value = URL_CODES.valueAt(i);
            if (value.length() > expansionLength && uriString.startsWith(value, pos)) {
                expansion = (byte) key;
                expansionLength = value.length();
            }
        }
        return expansion;
    }

    private static byte[] byteBufferToArray(ByteBuffer bb) {
        byte[] bytes = new byte[bb.position()];
        bb.rewind();
        bb.get(bytes, 0, bytes.length);
        return bytes;
    }
}