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

let kBeaconConnectionMaxTimePermitted = 10
let kGetBeaconsServer = "https://proximitybeacon.googleapis.com/v1beta1/beacons/"
let kEddystoneEIDCode = 4
let kBeaconScanTime = 5
let kBeaconDataRequestTimeout = 15

enum OperationState: String {
  case Connected
  case DidDiscoverServices
  case DiscoveringServicesError
  case NotImplementingGATTConfigService
  case DidDiscoverCharacteristics
  case DiscoveringCharacteristicsError
  case ConnectionTimeout
  case CentralManagerConnectionError
  case DidFailToConnect
  case Unknown

  var name: String? {
    switch self {
    case .DiscoveringServicesError:
      return "Discovering Services Error"
    case .DiscoveringCharacteristicsError:
      return "Discovering Characteristics Error"
    case .ConnectionTimeout:
      return "Connection Timeout"
    case .CentralManagerConnectionError:
      return "Central Manager Error"
    case .DidFailToConnect:
      return "Connect Failed"
    case .NotImplementingGATTConfigService:
      return "GATT Configuratin Service"
    default:
      return nil
    }
  }

  var description: String? {
    switch self {
    case .DiscoveringServicesError:
      return "The services could not be discovered."
    case .DiscoveringCharacteristicsError:
      return "The characteristics could not be discovered."
    case .ConnectionTimeout:
      return "The process of connecting to the beacon timed out."
    case .CentralManagerConnectionError:
      return "Central Manager is not ready."
    case .DidFailToConnect:
      return "Connecting to the beacon failed."
    case .NotImplementingGATTConfigService:
      return "The beacon does not implement the GATT configuration service."
    default:
      return nil
    }
  }

}

extension CBCentralManager {
  internal var centralManagerState: CBCentralManagerState  {
    get {
      return CBCentralManagerState(rawValue: state.rawValue) ?? .Unknown
    }
  }
}

///
/// BeaconScanner
///
class BeaconScanner: NSObject, CBCentralManagerDelegate {
  /// Dictionary holding the frames supported by each beacon that we find while scanning
  var beaconsFound = [CBPeripheral : BeaconFramesSupported]()
  var connectingTime = 0
  var scanningTime = 0
  var gettingBeaconDataTime = 0
  var scanningTimer: NSTimer?
  var connectionTimer: NSTimer?
  var gettingBeaconDataTimer: NSTimer?
  var lockQueue: dispatch_queue_t
  var lockQueueForBeaconData: dispatch_queue_t
  var didFinishScanningCallback: (() -> Void)?
  var outstandingBeaconRequests: Int = 0
  var activelyScanning = false

  ///
  /// When discovering a beacon which broadcasts an EID frame, we need to make a HTTP Get
  /// request to see if the beacon was registered with the project that the current user
  /// has selected, which means that the beacon belongs to the user.
  ///
  var myEIDBeacons: NSMutableDictionary = [:]
  private var operationState: OperationState
  private var centralManager: CBCentralManager!
  private var connectingPeripheral: CBPeripheral!
  private let beaconOperationsQueue: dispatch_queue_t =
    dispatch_queue_create("beacon_operations_queue", nil)
  private var shouldBeScanning: Bool = false
  private var GATTOperationCallback:((operationState: OperationState) -> Void)?

  override init() {
    self.operationState = OperationState.Unknown
    self.lockQueue = dispatch_queue_create("LockQueue", nil)
    self.lockQueueForBeaconData = dispatch_queue_create("LockQueueForBeaconData", nil)
    super.init()
    self.centralManager = CBCentralManager(delegate: self, queue: self.beaconOperationsQueue)
    self.centralManager.delegate = self
  }

