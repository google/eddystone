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

import Foundation
import UIKit

class StringUtils {
  class func containsOnlyCharactersInString(text: String, matchCharacters: String) -> Bool {
    let disallowedCharacterSet = NSCharacterSet(charactersInString: matchCharacters).invertedSet
    return text.rangeOfCharacterFromSet(disallowedCharacterSet) == nil
  }

  class func inHexadecimalString(text: String) -> Bool {
    return containsOnlyCharactersInString(text, matchCharacters: "0123456789ABCDEFabcdef")
  }

  ///
  /// converts a hex string to actual hex value
  /// e.g. "1a5fbb" -> [0x1a, 0x5f, 0xbb]
  ///
  class func transformStringToByteArray(string: String) -> [UInt8] {
    let characters = Array(string.characters)
    let bytes = 0.stride(to: characters.count, by: 2).map {
      UInt8(String(characters[$0 ..< $0+2]), radix: 16) ?? 0xff
    }
    return bytes
  }

  class func convertNSDataToBase64String(data: NSData) -> String {
    return data.base64EncodedStringWithOptions(.Encoding64CharacterLineLength)
  }

  class func md5(string: String) -> NSData {
    var digest = [UInt8](count: Int(CC_MD5_DIGEST_LENGTH), repeatedValue: 0)
    if let data = string.dataUsingEncoding(NSUTF8StringEncoding) {
      CC_MD5(data.bytes, CC_LONG(data.length), &digest)
    }
    let data = NSData(bytes: digest, length: digest.count * sizeof(UInt8))
    return data
  }
}
