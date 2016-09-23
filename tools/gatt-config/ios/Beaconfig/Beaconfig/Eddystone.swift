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
import CoreBluetooth

///
/// BeaconID
///
/// Uniquely identifies an Eddystone compliant beacon.
///
class BeaconID : NSObject {

  enum BeaconType {
    case Eddystone              // 10 bytes namespace + 6 bytes instance = 16 byte ID
    case EddystoneEID           // 8 byte ID
  }

  let beaconType: BeaconType

  ///
  /// The raw beaconID data. This is typically printed out in hex format.
  ///
  let beaconID: [UInt8]

  private init(beaconType: BeaconType!, beaconID: [UInt8]) {
    self.beaconID = beaconID
    self.beaconType = beaconType
  }

  override var description: String {
    if self.beaconType == BeaconType.Eddystone || self.beaconType == BeaconType.EddystoneEID {
      let hexid = hexBeaconID(self.beaconID)
      return "\(hexid)"
    } else {
      return "BeaconID with invalid type (\(beaconType))"
    }
  }

  private func hexBeaconID(beaconID: [UInt8]) -> String {
    var retval = ""
    for byte in beaconID {
      var s = String(byte, radix:16, uppercase: false)
      if s.characters.count == 1 {
        s = "0" + s
      }
      retval += s
    }
    return retval
  }

}

func ==(lhs: BeaconID, rhs: BeaconID) -> Bool {
  if lhs == rhs {
    return true;
  } else if lhs.beaconType == rhs.beaconType
    && rhs.beaconID == rhs.beaconID {
    return true;
  }

  return false;
}

///
/// BeaconInfo
///
/// Contains information fully describing a beacon, including its beaconID, transmission power,
/// RSSI, and possibly telemetry information.
///
class BeaconInfo : NSObject {

  static let EddystoneUIDFrameTypeID: UInt8 = 0x00
  static let EddystoneURLFrameTypeID: UInt8 = 0x10
  static let EddystoneTLMFrameTypeID: UInt8 = 0x20
  static let EddystoneEIDFrameTypeID: UInt8 = 0x30
  static let GATTServiceID = "A3C87500-8ED3-4BDF-8A39-A01BEBEDE295"

  enum EddystoneFrameType {
    case UnknownFrameType
    case UIDFrameType
    case URLFrameType
    case TelemetryFrameType
    case EIDFrameType
    case EddystoneGATTServiceFrameType
    case NotSetFrameType

    var description: String {
      switch self {
      case .UnknownFrameType:
        return "Unknown Frame Type"
      case .UIDFrameType:
        return "UID Frame"
      case .URLFrameType:
        return "URL Frame"
      case .TelemetryFrameType:
        return "TLM Frame"
      case .EIDFrameType:
        return "EID Frame"
      case .EddystoneGATTServiceFrameType:
        return "EddystoneGATTService Frame"
      case .NotSetFrameType:
        return "Frame type not set"
      }
    }
  }

  let beaconID: BeaconID
  let txPower: Int
  let RSSI: Int

  private init(beaconID: BeaconID, txPower: Int, RSSI: Int) {
    self.beaconID = beaconID
    self.txPower = txPower
    self.RSSI = RSSI
  }

  class func frameTypeForFrame(advertisementData: [NSObject : AnyObject])
    -> EddystoneFrameType {
      if let advertisementFrameList = advertisementData[CBAdvertisementDataServiceDataKey]
        as? [NSObject : AnyObject] {
        let uuid = CBUUID(string: "FEAA")
        if let frameData = advertisementFrameList[uuid] as? NSData {
          if frameData.length > 1 {
            let count = frameData.length
            var frameBytes = [UInt8](count: count, repeatedValue: 0)
            frameData.getBytes(&frameBytes, length: count)

            if frameBytes[0] == EddystoneUIDFrameTypeID {
              return EddystoneFrameType.UIDFrameType
            } else if frameBytes[0] == EddystoneTLMFrameTypeID {
              return EddystoneFrameType.TelemetryFrameType
            } else if frameBytes[0] == EddystoneEIDFrameTypeID {
              return EddystoneFrameType.EIDFrameType
            } else if frameBytes[0] == EddystoneURLFrameTypeID {
              return EddystoneFrameType.URLFrameType
            }
          } else {
            if let serviceUUIDs = advertisementData[CBAdvertisementDataServiceUUIDsKey] {
              for service in serviceUUIDs as! NSArray {
                let stringServiceUUID = String(service)
                if GATTServiceID.compare(stringServiceUUID, options: .CaseInsensitiveSearch) ==
                  NSComparisonResult.OrderedSame {
                  return EddystoneFrameType.EddystoneGATTServiceFrameType
                }
              }
            }
          }
        }
      }

      return EddystoneFrameType.UnknownFrameType
  }

  class func telemetryDataForFrame(advertisementFrameList: [NSObject : AnyObject]!) -> NSData? {
    return advertisementFrameList[CBUUID(string: "FEAA")] as? NSData
  }

