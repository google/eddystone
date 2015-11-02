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

package com.google.sample.txeddystone;

import android.view.View;

import java.util.Random;

class Utils {

  private Utils() {}  // static functions only

  private static final char[] HEX = "0123456789ABCDEF".toCharArray();

  static String toHexString(byte[] bytes) {
    char[] chars = new char[bytes.length * 2];
    for (int i = 0; i < bytes.length; i++) {
      int c = bytes[i] & 0xFF;
      chars[i * 2] = HEX[c >>> 4];
      chars[i * 2 + 1] = HEX[c & 0x0F];
    }
    return new String(chars).toLowerCase();
  }

  static boolean isValidHex(String s, int expectedLen) {
    return !(s == null || s.isEmpty()) && (s.length() / 2) == expectedLen && s.matches("[0-9A-F]+");
  }

  static byte[] toByteArray(String hexString) {
    // hexString guaranteed valid.
    int len = hexString.length();
    byte[] bytes = new byte[len / 2];
    for (int i = 0; i < len; i += 2) {
      bytes[i / 2] = (byte) ((Character.digit(hexString.charAt(i), 16) << 4)
          + Character.digit(hexString.charAt(i + 1), 16));
    }
    return bytes;
  }

  static String randomHexString(int length) {
    byte[] buf = new byte[length];
    new Random().nextBytes(buf);
    StringBuilder stringBuilder = new StringBuilder();
    for (int i = 0; i < length; i++) {
      stringBuilder.append(String.format("%02X", buf[i]));
    }
    return stringBuilder.toString();
  }

  static void setEnabledViews(boolean enabled, View... views) {
    if (views == null || views.length == 0) {
      return;
    }
    for (View v : views) {
      v.setEnabled(enabled);
    }
  }

}
