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

/// We want to wait no more than 10 seconds to wait for the HTTP request to give us an answer
let kRequestTimeout: Double = 10
let kGetEIDParamsServer = "https://proximitybeacon.googleapis.com/v1beta1/eidparams"
let kRegisterBeaconServer = "https://proximitybeacon.googleapis.com/v1beta1/beacons:register"
let kInstanceLength = 6
let kNamespaceLength = 10

///
/// The class implements all the functions that are necessary for configuring the EID and
/// cannot be included in the regular flow of other frame types configurations,
/// especially the HTTP requests.
///
class EIDConfiguration {
  ///
  /// Callback to the general configuration class saying whether or not the registration was
  /// successful.
  ///
  var EIDRegistrationCallback: ((didRegister: Bool) -> Void)?

  ///
  /// Project ID is needed whenever the user intends to select a new project ID in order to perform
  /// any EID registrations or beacons scannings (if the user wants to know if any of the beacons
  /// in the nearby area are related to the currently selected project).
  ///
  static var projectID: String?

  func parseEIDParams(data: NSData) -> (String, UInt8, UInt8)? {
    do {
      let json = try NSJSONSerialization.JSONObjectWithData(data, options: .AllowFragments)
      if let
        servicePublicKey = json["serviceEcdhPublicKey"] as? String,
        minRotationExponent = json["minRotationPeriodExponent"] as? NSNumber,
        maxRotationExponent = json["maxRotationPeriodExponent"] as? NSNumber {
        return (servicePublicKey, UInt8(Int(minRotationExponent)), UInt8(Int(maxRotationExponent)))
      }
    } catch {
      print("error serializing JSON: \(error)")
    }
    return nil
  }

  func httpRegisterRequest(beacon: Dictionary <String, AnyObject>) {
    let bearer = GIDSignIn.sharedInstance().currentUser.authentication.accessToken
    let bearerHeader = "Bearer \(bearer)"
    let server = "\(kRegisterBeaconServer)?projectId=\(EIDConfiguration.projectID!)"
    let url = NSURL(string: server)
    let httpHeaders = ["Authorization" : bearerHeader,
                       "Content-Type" : "application/json",
                       "Accept" : "application/json"]
    var body: NSData
    do {
      body = try NSJSONSerialization.dataWithJSONObject(beacon, options: .PrettyPrinted)
    } catch {
      print(error)
      return
    }
    if let serverURL = url {
      HTTPRequest.makeHTTPRequest(serverURL,
                                  method: "POST",
                                  postBody: body,
                                  requestHeaders: httpHeaders) { statusCode, content, error in
                                    if let callback = self.EIDRegistrationCallback {
                                      if statusCode == 200 {
                                        callback(didRegister: true)
                                      } else {
                                        callback(didRegister: false)
                                      }
                                    }
      }
    }
  }

  ///
  /// We make a request to the server to get the service's public ECDH Key and the minimum
  /// and maximum accepted values for the exponent.
  ///
  func getEIDParams(callback: (key: String?, minExponent:UInt8?, maxExponent: UInt8?) -> Void) {
    let bearer = GIDSignIn.sharedInstance().currentUser.authentication.accessToken
    let bearerHeader = "Bearer \(bearer)"
    let server = kGetEIDParamsServer
    let url = NSURL(string: server)
    let httpHeaders = ["Authorization" : bearerHeader,
                       "Accept" : "application/json"]
    if let serverURL = url {
      HTTPRequest.makeHTTPRequest(serverURL,
                                  method: "GET",
                                  postBody: nil,
                                  requestHeaders: httpHeaders) { statusCode, content, error in
                                    if statusCode == 200 {
                                      let (serviceKey, minExp, maxExp) =
                                        self.parseEIDParams(content!)!
                                      callback(key: serviceKey,
                                               minExponent: minExp,
                                               maxExponent: maxExp)
                                    } else {
                                      callback(key: nil, minExponent: nil, maxExponent: nil)
                                    }
      }
    }
  }

  func registerBeacon(registrationData: EIDRegistrationData,
                      callback: (didRegister: Bool) -> Void) {
    EIDRegistrationCallback = callback

    ///
    /// When registering a beacon that broadcasts Eddystone-EID, we must generate a 16-byte code
    /// that identifies the beacon; it will only be used for administering the beacon.
    ///
    let IDData = generateEddystone16ByteStableID()
    let IDString = IDData.base64EncodedStringWithOptions(NSDataBase64EncodingOptions(rawValue: 0))
    let advertisedID = ["type" : "EDDYSTONE",
                        "id" : IDString]
    let eidRegistration = ["beaconEcdhPublicKey" : registrationData.beaconEcdhPublicKey!,
                           "serviceEcdhPublicKey" : registrationData.serviceEcdhPublicKey!,
                           "rotationPeriodExponent" : registrationData.rotationPeriodExponent!,
                           "initialClockValue" : registrationData.initialClockValue!,
                           "initialEid" : registrationData.initialEid!]
    let beacon = ["advertisedId" : advertisedID,
                  "status" : "ACTIVE",
                  "ephemeralIdRegistration" : eidRegistration]
    httpRegisterRequest(beacon as! Dictionary<String, AnyObject>)
  }

  /// The namespace is a 10-byte code that is the first part of a MD5 hash of our projectID.
  func stableIDNamespace() -> NSData {
    let projectID: String = EIDConfiguration.projectID!
    let digest = StringUtils.md5(projectID)
    return digest.subdataWithRange(NSMakeRange(0, kNamespaceLength))
  }

  ///
  /// The "stable" Eddystone identifier is composed of:
  ///    - 10 bytes of namespace, which is taken from the MD5 hash of the project ID
  ///    -  6 bytes of instance, which is just a random number.
  ///
  func generateEddystone16ByteStableID() -> NSData {
    let namespace = stableIDNamespace()
    var instance = [UInt8](count: kInstanceLength, repeatedValue: 0)
    arc4random_buf(&instance, kInstanceLength)
    let instanceData = NSData(bytes: instance, length: kInstanceLength)
    let beaconID = NSMutableData()
    beaconID.appendData(namespace)
    beaconID.appendData(instanceData)
    return beaconID
  }
}