  ///
  /// Unfortunately, this can't be a failable convenience initialiser just yet because of a "bug"
  /// in the Swift compiler — it can't tear-down partially initialised objects, so we'll have to
  /// wait until this gets fixed. For now, class method will do.
  ///
  class func beaconInfoForUIDFrameData(frameData: NSData, RSSI: Int)
    -> BeaconInfo? {
      if frameData.length > 1 {
        let count = frameData.length
        var frameBytes = [UInt8](count: count, repeatedValue: 0)
        frameData.getBytes(&frameBytes, length: count)

        if frameBytes[0] != EddystoneUIDFrameTypeID {
          NSLog("Unexpected non UID Frame passed to BeaconInfoForUIDFrameData.")
          return nil
        } else if frameBytes.count < 18 {
          NSLog("Frame Data for UID Frame unexpectedly truncated in BeaconInfoForUIDFrameData.")
        }

        let txPower = Int(Int8(bitPattern:frameBytes[1]))
        let beaconID: [UInt8] = Array(frameBytes[2..<18])
        let bid = BeaconID(beaconType: BeaconID.BeaconType.Eddystone, beaconID: beaconID)
        return BeaconInfo(beaconID: bid, txPower: txPower, RSSI: RSSI)
      }

      return nil
  }

  class func beaconInfoForEIDFrameData(frameData: NSData, RSSI: Int)
    -> BeaconInfo? {
      if frameData.length > 1 {
        let count = frameData.length
        var frameBytes = [UInt8](count: count, repeatedValue: 0)
        frameData.getBytes(&frameBytes, length: count)

        if frameBytes[0] != EddystoneEIDFrameTypeID {
          NSLog("Unexpected non EID Frame passed to BeaconInfoForEIDFrameData.")
          return nil
        } else if frameBytes.count < 10 {
          NSLog("Frame Data for EID Frame unexpectedly truncated in BeaconInfoForEIDFrameData.")
        }

        let txPower = Int(Int8(bitPattern:frameBytes[1]))
        let beaconID: [UInt8] = Array(frameBytes[2..<10])
        let bid = BeaconID(beaconType: BeaconID.BeaconType.EddystoneEID, beaconID: beaconID)
        return BeaconInfo(beaconID: bid, txPower: txPower, RSSI: RSSI)
      }

      return nil
  }

  class func parseURLFromFrame(frameData: NSData) -> NSURL? {
    if frameData.length > 0 {
      let count = frameData.length
      var frameBytes = [UInt8](count: count, repeatedValue: 0)
      frameData.getBytes(&frameBytes, length: count)
      let frameTypeLength = 2
      /// Check if the URL was corrupted.
      if count > frameTypeLength {
        if let URLPrefix = URLPrefixFromByte(frameBytes[2]) {
          var output = URLPrefix
          for i in 3..<frameBytes.count {
            if let encoded = encodedStringFromByte(frameBytes[i]) {
              output.appendContentsOf(encoded)
            }
          }
          return NSURL(string: output)
        }
      }
    }

    return nil
  }

  class func parseTLMFromFrame(frameData: NSData) -> (UInt16, Double, UInt32, UInt32)? {

    ///
    /// The unencrypted telemetry frame has the length of 14 bytes; we want to display information
    /// in the scan results list, but we're not interested for now in the encrypted telemetry.
    ///
    if frameData.length == 14 {
      var batteryVoltage: UInt16 = 0
      var beaconTemperature: UInt16 = 0
      var advPDUCount: UInt32 = 0
      var timeSinceReboot: UInt32 = 0

      frameData.getBytes(&batteryVoltage, range: NSMakeRange(2, 2))
      frameData.getBytes(&beaconTemperature, range: NSMakeRange(4, 2))
      frameData.getBytes(&advPDUCount, range: NSMakeRange(6, 4))
      frameData.getBytes(&timeSinceReboot, range: NSMakeRange(10, 4))
      batteryVoltage = CFSwapInt16BigToHost(batteryVoltage)
      beaconTemperature = CFSwapInt16BigToHost(beaconTemperature)
      /// Convert temperature from 8.8 fixed point notation.
      let realBeaconTemperature: Double = Double(beaconTemperature) / 256.0
      advPDUCount = CFSwapInt32BigToHost(advPDUCount)
      timeSinceReboot = CFSwapInt32BigToHost(timeSinceReboot)
      return (batteryVoltage, realBeaconTemperature, advPDUCount, timeSinceReboot)
    }
    return nil
  }

  override var description: String {
    switch self.beaconID.beaconType {
    case .Eddystone:
      return "Eddystone \(self.beaconID), txPower: \(self.txPower), RSSI: \(self.RSSI)"
    case .EddystoneEID:
      return "Eddystone EID \(self.beaconID), txPower: \(self.txPower), RSSI: \(self.RSSI)"
    }
  }

