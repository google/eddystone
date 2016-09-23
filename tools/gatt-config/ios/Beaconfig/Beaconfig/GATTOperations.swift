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

enum LockState: UInt8 {
  case Locked = 0
  case Unlocked
  case UnlockedPreventAutolock
  case Unknown = 255
}

enum CharacteristicID {
  case capabilities
  case activeSlot
  case advertisingInterval
  case radioTxPower
  case advertisedTxPower
  case lockState
  case unlock
  case publicECDHKey
  case EIDIdentityKey
  case ADVSlotData
  case factoryReset
  case remainConnectable

  var UUID: CBUUID {
    switch self {
    case .capabilities:
      return CBUUID(string: "A3C87501-8ED3-4BDF-8A39-A01BEBEDE295")
    case .activeSlot:
      return CBUUID(string: "A3C87502-8ED3-4BDF-8A39-A01BEBEDE295")
    case .advertisingInterval:
      return CBUUID(string: "A3C87503-8ED3-4BDF-8A39-A01BEBEDE295")
    case .radioTxPower:
      return CBUUID(string: "A3C87504-8ED3-4BDF-8A39-A01BEBEDE295")
    case .advertisedTxPower:
      return CBUUID(string: "A3C87505-8ED3-4BDF-8A39-A01BEBEDE295")
    case .lockState:
      return CBUUID(string: "A3C87506-8ED3-4BDF-8A39-A01BEBEDE295")
    case .unlock:
      return CBUUID(string: "A3C87507-8ED3-4BDF-8A39-A01BEBEDE295")
    case .publicECDHKey:
      return CBUUID(string: "A3C87508-8ED3-4BDF-8A39-A01BEBEDE295")
    case .EIDIdentityKey:
      return CBUUID(string: "A3C87509-8ED3-4BDF-8A39-A01BEBEDE295")
    case .ADVSlotData:
      return CBUUID(string: "A3C8750A-8ED3-4BDF-8A39-A01BEBEDE295")
    case .factoryReset:
      return CBUUID(string: "A3C8750B-8ED3-4BDF-8A39-A01BEBEDE295")
    case .remainConnectable:
      return CBUUID(string: "A3C8750C-8ED3-4BDF-8A39-A01BEBEDE295")
    }
  }
}

let GATTCharacteristics: [CBUUID] = [CharacteristicID.capabilities.UUID,
                                     CharacteristicID.activeSlot.UUID,
                                     CharacteristicID.advertisingInterval.UUID,
                                     CharacteristicID.radioTxPower.UUID,
                                     CharacteristicID.advertisedTxPower.UUID,
                                     CharacteristicID.lockState.UUID,
                                     CharacteristicID.unlock.UUID,
                                     CharacteristicID.publicECDHKey.UUID,
                                     CharacteristicID.EIDIdentityKey.UUID,
                                     CharacteristicID.ADVSlotData.UUID,
                                     CharacteristicID.factoryReset.UUID,
                                     CharacteristicID.remainConnectable.UUID]

class GATTOperations: NSObject, CBPeripheralDelegate {
  var peripheral: CBPeripheral!
  let GATTServiceID = "A3C87500-8ED3-4BDF-8A39-A01BEBEDE295"
  var gotUnlockChallenge: Bool
  var unlockChallenge: NSData?
  var beaconOperationsCallback: ((operationState: OperationState) -> Void)?
  var lockStateCallback: ((lockState: LockState) -> Void)?
  var updateLockStateCallback: ((lockState: LockState) -> Void)?
  var remainConnectableCallback: (() -> Void)?
  var factoryResetCallback: (() -> Void)?
  var userPasskey: String?
  var didAttemptUnlocking = false

  init(peripheral: CBPeripheral) {
    gotUnlockChallenge = false
    super.init()
    self.peripheral = peripheral
  }

  func didUpdateOperationState(operationState: OperationState) {
    switch operationState {
    case OperationState.DidDiscoverServices:
      discoverCharacteristics()
    default:
      if let callback = beaconOperationsCallback {
        callback(operationState: operationState)
      }
    }
  }

  func discoverServices(callback: (operationState: OperationState) -> Void) {
    peripheral.delegate = self
    beaconOperationsCallback = callback
    if let _ = peripheral.services {
      /// services were already discovered
      discoverCharacteristics()
    } else {
      let service = CBUUID(string: GATTServiceID)
      peripheral.discoverServices([service])
    }
  }

