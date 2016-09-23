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
import Foundation
import CoreBluetooth

let kEIDFrameLength = 14
///
/// Beacons need a couple of seconds to generate the correct EID ADV Slot data after writing out to
/// that slot and to generate the beacon ECDH public key, so we're waiting for them.
///
let kWaitingForBeaconTime: Double = 3
let kECDHLengthKey = 32
let kEIDLength = 8

///
/// We need a couple of things both from the beacon and the service and we want to make sure we
/// gather all of it and that it's correct.
///
class EIDRegistrationData {
  var beaconEcdhPublicKey: String?
  var serviceEcdhPublicKey: String?
  var rotationPeriodExponent: NSNumber?
  var initialClockValue: String?
  var initialEid: String?

  func isValid() -> Bool {
    return beaconEcdhPublicKey != nil &&
           serviceEcdhPublicKey != nil &&
           rotationPeriodExponent != nil &&
           initialClockValue != nil &&
           initialEid != nil
  }
}

class ConfigureBeaconSlotData: GATTOperations {
  var maxSupportedTotalSlots = 0
  var currentlyUpdatedSlot = 0
  var slotUpdateData = [NSNumber : [String : NSData]]()
  var beaconCapabilities: NSDictionary = [:]
  var callback: (() -> Void)?
  var isEIDSlot = false
  var registrationEIDData: EIDRegistrationData?
  var statusInfoUpdateAlert: UIAlertController?

  enum ConfigurationState {
    case ReceivedUpdateData
    case DidSetActiveSlot
    case ErrorSettingActiveSlot
    case DidUpdateTxPower
    case ErrorUpdatingTxPower
    case DidUpdateAdvInterval
    case ErrorUpdatingAdvInterval
    case DidUpdateSlotData
    case ErrorUpdatingSlotData
    case UpdatedAllSlots
  }

  func didUpdateConfigurationState(configurationState: ConfigurationState) {
    switch configurationState {
    case .ReceivedUpdateData:
      setActiveSlot()
    case .DidSetActiveSlot:
      updateTxPower()
    case .DidUpdateTxPower:
      updateAdvInterval()
    case .DidUpdateAdvInterval:
      updateSlotData()
    case .DidUpdateSlotData:
      ///
      /// The EID slot configuration is special and requires some extra steps:
      ///   * reading the exponent, the value and the initial EID from the slot
      ///   * reading the public ECDH Key
      ///
      if isEIDSlot {
        let delayTime = dispatch_time(DISPATCH_TIME_NOW,
                                      Int64(kWaitingForBeaconTime * Double(NSEC_PER_SEC)))
        dispatch_after(delayTime, dispatch_get_main_queue()) {
          self.readEIDFrame()
        }
        isEIDSlot = false
      } else {
        currentlyUpdatedSlot += 1; setActiveSlot()
      }
    /// We're done!
    case .UpdatedAllSlots:
      if let UICallback = callback {
        UICallback()
      }
    ///
    /// If we have any problem with the update of the current slot, we just try to update the next
    /// one; there's no point in discarding all changes just because one failed.
    ///
    case .ErrorUpdatingSlotData:
      currentlyUpdatedSlot += 1; setActiveSlot()
    default:
      break
    }
  }

  func setActiveSlot() {
    if currentlyUpdatedSlot >= maxSupportedTotalSlots {
      didUpdateConfigurationState(ConfigurationState.UpdatedAllSlots)
    } else {
      if let characteristic = findCharacteristicByID(CharacteristicID.activeSlot.UUID) {
        let currentSlot = NSData(bytes: &currentlyUpdatedSlot, length: 1)
        peripheral.writeValue(currentSlot,
                              forCharacteristic: characteristic,
                              type: CBCharacteristicWriteType.WithResponse)
      }
    }
  }

  override func peripheral(peripheral: CBPeripheral,
                           didWriteValueForCharacteristic characteristic: CBCharacteristic,
                                                          error: NSError?) {

    if error != nil {
      switch characteristic.UUID {
      case CharacteristicID.activeSlot.UUID:
        didUpdateConfigurationState(ConfigurationState.ErrorSettingActiveSlot)
      case CharacteristicID.radioTxPower.UUID:
        didUpdateConfigurationState(ConfigurationState.ErrorUpdatingTxPower)
      case CharacteristicID.advertisingInterval.UUID:
        didUpdateConfigurationState(ConfigurationState.ErrorUpdatingAdvInterval)
      case CharacteristicID.ADVSlotData.UUID:
        didUpdateConfigurationState(ConfigurationState.ErrorUpdatingSlotData)
      default:
        break
      }
    } else {
      switch characteristic.UUID {
      case CharacteristicID.activeSlot.UUID:
        didUpdateConfigurationState(ConfigurationState.DidSetActiveSlot)
      case CharacteristicID.radioTxPower.UUID:
        didUpdateConfigurationState(ConfigurationState.DidUpdateTxPower)
      case CharacteristicID.advertisingInterval.UUID:
        didUpdateConfigurationState(ConfigurationState.DidUpdateAdvInterval)
      case CharacteristicID.ADVSlotData.UUID:
        didUpdateConfigurationState(ConfigurationState.DidUpdateSlotData)
      default:
        break
      }
    }
  }

