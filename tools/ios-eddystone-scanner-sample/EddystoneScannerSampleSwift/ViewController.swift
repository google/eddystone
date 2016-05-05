// Copyright 2015-2016 Google Inc. All rights reserved.
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

class ViewController: UIViewController, BeaconScannerDelegate {

  var beaconScanner: BeaconScanner!

  override func viewDidLoad() {
    super.viewDidLoad()

    self.beaconScanner = BeaconScanner()
    self.beaconScanner!.delegate = self
    self.beaconScanner!.startScanning()
  }

  func didFindBeacon(beaconScanner: BeaconScanner, beaconInfo: BeaconInfo) {
    NSLog("FIND: %@", beaconInfo.description)
  }
  func didLoseBeacon(beaconScanner: BeaconScanner, beaconInfo: BeaconInfo) {
    NSLog("LOST: %@", beaconInfo.description)
  }
  func didUpdateBeacon(beaconScanner: BeaconScanner, beaconInfo: BeaconInfo) {
    NSLog("UPDATE: %@", beaconInfo.description)
  }
  func didObserveURLBeacon(beaconScanner: BeaconScanner, URL: NSURL, RSSI: Int) {
    NSLog("URL SEEN: %@, RSSI: %d", URL, RSSI)
  }

}