  func peripheral(peripheral: CBPeripheral,
                  didDiscoverServices error: NSError?) {
    if let identifiedError = error {
      NSLog("Error discovering services: \(identifiedError)")
      didUpdateOperationState(OperationState.DiscoveringServicesError)
    } else {
      didUpdateOperationState(OperationState.DidDiscoverServices)
    }
  }

  func discoverCharacteristics() {
    if let services = peripheral.services {
      for service in services {
        if service.UUID == CBUUID(string: GATTServiceID) {
          peripheral.discoverCharacteristics(GATTCharacteristics, forService: service)
          return
        }
      }
      NSLog("The beacon does not implement the GATT Configuration Service.")
      didUpdateOperationState(OperationState.NotImplementingGATTConfigService)
    }
  }

  func peripheral(peripheral: CBPeripheral,
                  didDiscoverCharacteristicsForService service: CBService,
                                                       error: NSError?) {
    if let identifiedError = error {
      NSLog("Error discovering characteristics: \(identifiedError)")
      didUpdateOperationState(OperationState.DiscoveringCharacteristicsError)
    } else {
      didUpdateOperationState(OperationState.DidDiscoverCharacteristics)
    }
  }

  func peripheral(peripheral: CBPeripheral,
                  didWriteValueForCharacteristic characteristic: CBCharacteristic,
                                                 error: NSError?) {

    print("did write value for characteristic \(characteristic.UUID)")
    if error != nil {
      print("there was an error while writing to the characteristis \(characteristic)")
    }
    if characteristic.UUID == CharacteristicID.unlock.UUID {
      if let callback = lockStateCallback {
        checkLockState(nil, lockStateCallback: callback)
      }
    } else if characteristic.UUID == CharacteristicID.lockState.UUID {
      if let callback = updateLockStateCallback {
        lockStateCallback = callback
        getUnlockChallenge()
      }
    } else if characteristic.UUID == CharacteristicID.factoryReset.UUID {
      if let callback = factoryResetCallback {
        callback()
      }
    } else if characteristic.UUID == CharacteristicID.remainConnectable.UUID {
      if let callback = remainConnectableCallback {
        callback()
      }
    }
  }

  func peripheral(peripheral: CBPeripheral,
                  didUpdateValueForCharacteristic characteristic: CBCharacteristic,
                                                  error: NSError?) {
    print("did update value for characteristic \(characteristic.UUID)")
    if let identifiedError = error {
      NSLog("Error reading characteristic: \(identifiedError)")
    } else {
      if characteristic.UUID == CharacteristicID.lockState.UUID {
        parseLockStateValue()
      } else if characteristic.UUID == CharacteristicID.unlock.UUID {
        unlockBeacon()
      }
    }
  }

  func parseLockStateValue() {
    if let
      characteristic = findCharacteristicByID(CharacteristicID.lockState.UUID),
      value = characteristic.value {
      if value.length == 1 {
        var lockState: UInt8 = 0
        value.getBytes(&lockState, length: 1)
        if lockState == LockState.Locked.rawValue {
          NSLog("The beacon is locked :( .")
          didUpdateLockState(LockState.Locked)
        } else {
          NSLog("The beacon is unlocked!")
          didUpdateLockState(LockState.Unlocked)
        }
      }
    }
  }

  func didUpdateLockState(lockState: LockState) {
    if let callback = lockStateCallback {
      switch lockState {
      case LockState.Locked:
        if userPasskey != nil {
          if didAttemptUnlocking {
            callback(lockState: LockState.Locked)
          } else {
            getUnlockChallenge()
          }
        } else {
          callback(lockState: LockState.Locked)
        }
      case LockState.Unlocked:
        callback(lockState: LockState.Unlocked)
      default:
        callback(lockState: LockState.Unknown)
      }
    }
  }

  func checkLockState(passkey: String?,
                      lockStateCallback: (lockState: LockState) -> Void) {
    self.lockStateCallback = lockStateCallback
    if passkey != nil {
      userPasskey = passkey
    }
    if let lockStateCharacteristic = findCharacteristicByID(CharacteristicID.lockState.UUID) {
      peripheral.readValueForCharacteristic(lockStateCharacteristic)
    }
  }

