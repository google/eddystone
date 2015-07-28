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

import UIKit
import CoreBluetooth

///
/// BeaconScannerDelegate
///
/// Implement this to receive notifications about beacons.
protocol BeaconScannerDelegate {
  func didFindBeacon(beaconScanner: BeaconScanner, beaconInfo: BeaconInfo)
  func didLoseBeacon(beaconScanner: BeaconScanner, beaconInfo: BeaconInfo)
  func didUpdateBeacon(beaconScanner: BeaconScanner, beaconInfo: BeaconInfo)
}

///
/// BeaconScanner
///
/// Scans for Eddystone compliant beacons using Core Bluetooth. To receive notifications of any
/// sighted beacons, be sure to implement BeaconScannerDelegate and set that on the scanner.
///
class BeaconScanner: NSObject, CBCentralManagerDelegate {

  var delegate: BeaconScannerDelegate?

  ///
  /// How long we should go without a beacon sighting before considering it "lost". In seconds.
  ///
  var onLostTimeout: Double = 15.0

  private var centralManager: CBCentralManager!
  private let beaconOperationsQueue: dispatch_queue_t =
      dispatch_queue_create("beacon_operations_queue", nil)
  private var shouldBeScanning: Bool = false

  private var seenEddystoneCache = [String : [String : AnyObject]]()
  private var deviceIDCache = [NSUUID : NSData]()

  override init() {
    super.init()

    self.centralManager = CBCentralManager(delegate: self, queue: self.beaconOperationsQueue)
    self.centralManager.delegate = self
  }

  ///
  /// Start scanning. If Core Bluetooth isn't ready for us just yet, then waits and THEN starts
  /// scanning.
  ///
  func startScanning() {
    dispatch_async(self.beaconOperationsQueue) {
      self.startScanningSynchronized()
    }
  }

  ///
  /// Stops scanning for Eddystone beacons.
  ///
  func stopScanning() {
    self.centralManager.stopScan()
  }

  ///
  /// MARK - private methods and delegate callbacks
  ///

  func centralManagerDidUpdateState(central: CBCentralManager!)  {
    if central.state == CBCentralManagerState.PoweredOn && self.shouldBeScanning {
      self.startScanningSynchronized();
    }
  }

  ///
  /// Core Bluetooth CBCentralManager callback when we discover a beacon. We're not super 
  /// interested in any error situations at this point in time.
  ///
  func centralManager(central: CBCentralManager!,
    didDiscoverPeripheral peripheral: CBPeripheral!,
    advertisementData: [NSObject : AnyObject]!,
    RSSI: NSNumber!) {

      if let serviceData = advertisementData[CBAdvertisementDataServiceDataKey]
        as? [NSObject : AnyObject] {
          var eft: BeaconInfo.EddystoneFrameType
          eft = BeaconInfo.frameTypeForFrame(serviceData)

          // If it's a telemetry frame, stash it away and we'll send it along with the next regular
          // frame we see. Otherwise, process the UID frame.
          if eft == BeaconInfo.EddystoneFrameType.TelemetryFrameType {
            deviceIDCache[peripheral.identifier] = BeaconInfo.telemetryDataForFrame(serviceData)
          } else if eft == BeaconInfo.EddystoneFrameType.UIDFrameType {
            let telemetry = self.deviceIDCache[peripheral.identifier]
            let serviceUUID = CBUUID(string: "FEAA")
            let _RSSI: Int = RSSI.integerValue

            if let
              beaconServiceData = serviceData[serviceUUID] as? NSData,
              beaconInfo = BeaconInfo.beaconInfoForUIDFrameData(beaconServiceData,
              telemetry: telemetry, RSSI: _RSSI) {
                // NOTE: At this point you can choose whether to keep or get rid of the telemetry
                //       data. You can either opt to include it with every single beacon sighting
                //       for this beacon, or delete it until we get a new / "fresh" TLM frame.
                //       We'll treat it as "report it only when you see it", so we'll delete it
                //       each time.
                self.deviceIDCache.removeValueForKey(peripheral.identifier)

                if let cachedData = self.seenEddystoneCache[beaconInfo.beaconID.description] {
                  // Reset the onLost timer and fire the didUpdate.
                  if let timer =
                    self.seenEddystoneCache[beaconInfo.beaconID.description]?["onLostTimer"]
                      as? DispatchTimer {
                        timer.reschedule()
                  }

                  self.delegate?.didUpdateBeacon(self, beaconInfo: beaconInfo)
                } else {
                  // We've never seen this beacon before, fire a didFindBeacon, and mark it as seen
                  self.delegate?.didFindBeacon(self, beaconInfo: beaconInfo)

                  let onLostTimer = DispatchTimer(delay: self.onLostTimeout,
                    queue: dispatch_get_main_queue()) {
                      (timer: DispatchTimer) -> () in
                      let cacheKey = beaconInfo.beaconID.description
                      if let
                        beaconCache = self.seenEddystoneCache[cacheKey],
                        lostBeaconInfo = beaconCache["beaconInfo"] as? BeaconInfo {
                        self.delegate?.didLoseBeacon(self, beaconInfo: lostBeaconInfo)
                        self.seenEddystoneCache.removeValueForKey(
                          beaconInfo.beaconID.description)
                      }
                  }
                  
                  self.seenEddystoneCache[beaconInfo.beaconID.description] = [
                    "beaconInfo" : beaconInfo,
                    "onLostTimer" : onLostTimer
                  ]
                }
            }
          } else if eft == BeaconInfo.EddystoneFrameType.URLFrameType {
            NSLog("Found URL Frame %@", serviceData);
        }
      } else {
        NSLog("Unable to find service data; can't process Eddystone")
      }
  }

  private func startScanningSynchronized() {
    if self.centralManager.state != CBCentralManagerState.PoweredOn {
      NSLog("CentralManager state is %d, cannot start scan", self.centralManager.state.rawValue)
      self.shouldBeScanning = true
    } else {
      NSLog("Starting to scan for Eddystones")
      let services = [CBUUID(string: "FEAA")]
      let options = [CBCentralManagerScanOptionAllowDuplicatesKey : true]
      self.centralManager.scanForPeripheralsWithServices(services, options: options)
    }
  }
}
