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

public class Utils {
  private static final char[] HEX = "0123456789ABCDEF".toCharArray();

  static String toHexString(byte[] bytes) {
    if (bytes.length == 0) {
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

  static boolean isZeroed(byte[] bytes) {
    for (byte b : bytes) {
      if (b != 0x00) {
        return false;
      }
    }
    return true;
  }

}