  func findCharacteristicByID(characteristicID: CBUUID) -> CBCharacteristic? {
    if let services = peripheral.services {
      for service in services {
        for characteristic in service.characteristics! {
          if characteristic.UUID == characteristicID {
            return characteristic
          }
        }
      }
    }
    return nil
  }

  func beginUnlockingBeacon(passKey: String,
                            lockStateCallback: (lockState: LockState) -> Void) {
    didAttemptUnlocking = false
    checkLockState(passKey, lockStateCallback: lockStateCallback)

  }

  func getUnlockChallenge() {
    if let characteristic = findCharacteristicByID(CharacteristicID.unlock.UUID) {
      peripheral.readValueForCharacteristic(characteristic)
    }
  }

  func unlockBeacon() {
    if let
      passKey = userPasskey,
      characteristic = findCharacteristicByID(CharacteristicID.unlock.UUID),
      unlockChallenge = characteristic.value {
      let token: NSData? = AESEncrypt(unlockChallenge, key: passKey)
      // erase old password
      userPasskey = nil
      didAttemptUnlocking = true
      if let unlockToken = token {
        peripheral.writeValue(unlockToken,
                              forCharacteristic: characteristic,
                              type: CBCharacteristicWriteType.WithResponse)
      }
    }
  }

  func AESEncrypt(data: NSData, key: String?) -> NSData? {
    if let passKey = key {
      let keyBytes = StringUtils.transformStringToByteArray(passKey)
      let cryptData = NSMutableData(length: Int(data.length) + kCCBlockSizeAES128)!
      let operation: CCOperation = UInt32(kCCEncrypt)
      let algoritm:  CCAlgorithm = UInt32(kCCAlgorithmAES128)
      let options:   CCOptions   = UInt32(kCCOptionECBMode)
      var numBytesEncrypted :size_t = 0
      let cryptStatus = CCCrypt(operation,
                                algoritm,
                                options,
                                keyBytes,
                                keyBytes.count,
                                nil,
                                data.bytes,
                                data.length,
                                cryptData.mutableBytes,
                                cryptData.length,
                                &numBytesEncrypted)

      if UInt32(cryptStatus) == UInt32(kCCSuccess) {
        cryptData.length = Int(numBytesEncrypted)
        return cryptData as NSData
      } else {
        NSLog("Error: \(cryptStatus)")
      }
    }
    return nil
  }

  func writeNewLockCode(encryptedKey: NSData) {
    let value = NSMutableData(bytes: [0x00 as UInt8], length: 1)
    value.appendData(encryptedKey)
    if let characteristic = findCharacteristicByID(CharacteristicID.lockState.UUID) {
      peripheral.writeValue(value, forCharacteristic: characteristic, type: .WithResponse)
    }
  }

  func changeLockCode(oldCode: String, newCode: String, callback: (lockState: LockState) -> Void) {
    peripheral.delegate = self
    updateLockStateCallback = callback

    let newCodeBytes = StringUtils.transformStringToByteArray(newCode)
    let newCodeData = NSData(bytes: newCodeBytes, length: newCodeBytes.count)
    let encryptedKey = AESEncrypt(newCodeData, key: oldCode)
    userPasskey = newCode
    if let key = encryptedKey {
      writeNewLockCode(key)
    }
  }

  func factoryReset(callback: () -> Void) {
    factoryResetCallback = callback
    peripheral.delegate = self
    if let characteristic = findCharacteristicByID(CharacteristicID.factoryReset.UUID) {
      let value = NSData(bytes: [0x0B as UInt8], length: 1)
      peripheral.writeValue(value, forCharacteristic: characteristic, type: .WithResponse)
    }
  }

  func changeRemainConnectableState(on: Bool, callback: () -> Void) {
    remainConnectableCallback = callback
    peripheral.delegate = self
    if let characteristic = findCharacteristicByID(CharacteristicID.remainConnectable.UUID) {
      var value: UInt8 = 0
      if on {
        value = 1
      }
      let data = NSData(bytes: [value], length: 1)
      peripheral.writeValue(data, forCharacteristic: characteristic, type: .WithResponse)
    }
  }
}