  ///
  /// Start scanning. If Core Bluetooth isn't ready for us just yet, then waits and THEN starts
  /// scanning.
  ///
  func startScanning(completionHandler: () -> Void) {
    myEIDBeacons.removeAllObjects()
    didFinishScanningCallback = completionHandler
    scanningTime = kBeaconScanTime
    activelyScanning = true
    scanningTimer = NSTimer.scheduledTimerWithTimeInterval(
      1.0,
      target: self,
      selector: #selector(BeaconScanner.subtractScanningTime),
      userInfo: nil,
      repeats: true)
    dispatch_async(self.beaconOperationsQueue) {
      self.startScanningSynchronized()
    }
    if EIDConfiguration.projectID != nil {
      gettingBeaconDataTime = kBeaconDataRequestTimeout
      gettingBeaconDataTimer = NSTimer.scheduledTimerWithTimeInterval(
        1.0,
        target: self,
        selector: #selector(BeaconScanner.subtractGettingBeaconDataTime),
        userInfo: nil,
        repeats: true)
    }
  }

  func subtractScanningTime() {
    scanningTime -= 1
    if scanningTime == 0 {
      self.stopScanning()
    }
  }

  func subtractGettingBeaconDataTime() {
    gettingBeaconDataTime -= 1
    if gettingBeaconDataTime == 0 {
      if let currentTimer = gettingBeaconDataTimer {
        currentTimer.invalidate()
        gettingBeaconDataTimer = nil
      }
      if let callback = didFinishScanningCallback {
        callback()
      }
    }
  }

  ///
  /// Stops scanning for Eddystone beacons.
  ///
  func stopScanning() {
    self.centralManager.stopScan()
    if let currentTimer = scanningTimer {
      currentTimer.invalidate()
    }
    activelyScanning = false

    /// Check if we received all the beacon data by now.
    var outstandingRequests: Int = 0
    dispatch_sync(lockQueueForBeaconData) {
      outstandingRequests = self.outstandingBeaconRequests
    }

    if (outstandingRequests == 0 || gettingBeaconDataTimer == nil) {
      if let callback = didFinishScanningCallback {
        callback()
      }
    }
  }

  ///
  /// MARK - private methods and delegate callbacks
  ///
  func centralManagerDidUpdateState(central: CBCentralManager) {
    if central.centralManagerState == CBCentralManagerState.PoweredOn && self.shouldBeScanning {
      self.startScanningSynchronized()
    }
  }

  ///
  /// Core Bluetooth CBCentralManager callback when we discover a beacon. We're not super
  /// interested in any error situations at this point in time, we only parse each frame
  /// and save it.
  ///
  func centralManager(central: CBCentralManager,
                      didDiscoverPeripheral peripheral: CBPeripheral,
                                            advertisementData: [String : AnyObject],
                                            RSSI: NSNumber) {
    if let serviceData = advertisementData[CBAdvertisementDataServiceDataKey]
      as? [NSObject : AnyObject] {
      var eft: BeaconInfo.EddystoneFrameType
      eft = BeaconInfo.frameTypeForFrame(advertisementData)
      if beaconsFound[peripheral] == nil {
        let beaconGATTOperations = GATTOperations(peripheral: peripheral)
        beaconsFound[peripheral] = BeaconFramesSupported(operations: beaconGATTOperations)
      }

      if eft == BeaconInfo.EddystoneFrameType.TelemetryFrameType {
        let serviceUUID = CBUUID(string: "FEAA")
        if let
          beaconServiceData = serviceData[serviceUUID] as? NSData,
          (battery, temperature, PDU, time) = BeaconInfo.parseTLMFromFrame(beaconServiceData) {
          let (days, hours, _) = BeaconInfo.convertDeciseconds(time)
          let TLM = "\(battery)mV/bit\n\(temperature) °C\n\(PDU)\n\(days) days, \(hours) hours"
          beaconsFound[peripheral]?.setTLMframe(TLM)
        }
      } else if eft == BeaconInfo.EddystoneFrameType.UIDFrameType
        || eft == BeaconInfo.EddystoneFrameType.EIDFrameType {

        let serviceUUID = CBUUID(string: "FEAA")
        let _RSSI: Int = RSSI.integerValue

        if let
          beaconServiceData = serviceData[serviceUUID] as? NSData,
          beaconInfo =
          (eft == BeaconInfo.EddystoneFrameType.UIDFrameType
            ? BeaconInfo.beaconInfoForUIDFrameData(beaconServiceData, RSSI: _RSSI)
            : BeaconInfo.beaconInfoForEIDFrameData(beaconServiceData, RSSI: _RSSI)) {

          if eft == BeaconInfo.EddystoneFrameType.UIDFrameType {
            if let beacon = beaconsFound[peripheral] {
              beacon.setUIDframe(String(beaconInfo.beaconID))
            }
          }
          else if eft == BeaconInfo.EddystoneFrameType.EIDFrameType {
            beaconsFound[peripheral]?.setEIDframe(String(beaconInfo.beaconID))
            ///
            /// If we already have information whether the EID Beacon was registered with the
            /// selected project, there's no need for us to make another request.
            ///
            if myEIDBeacons[String(beaconInfo.beaconID)] == nil {
              ///
              /// The user can use the app without being logged in or having a project selected,
              /// so we will simply not check if the beacon belongs to the user if the user is not
              /// authenticated.
              ////
              if let projectID = EIDConfiguration.projectID {
                dispatch_sync(lockQueueForBeaconData) {
                  ////
                  /// We want to know how many requestst we made, in order to keep track of how
                  /// many of them return to us.
                  ////
                  self.outstandingBeaconRequests += 1
                }
                ///
                /// We're interested to find if the EID-Beacon that we have was registered
                /// using the project that is currently selected.
                ///
                getBeaconHTTPRequest(String(beaconInfo.beaconID), projectID: projectID)
              }
            }
          }
        }
      } else if eft == BeaconInfo.EddystoneFrameType.URLFrameType {
        let serviceUUID = CBUUID(string: "FEAA")

        if let
          beaconServiceData = serviceData[serviceUUID] as? NSData,
          URL = BeaconInfo.parseURLFromFrame(beaconServiceData) {
          beaconsFound[peripheral]?.setURLframe(String(URL))
        }
      } else if eft == BeaconInfo.EddystoneFrameType.EddystoneGATTServiceFrameType {
        beaconsFound[peripheral]?.setEddystoneGATTServiceFrame()
      }
    }
  }