  override func peripheral(peripheral: CBPeripheral,
                           didUpdateValueForCharacteristic characteristic: CBCharacteristic,
                                                           error: NSError?) {
    NSLog("did update value for characteristic \(characteristic.UUID)")
    if let identifiedError = error {
      NSLog("Error reading characteristic: \(identifiedError)")
    } else {
      switch characteristic.UUID {
      case CharacteristicID.ADVSlotData.UUID:
        parseEIDFrame()
      case CharacteristicID.publicECDHKey.UUID:
        didReadPublicECDH()
      default: break
      }
    }
  }

  func readEIDFrame() {
    if let characteristic = findCharacteristicByID(CharacteristicID.ADVSlotData.UUID) {
      peripheral.readValueForCharacteristic(characteristic)
    }
  }

  func readPublicECDH() {
    if let characteristic = findCharacteristicByID(CharacteristicID.publicECDHKey.UUID) {
      peripheral.readValueForCharacteristic(characteristic)
    }
  }

  func didReadPublicECDH() {
    if let
      characteristic = findCharacteristicByID(CharacteristicID.publicECDHKey.UUID),
      value = characteristic.value {
      // The ECDH public key's length is 32 bytes
      if value.length == kECDHLengthKey {
        var publicECDH = [UInt8](count: kECDHLengthKey, repeatedValue: 0)
        value.getBytes(&publicECDH, length: kECDHLengthKey * sizeof(UInt8))
        let publicECDHBase64String = StringUtils.convertNSDataToBase64String(value)
        if let registrationData = registrationEIDData {
          registrationData.beaconEcdhPublicKey = publicECDHBase64String
          if registrationData.isValid() {
            /// If we have all the registration data, we can finally register the beacon.
            let configureEID = EIDConfiguration()
            configureEID.registerBeacon(registrationData) { didRegister in
              if didRegister {
                self.didUpdateConfigurationState(ConfigurationState.DidUpdateSlotData)
              } else {
                self.didUpdateConfigurationState(ConfigurationState.ErrorUpdatingSlotData)
              }
            }
          } else {
            didUpdateConfigurationState(ConfigurationState.ErrorUpdatingSlotData)
          }
        } else {
          didUpdateConfigurationState(ConfigurationState.ErrorUpdatingSlotData)
        }
      }
    }
  }

  func parseEIDFrame() {
    if let
      characteristic = findCharacteristicByID(CharacteristicID.ADVSlotData.UUID),
      value = characteristic.value {
      var frameType: UInt8 = 0
      value.getBytes(&frameType, length: sizeof(UInt8))
      if frameType == BeaconInfo.EddystoneEIDFrameTypeID && value.length == kEIDFrameLength {
        // The EID Slot Data has the following structure:
        //    1 byte frame type
        //    1 byte exponent
        //    4 byte clock value
        //    8 byte EID
        var exponent: UInt8 = 0
        var clockValue: UInt32 = 0
        var EIDValue = [UInt8](count: kEIDLength, repeatedValue: 0)
        value.getBytes(&exponent, range: NSMakeRange(1, 1))
        value.getBytes(&clockValue, range: NSMakeRange(2, 4))
        value.getBytes(&EIDValue, range: NSMakeRange(6, 8))
        /// The clock value is in big endian format, so we want to convert it to little endian.
        clockValue = CFSwapInt32BigToHost(clockValue)
        if let registrationData = registrationEIDData {
          registrationData.rotationPeriodExponent = NSNumber(unsignedChar: exponent)
          let clockValueString = "\(clockValue)"
          registrationData.initialClockValue = clockValueString
          let EIDData = NSData(bytes: EIDValue, length: kEIDLength)
          let EIDBase64String = StringUtils.convertNSDataToBase64String(EIDData)
          registrationData.initialEid = EIDBase64String
        }

        /// We're giving the beacon it needs to compute its public ECDH key.
        let delayTime = dispatch_time(DISPATCH_TIME_NOW,
                                      Int64(kWaitingForBeaconTime * Double(NSEC_PER_SEC)))
        dispatch_after(delayTime, dispatch_get_main_queue()) {
          self.readPublicECDH()
        }
      } else {
        didUpdateConfigurationState(ConfigurationState.ErrorUpdatingSlotData)
      }
    }
  }

  // TODO: possibly send as callback a list of errors that appeared while configuring slots
  func beginBeaconConfiguration(beaconCapabilities: NSDictionary,
                                statusInfoAlert: UIAlertController,
                                slotUpdateData: Dictionary <NSNumber, Dictionary <String, NSData>>,
                                callback: () -> Void) {
    self.callback = callback
    self.maxSupportedTotalSlots = beaconCapabilities[maxSupportedSlotsKey] as! Int
    self.currentlyUpdatedSlot = 0
    self.slotUpdateData = slotUpdateData
    self.beaconCapabilities = beaconCapabilities
    self.statusInfoUpdateAlert = statusInfoAlert
    peripheral.delegate = self
    didUpdateConfigurationState(ConfigurationState.ReceivedUpdateData)
  }

