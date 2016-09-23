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
import CoreBluetooth

///
/// We're interested in having all the information that we need about each beacon that we find
/// while scanning in one place in order to use it when displaying the results. The frames for
/// each beacon are discovered at different moments of time and we need to be able to update the
/// information all the time.
///
class BeaconItem {
  var peripheral: CBPeripheral
  var frames: BeaconFramesSupported
  var operations: GATTOperations?
  var framesView: UIView

  init(peripheral: CBPeripheral, frames: BeaconFramesSupported, operations: GATTOperations) {
    self.peripheral = peripheral
    self.frames = frames
    self.operations = operations
    self.framesView = UIView()
    var topView: UIView = self.framesView
    var pinTop: NSLayoutAttribute = .Top
    if let UID = frames.UIDData {
      topView = CustomViews.createFrameDisplayView(.UIDFrameType,
                                                   data:UID,
                                                   holder: self.framesView,
                                                   topView: topView,
                                                   pinTop: pinTop,
                                                   setBottomConstraints:
                                                      UIDNeedBottomConstraints(frames))
    }

    if let EID = frames.EIDData {
      if topView != self.framesView {
        pinTop = .Bottom
      }
      topView = CustomViews.createFrameDisplayView(.EIDFrameType,
                                                   data: EID,
                                                   holder: self.framesView,
                                                   topView: topView,
                                                   pinTop: pinTop,
                                                   setBottomConstraints:
                                                      EIDNeedBottomConstraints(frames))
    }

    if let URL = frames.URLData {
      if topView != self.framesView {
        pinTop = .Bottom
      }
      topView = CustomViews.createFrameDisplayView(.URLFrameType,
                                                   data:URL,
                                                   holder: self.framesView,
                                                   topView: topView,
                                                   pinTop: pinTop,
                                                   setBottomConstraints:
                                                      URLNeedBottomConstraints(frames))
    }

    if let TLM = frames.TLMData {
      if topView != self.framesView {
        pinTop = .Bottom
      }
      topView = CustomViews.createFrameDisplayView(.TelemetryFrameType,
                                                   data: TLM,
                                                   holder: self.framesView,
                                                   topView: topView,
                                                   pinTop: pinTop,
                                                   setBottomConstraints:
                                                      TLMNeedBottomConstraints(frames))
    }

    if frames.EddystoneGATTServiceFrameSupported {
      if topView != self.framesView {
        pinTop = .Bottom
      }
      CustomViews.displayConfigurableBeacon(self.framesView, topView: topView, pinTop: pinTop)
    }
  }

  func UIDNeedBottomConstraints(frames: BeaconFramesSupported) -> Bool {
    return  !frames.EIDFrameSupported &&
      !frames.URLFrameSupported &&
      !frames.TLMFrameSupported &&
      !frames.EddystoneGATTServiceFrameSupported
  }

  func EIDNeedBottomConstraints(frames: BeaconFramesSupported) -> Bool {
    return  !frames.URLFrameSupported &&
      !frames.TLMFrameSupported &&
      !frames.EddystoneGATTServiceFrameSupported
  }

  func URLNeedBottomConstraints(frames: BeaconFramesSupported) -> Bool {
    return  !frames.TLMFrameSupported &&
      !frames.EddystoneGATTServiceFrameSupported
  }

  func TLMNeedBottomConstraints(frames: BeaconFramesSupported) -> Bool {
    return !frames.EddystoneGATTServiceFrameSupported
  }

}