  func didUpdateState(operationState: OperationState) {
    self.operationState = operationState

    if let currentTimer = connectionTimer {
      currentTimer.invalidate()
    }
    if  operationState == OperationState.Connected {
      if let beaconGATTOperations = beaconsFound[connectingPeripheral]?.operations {
        if let callback = GATTOperationCallback {
          beaconGATTOperations.discoverServices(callback)
        }
      }
    } else if let UICallback = GATTOperationCallback {
      UICallback(operationState: operationState)
    }
  }

  ///
  /// Attempts to connect to a beacon if the CB Central Manager is ready;
  /// We set a timer because we assume that if the connection can't be established in that
  /// amount of time, there's a problem that the user needs to consider.
  ///
  func connectToBeacon(peripheral: CBPeripheral,
                       callback: (operationState: OperationState) -> Void) {
    GATTOperationCallback = callback
    if self.centralManager.centralManagerState == CBCentralManagerState.PoweredOn {
      connectingPeripheral = peripheral
      if peripheral.state != CBPeripheralState.Connected {
        self.connectToPeripheralWithTimeout(connectingPeripheral)
      } else {
        if let beaconGATTOperations = beaconsFound[connectingPeripheral]?.operations {
          beaconGATTOperations.discoverServices(callback)
        }
      }
    } else {
      NSLog("CentralManager state is %d, cannot connect", self.centralManager.state.rawValue)
      didUpdateState(OperationState.CentralManagerConnectionError)
    }
  }

  func connectToPeripheralWithTimeout(peripheral: CBPeripheral) {
    self.setupTimer()
    centralManager.connectPeripheral(connectingPeripheral, options: nil)
  }

  func setupTimer() {
    connectingTime = kBeaconConnectionMaxTimePermitted
    connectionTimer = NSTimer.scheduledTimerWithTimeInterval(
      1.0,
      target: self,
      selector: #selector(BeaconScanner.subtractConnectingTime),
      userInfo: nil,
      repeats: true)
  }

  func subtractConnectingTime() {
    connectingTime -= 1
    if connectingTime == 0 {
      centralManager.cancelPeripheralConnection(connectingPeripheral)
      didUpdateState(OperationState.ConnectionTimeout)
    }
  }

