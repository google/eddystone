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

import android.util.Log;
import android.util.SparseArray;
import android.webkit.URLUtil;

import java.nio.BufferUnderflowException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.UUID;


/**
 * Helpers for Eddystone-URL frame validation. Copied from
 * https://github.com/google/uribeacon/android-uribeacon/uribeacon-library
 */
public class UrlUtils {
  private static final String TAG = UrlUtils.class.getSimpleName();

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

  static String decodeUrl(byte[] serviceData) {
    StringBuilder url = new StringBuilder();
    int offset = 2;
    byte b = serviceData[offset++];
    String scheme = URI_SCHEMES.get(b);
    if (scheme != null) {
      url.append(scheme);
      if (URLUtil.isNetworkUrl(scheme)) {
        return decodeUrl(serviceData, offset, url);
      } else if ("urn:uuid:".equals(scheme)) {
        return decodeUrnUuid(serviceData, offset, url);
      }
    }
    return url.toString();
  }

  static String decodeUrl(byte[] serviceData, int offset, StringBuilder urlBuilder) {
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
      Log.w(TAG, "decodeUrnUuid BufferUnderflowException!");
      return null;
    }
    UUID uuid = new UUID(mostSignificantBytes, leastSignificantBytes);
    urnBuilder.append(uuid.toString());
    return urnBuilder.toString();
  }

}
