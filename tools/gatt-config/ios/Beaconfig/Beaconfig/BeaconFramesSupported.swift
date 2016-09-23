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

import UIKit

///
/// Identifies the frames broadcasted by a beacon during the scan and
/// parses the information for each type. It is used to provide the frame
/// types for each beacon.
///
class BeaconFramesSupported {
  var UIDFrameSupported: Bool = false
  var EIDFrameSupported: Bool = false
  var TLMFrameSupported: Bool = false
  var URLFrameSupported: Bool = false
  var EddystoneGATTServiceFrameSupported: Bool = false
  var URLData: String?
  var UIDData: String?
  var EIDData: String?
  var TLMData: String?
  var operations: GATTOperations

  init(operations: GATTOperations) {
    self.operations = operations
  }

  /// We check if the beacon we scanned for has any frame that is useful for us.
  func containsUsefulFrames() -> Bool {
    return UIDFrameSupported || EIDFrameSupported || EddystoneGATTServiceFrameSupported ||
           URLFrameSupported || TLMFrameSupported
  }

  func setUIDframe(UID: String) {
    UIDFrameSupported = true
    UIDData = UID
  }

  func setEIDframe(EID: String) {
    EIDFrameSupported = true
    EIDData = EID
  }

  func setTLMframe(TLM: String) {
    TLMFrameSupported = true
    TLMData = TLM
  }

  func setURLframe(URL: String) {
    URLFrameSupported = true
    URLData = URL
  }

  func setEddystoneGATTServiceFrame() {
    EddystoneGATTServiceFrameSupported = true
  }
}