  func centralManager(central: CBCentralManager,
                      didFailToConnectPeripheral peripheral: CBPeripheral,
                                                 error: NSError?) {
    NSLog("Connecting failed");
    ///
    /// Other situations in which the connection attempt fails,
    /// the timeout being treated separately.
    ///
    if operationState != OperationState.ConnectionTimeout {
      didUpdateState(OperationState.DidFailToConnect)
    }
  }

  func centralManager(central: CBCentralManager, didConnectPeripheral peripheral: CBPeripheral) {
    NSLog("Beacon connected")
    didUpdateState(OperationState.Connected)
  }

  func disconnectFromBeacon(peripheral: CBPeripheral) {
    if self.centralManager.centralManagerState == CBCentralManagerState.PoweredOn {
      centralManager.cancelPeripheralConnection(peripheral)
    } else {
      NSLog("CentralManager state is %d, cannot disconnect", self.centralManager.state.rawValue)
    }
  }

  private func startScanningSynchronized() {
    if self.centralManager.centralManagerState != CBCentralManagerState.PoweredOn {
      NSLog("CentralManager state is %d, cannot start scan", self.centralManager.state.rawValue)
      self.shouldBeScanning = true
    } else {
      NSLog("Starting to scan for Eddystones")
      let services = [CBUUID(string: "FEAA")]
      let options = [CBCentralManagerScanOptionAllowDuplicatesKey : true]
      self.centralManager.scanForPeripheralsWithServices(services, options: options)
    }
  }

  /// Creates the array with all the necessary information to be displayed.
  func populateBeaconItems() -> [BeaconItem] {
    var beaconItems: [BeaconItem] = [BeaconItem]()
    for (beacon, frames) in beaconsFound {
      let beaconGATTOperations = beaconsFound[beacon]!.operations
      let beaconItem: BeaconItem = BeaconItem(peripheral: beacon,
                                              frames: frames,
                                              operations: beaconGATTOperations)
      if frames.containsUsefulFrames() {
        beaconItems.append(beaconItem)
      }

    }
    return beaconItems
  }

  /// Checkes whether or not an EID beacon was registered using this projectID.
  func getBeaconHTTPRequest(EID: String, projectID: String) {
    if GIDSignIn.sharedInstance().currentUser != nil {
      let bearer = GIDSignIn.sharedInstance().currentUser.authentication.accessToken
      let bearerHeader = "Bearer \(bearer)"
      let server = "\(kGetBeaconsServer)\(kEddystoneEIDCode)!\(EID)?projectId=\(projectID)"
      let url = NSURL(string: server)
      let httpHeaders = ["Authorization" : bearerHeader,
                         "Accept" : "application/json"]
      if let requestURL = url {
        ///
        /// The actual response of the request is not important; we're just checking if we are
        /// able to get the beacon information, which would mean that it was registered using
        /// the current project.
        ///
        HTTPRequest.makeHTTPRequest(requestURL,
                                    method: "GET",
                                    postBody: nil,
                                    requestHeaders: httpHeaders) { statusCode, _, _ in
          if self.gettingBeaconDataTimer != nil {
            var outstandingRequests: Int = 0
            dispatch_sync(self.lockQueueForBeaconData) {
              self.outstandingBeaconRequests -= 1
              outstandingRequests = self.outstandingBeaconRequests
            }
            dispatch_sync(self.lockQueue) {
              /// We're not interested in any statusCode other than 200.
              if statusCode == 200 {
                self.myEIDBeacons[EID] = true
              } else {
                self.myEIDBeacons[EID] = false
              }
            }
            ///
            /// If we are not currently scanning and we have no active
            /// requests, it means that we're done and we have all the
            /// information that we need.
            ///
            if outstandingRequests == 0 && !self.activelyScanning {
              if let
                currentTimer = self.gettingBeaconDataTimer,
                callback = self.didFinishScanningCallback {
                currentTimer.invalidate()
                self.gettingBeaconDataTimer = nil
                callback()
              }
            }
          }
        }
      }
    }
  }
}