  class func URLPrefixFromByte(schemeID: UInt8) -> String? {
    switch schemeID {
    case 0x00:
      return "http://www."
    case 0x01:
      return "https://www."
    case 0x02:
      return "http://"
    case 0x03:
      return "https://"
    default:
      return nil
    }
  }

  class func encodedStringFromByte(charVal: UInt8) -> String? {
    switch charVal {
    case 0x00:
      return ".com/"
    case 0x01:
      return ".org/"
    case 0x02:
      return ".edu/"
    case 0x03:
      return ".net/"
    case 0x04:
      return ".info/"
    case 0x05:
      return ".biz/"
    case 0x06:
      return ".gov/"
    case 0x07:
      return ".com"
    case 0x08:
      return ".org"
    case 0x09:
      return ".edu"
    case 0x0a:
      return ".net"
    case 0x0b:
      return ".info"
    case 0x0c:
      return ".biz"
    case 0x0d:
      return ".gov"
    default:
      return String(data: NSData(bytes: [ charVal ] as [UInt8], length: 1),
                    encoding: NSUTF8StringEncoding)
    }
  }

  ///
  /// Swift if goofy when working with Strings because characters can't be accessed
  /// using integer indexes. The function tries to match the beginning of the string
  /// and encodes it, returning the byte and the truncated string. The function
  /// is called until the string has no more characters.
  ///
  class func byteFromEncodedString(urlText: String) -> (UInt8, String?) {
    var index = urlText.startIndex
    if urlText.hasPrefix("http://www.") {
      index = urlText.startIndex.advancedBy(11)
      return (0x00, urlText.substringFromIndex(index))
    } else if urlText.hasPrefix("https://www.") {
      index = urlText.startIndex.advancedBy(12)
      return (0x01, urlText.substringFromIndex(index))
    } else if urlText.hasPrefix("http://") {
      index = urlText.startIndex.advancedBy(7)
      return (0x02, urlText.substringFromIndex(index))
    } else if urlText.hasPrefix("https://") {
      index = urlText.startIndex.advancedBy(8)
      return (0x03, urlText.substringFromIndex(index))
    } else if urlText.hasPrefix(".com/") {
      index = urlText.startIndex.advancedBy(5)
      return (0x00, urlText.substringFromIndex(index))
    } else if urlText.hasPrefix(".org/") {
      index = urlText.startIndex.advancedBy(5)
      return (0x01, urlText.substringFromIndex(index))
    } else if urlText.hasPrefix(".edu/") {
      index = urlText.startIndex.advancedBy(5)
      return (0x02, urlText.substringFromIndex(index))
    } else if urlText.hasPrefix(".net/") {
      index = urlText.startIndex.advancedBy(5)
      return (0x03, urlText.substringFromIndex(index))
    } else if urlText.hasPrefix(".info/") {
      index = urlText.startIndex.advancedBy(6)
      return (0x04, urlText.substringFromIndex(index))
    } else if urlText.hasPrefix(".biz/") {
      index = urlText.startIndex.advancedBy(5)
      return (0x05, urlText.substringFromIndex(index))
    } else if urlText.hasPrefix(".gov/") {
      index = urlText.startIndex.advancedBy(5)
      return (0x06, urlText.substringFromIndex(index))
    } else if urlText.hasPrefix(".com") {
      index = urlText.startIndex.advancedBy(4)
      return (0x07, urlText.substringFromIndex(index))
    } else if urlText.hasPrefix(".org") {
      index = urlText.startIndex.advancedBy(4)
      return (0x08, urlText.substringFromIndex(index))
    } else if urlText.hasPrefix(".edu") {
      index = urlText.startIndex.advancedBy(4)
      return (0x09, urlText.substringFromIndex(index))
    } else if urlText.hasPrefix(".net") {
      index = urlText.startIndex.advancedBy(4)
      return (0x0a, urlText.substringFromIndex(index))
    } else if urlText.hasPrefix(".info") {
      index = urlText.startIndex.advancedBy(5)
      return (0x0b, urlText.substringFromIndex(index))
    } else if urlText.hasPrefix(".biz") {
      index = urlText.startIndex.advancedBy(4)
      return (0x0c, urlText.substringFromIndex(index))
    } else if urlText.hasPrefix(".gov") {
      index = urlText.startIndex.advancedBy(4)
      return (0x0d, urlText.substringFromIndex(index))
    }
    return ([UInt8](String(urlText.characters.first!).utf8)[0],
            String(urlText.characters.dropFirst()))
  }

  class func convertDeciseconds(deciseconds: UInt32) -> (Int, Int, Int) {
    let days: Int = Int(deciseconds) / (24 * 60 * 60 * 10)
    let hours: Int = Int(deciseconds) / (60 * 60 * 10) - days * 24
    let minutes: Int = Int(deciseconds / 600) - days * 24 * 60 - hours * 60
    return (days, hours, minutes)
  }
}