  func updateTxPower() {
    var slotNumber = currentlyUpdatedSlot + 1
    if let perSlotTxPowerSupported = beaconCapabilities[perSlotTxPowerSupportedKey] {
      if !(perSlotTxPowerSupported as! Bool) {
        slotNumber = 0
      }
    }
    if let
      currentSlotData = slotUpdateData[slotNumber],
      value = currentSlotData[slotDataTxPowerKey],
      characteristic = findCharacteristicByID(CharacteristicID.radioTxPower.UUID) {
      var txPower: Int8 = 0
      value.getBytes(&txPower, length: sizeof(Int8))
      let val = NSData(bytes: &txPower, length: sizeof(Int8))
      peripheral.writeValue(val, forCharacteristic: characteristic, type: .WithResponse)
    } else {
      updateAdvInterval()
    }
  }

  func updateAdvInterval() {
    var slotNumber = currentlyUpdatedSlot + 1
    if let perSlotAdvIntervalSupported = beaconCapabilities[perSlotAdvIntervalsSupportedKey] {
      if !(perSlotAdvIntervalSupported as! Bool) {
        slotNumber = 0
      }
    }
    if let
      currentSlotData = slotUpdateData[slotNumber],
      value = currentSlotData[slotDataAdvIntervalKey],
      characteristic = findCharacteristicByID(CharacteristicID.advertisingInterval.UUID) {
      var advInterval: UInt16 = 0
      value.getBytes(&advInterval, length: sizeof(UInt16))
      var bigEndianAdvInterv: UInt16 = CFSwapInt16HostToBig(advInterval)
      let val = NSData(bytes: &bigEndianAdvInterv, length: sizeof(UInt16))
      peripheral.writeValue(val, forCharacteristic: characteristic, type: .WithResponse)
    } else {
      updateSlotData()
    }
  }

  ///
  /// We usually get the slot data in the right format from the moment when we create a dictionary
  /// with it. There's no need to do more computations.
  ///
  func updateSlotData() {
    isEIDSlot = false
    let slotNumber = currentlyUpdatedSlot + 1
    if let
      currentSlotData = slotUpdateData[slotNumber],
      value = currentSlotData[slotDataURLKey],
      characteristic = findCharacteristicByID(CharacteristicID.ADVSlotData.UUID) {
      peripheral.writeValue(value, forCharacteristic: characteristic, type: .WithResponse)
    } else if let
      currentSlotData = slotUpdateData[slotNumber],
      value = currentSlotData[slotDataUIDKey],
      characteristic = findCharacteristicByID(CharacteristicID.ADVSlotData.UUID) {
      peripheral.writeValue(value, forCharacteristic: characteristic, type: .WithResponse)
    } else if let
      currentSlotData = slotUpdateData[slotNumber],
      value = currentSlotData[slotDataTLMKey],
      characteristic = findCharacteristicByID(CharacteristicID.ADVSlotData.UUID) {
      peripheral.writeValue(value, forCharacteristic: characteristic, type: .WithResponse)
    } else if let
      currentSlotData = slotUpdateData[slotNumber],
      value = currentSlotData[slotDataNoFrameKey],
      characteristic = findCharacteristicByID(CharacteristicID.ADVSlotData.UUID) {
      peripheral.writeValue(value, forCharacteristic: characteristic, type: .WithResponse)
    } else if let
      currentSlotData = slotUpdateData[slotNumber],
      _ = currentSlotData[slotDataEIDKey],
      characteristic = findCharacteristicByID(CharacteristicID.ADVSlotData.UUID) {
      ///
      /// The EID configuration requires additional steps; we don't have the frameData yet and
      /// we must take it from the server. After writing it, we're still not done with this!
      ///
      isEIDSlot = true
      let configurationEID = EIDConfiguration()
      configurationEID.getEIDParams() {serviceKey, minRotationExponent, maxRotationExponent in
        if let
          key = serviceKey,
          minExponent = minRotationExponent,
          _ = maxRotationExponent {
          let frameData = NSMutableData()
          frameData.appendBytes([BeaconInfo.EddystoneEIDFrameTypeID], length: sizeof(UInt8))
          if let
            serviceKeyData = NSData(base64EncodedString: key,
              options: NSDataBase64DecodingOptions.IgnoreUnknownCharacters) {
            frameData.appendData(serviceKeyData)
            if serviceKeyData.length == kECDHLengthKey {
              self.registrationEIDData = EIDRegistrationData()
              self.registrationEIDData?.serviceEcdhPublicKey = key
              frameData.appendBytes([minExponent], length: sizeof(UInt8))
              self.peripheral.writeValue(frameData,
                forCharacteristic: characteristic,
                type: .WithResponse)
            } else {
              self.didUpdateConfigurationState(ConfigurationState.ErrorUpdatingSlotData)
            }
          }
        } else {
          self.didUpdateConfigurationState(ConfigurationState.ErrorUpdatingSlotData)
        }
      }
    } else {
      currentlyUpdatedSlot += 1
      setActiveSlot()
    }
  }
}
