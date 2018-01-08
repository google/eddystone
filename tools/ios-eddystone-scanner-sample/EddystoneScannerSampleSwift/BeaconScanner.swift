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
import CoreBluetooth

///
/// BeaconScannerDelegate
///
/// Implement this to receive notifications about beacons.
protocol BeaconScannerDelegate {
  func didFindBeacon(beaconScanner: BeaconScanner, beaconInfo: BeaconInfo)
  func didLoseBeacon(beaconScanner: BeaconScanner, beaconInfo: BeaconInfo)
  func didUpdateBeacon(beaconScanner: BeaconScanner, beaconInfo: BeaconInfo)
  func didObserveURLBeacon(beaconScanner: BeaconScanner, URL: NSURL, RSSI: Int)
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
  private let beaconOperationsQueue = DispatchQueue(label: "beacon_operations_queue")
  private var shouldBeScanning = false

  private var seenEddystoneCache = [String : [String : AnyObject]]()
  private var deviceIDCache = [UUID : NSData]()

  override init() {
    super.init()

    self.centralManager = CBCentralManager(delegate: self, queue: beaconOperationsQueue)
    self.centralManager.delegate = self
  }

  ///
  /// Start scanning. If Core Bluetooth isn't ready for us just yet, then waits and THEN starts
  /// scanning.
  ///
  func startScanning() {
    beaconOperationsQueue.async {
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
    func centralManagerDidUpdateState(_ central: CBCentralManager) {
    if central.state == .poweredOn && self.shouldBeScanning {
      self.startScanningSynchronized();
    }
  }

  ///
  /// Core Bluetooth CBCentralManager callback when we discover a beacon. We're not super 
  /// interested in any error situations at this point in time.
  ///
  func centralManager(_ central: CBCentralManager,
                      didDiscover peripheral: CBPeripheral,
                      advertisementData: [String : Any],
                      rssi RSSI: NSNumber) {
    if let serviceData = advertisementData[CBAdvertisementDataServiceDataKey]
      as? [NSObject : AnyObject] {
      var eft: BeaconInfo.EddystoneFrameType
      eft = BeaconInfo.frameTypeForFrame(advertisementFrameList: serviceData)

      // If it's a telemetry frame, stash it away and we'll send it along with the next regular
      // frame we see. Otherwise, process the UID frame.
      if eft == BeaconInfo.EddystoneFrameType.TelemetryFrameType {
        deviceIDCache[peripheral.identifier] = BeaconInfo.telemetryDataForFrame(advertisementFrameList: serviceData)
      } else if eft == BeaconInfo.EddystoneFrameType.UIDFrameType
                || eft == BeaconInfo.EddystoneFrameType.EIDFrameType {
        let telemetry = self.deviceIDCache[peripheral.identifier]
        let serviceUUID = CBUUID(string: "FEAA")
        let _RSSI: Int = RSSI.intValue

        if let beaconServiceData = serviceData[serviceUUID] as? NSData,
          let beaconInfo =
            (eft == BeaconInfo.EddystoneFrameType.UIDFrameType
              ? BeaconInfo.beaconInfoForUIDFrameData(frameData: beaconServiceData, telemetry: telemetry,
                                                     RSSI: _RSSI)
              : BeaconInfo.beaconInfoForEIDFrameData(frameData: beaconServiceData, telemetry: telemetry,
                                                     RSSI: _RSSI)) {

          // NOTE: At this point you can choose whether to keep or get rid of the telemetry
          //       data. You can either opt to include it with every single beacon sighting
          //       for this beacon, or delete it until we get a new / "fresh" TLM frame.
          //       We'll treat it as "report it only when you see it", so we'll delete it
          //       each time.
          self.deviceIDCache.removeValue(forKey: peripheral.identifier)

          if (self.seenEddystoneCache[beaconInfo.beaconID.description] != nil) {
            // Reset the onLost timer and fire the didUpdate.
            if let timer =
              self.seenEddystoneCache[beaconInfo.beaconID.description]?["onLostTimer"]
                as? DispatchTimer {
              timer.reschedule()
            }

            self.delegate?.didUpdateBeacon(beaconScanner: self, beaconInfo: beaconInfo)
          } else {
            // We've never seen this beacon before
            self.delegate?.didFindBeacon(beaconScanner: self, beaconInfo: beaconInfo)

            let onLostTimer = DispatchTimer.scheduledDispatchTimer(
              delay: self.onLostTimeout,
              queue: DispatchQueue.main) {
                (timer: DispatchTimer) -> () in
                let cacheKey = beaconInfo.beaconID.description
                if let
                  beaconCache = self.seenEddystoneCache[cacheKey],
                  let lostBeaconInfo = beaconCache["beaconInfo"] as? BeaconInfo {
                  self.delegate?.didLoseBeacon(beaconScanner: self, beaconInfo: lostBeaconInfo)
                  self.seenEddystoneCache.removeValue(
                    forKey: beaconInfo.beaconID.description)
                }
            }

            self.seenEddystoneCache[beaconInfo.beaconID.description] = [
              "beaconInfo" : beaconInfo,
              "onLostTimer" : onLostTimer
            ]
          }
        }
      } else if eft == BeaconInfo.EddystoneFrameType.URLFrameType {
        let serviceUUID = CBUUID(string: "FEAA")
        let _RSSI: Int = RSSI.intValue

        if let beaconServiceData = serviceData[serviceUUID] as? NSData,
          let URL = BeaconInfo.parseURLFromFrame(frameData: beaconServiceData) {
          self.delegate?.didObserveURLBeacon(beaconScanner: self, URL: URL, RSSI: _RSSI)
        }
      }
    } else {
      NSLog("Unable to find service data; can't process Eddystone")
    }
  }

  private func startScanningSynchronized() {
    if self.centralManager.state != .poweredOn {
      NSLog("CentralManager state is %d, cannot start scan", self.centralManager.state.rawValue)
      self.shouldBeScanning = true
    } else {
      NSLog("Starting to scan for Eddystones")
      let services = [CBUUID(string: "FEAA")]
      let options = [CBCentralManagerScanOptionAllowDuplicatesKey : true]
      self.centralManager.scanForPeripherals(withServices: services, options: options)
    }
  }
}
