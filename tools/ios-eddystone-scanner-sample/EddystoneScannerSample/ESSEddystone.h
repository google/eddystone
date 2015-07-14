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

#import <Foundation/Foundation.h>
#import <CoreBluetooth/CoreBluetooth.h>

typedef NS_ENUM(NSUInteger, ESSBeaconType) {
  kESSBeaconTypeEddystone = 1,
};

typedef NS_ENUM(NSUInteger, ESSFrameType) {
  kESSEddystoneUnknownFrameType = 0,
  kESSEddystoneUIDFrameType = 1,
  kESSEddystoneTelemetryFrameType,
};

/**
 *=-----------------------------------------------------------------------------------------------=
 * ESSBeaconID
 *=-----------------------------------------------------------------------------------------------=
 */
@interface ESSBeaconID : NSObject <NSCopying>

/**
 * Currently there's only the Eddystone format, but we'd like to leave the door open to other
 * possibilities, so let's have a beacon type here in the info.
 */
@property(nonatomic, assign, readonly) ESSBeaconType beaconType;

/**
 * The raw beaconID data.
 */
@property(nonatomic, copy, readonly) NSData *beaconID;

@end


/**
 *=-----------------------------------------------------------------------------------------------=
 * ESSBeaconInfo
 *=-----------------------------------------------------------------------------------------------=
 */
@interface ESSBeaconInfo : NSObject

/**
 * The most recent RSSI we got for this sighting. Sometimes the OS cannot compute one reliably, so
 * this value can be null.
 */
@property(nonatomic, strong, readonly) NSNumber *RSSI;


/**
 * The beaconID for this Eddystone. All beacons have an ID.
 */
@property(nonatomic, strong, readonly) ESSBeaconID *beaconID;

/**
 * The telemetry that may or may not have been seen for this beacon. If it's set, the contents of
 * it aren't terribly relevant to us, in general. See the Eddystone spec for more information
 * if you're really interested in the exact details.
 */
@property(nonatomic, copy, readonly) NSData *telemetry;

/**
 * Transmission power reported by beacon. This is in dB.
 */
@property(nonatomic, strong, readonly) NSNumber *txPower;


/**
 * The scanner has seen a frame for an Eddystone. We'll need to know what type of Eddystone frame
 * it is, as there are a few types.
 */
+ (ESSFrameType)frameTypeForFrame:(NSDictionary *)advFrameList;

/**
 * Given some advertisement data that we have already verified is a TLM frame (using
 * frameTypeForFrame:), return the actual telemetry data for that frame.
 */
+ (NSData *)telemetryDataForFrame:(NSDictionary *)advFrameList;

/**
 * Given the service data for a frame we know to be a UID frame, an RSSI sighting,
 * and -- optionally -- telemetry data (if we've seen it), create a new ESSBeaconInfo object to
 * represent this Eddystone
 */
+ (instancetype)beaconInfoForUIDFrameData:(NSData *)UIDFrameData
                                telemetry:(NSData *)telemetry
                                     RSSI:(NSNumber *)initialRSSI;

/**
 * Convenience method to save everybody from creating these things all the time.
 */
+ (CBUUID *)eddystoneServiceID;

+ (ESSBeaconInfo *)testBeaconFromBeaconIDString:(NSString *)beaconID;

@end
