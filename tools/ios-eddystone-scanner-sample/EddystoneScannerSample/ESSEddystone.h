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
  kESSEddystoneURLFrameType = 2,
  kESSEddystoneTLMFrameType = 3,
};

/**
 *=-----------------------------------------------------------------------------------------------=
 * ESSBeaconUID
 *=-----------------------------------------------------------------------------------------------=
 */
@interface ESSBeaconUID : NSObject

/**
 * TX power of the UID frame.
 */
@property(nonatomic, assign) int txPower;

/**
 * 10-byte ID Namespace.
 */
@property(nonatomic, assign) NSString *idNamespace;

/**
 * 6-byte ID Instance.
 */
@property(nonatomic, assign) NSString *idInstance;

@end

/**
 *=-----------------------------------------------------------------------------------------------=
 * ESSBeaconURL
 *=-----------------------------------------------------------------------------------------------=
 */
@interface ESSBeaconURL : NSObject

/**
 * TX Power reading of the URL frame.
 */
@property(nonatomic, assign) int txPower;

/**
 * Friendly URL string.
 */
@property(nonatomic, assign) NSString *urlString;

@end

/**
 *=-----------------------------------------------------------------------------------------------=
 * ESSBeaconTLM
 *=-----------------------------------------------------------------------------------------------=
 */
@interface ESSBeaconTLM : NSObject

/**
 *  TLM version.
 */
@property(nonatomic, assign) unsigned int version;

/**
 * Battery voltage 1 mV/bit.
 */
@property(nonatomic, assign) unsigned int voltage;

/**
 * Beacon temperature.
 */
@property(nonatomic, assign) double temperature;

/**
 * Advertising PDU count.
 */
@property(nonatomic, assign) unsigned int advCount;

/**
 * Time since power-on or reboot.
 */
@property(nonatomic, assign) unsigned int uptime;

@end

/**
 *=-----------------------------------------------------------------------------------------------=
 * ESSBeaconInfo
 *=-----------------------------------------------------------------------------------------------=
 */
@interface ESSBeaconInfo : NSObject

/**
 * The unique perhiperal identifier for the beacon device.
 */
@property(nonatomic, copy) NSUUID *identifier;

/**
 * The most recent RSSI we got for this sighting. Sometimes the OS cannot compute one reliably, so
 * this value can be null.
 */
@property(nonatomic, strong) NSNumber *RSSI;

/**
 * The raw UID frame data that may or may not have been seen for the beacon.
 */
@property(nonatomic, copy) NSData *uidFrame;

/**
 * The raw URL frame data that may or may not have been seen for the beacon.
 */
@property(nonatomic, copy) NSData *urlFrame;

/**
 * The raw TLM frame data that may or may not have been seen for the beacon.
 */
@property(nonatomic, copy) NSData *tlmFrame;

/**
 * Initialise the beacon info with predefined raw data.
 */
- (instancetype)initWithFrameData:(NSNumber *)RSSI
                              uid:(NSData *)uid
                              url:(NSData *)url
                              tlm:(NSData *)tlm;

/**
 * The scanner has seen a frame for an Eddystone. We'll need to know what type of Eddystone frame
 * it is, as there are a few types.
 */
+ (ESSFrameType)frameTypeForFrame:(NSDictionary *)advFrameList;

/**
 * Convenience method to save everybody from creating these things all the time.
 */
+ (CBUUID *)eddystoneServiceID;

/**
 * Calculate the friendly encoded data for a UID frame. May be null if the UID data is not available.
 */
+ (ESSBeaconUID *)dataForUIDFrame:(NSData *)rawFrameData;

/**
 * Calculate the friendly encoded data for a URL frame. May be null if the UID data is not available.
 */
+ (ESSBeaconURL *)dataForURLFrame:(NSData *)rawFrameData;

/**
 * Calculate the friendly encoded data for a TLM frame. May be null if the UID data is not available.
 */
+ (ESSBeaconTLM *)dataForTLMFrame:(NSData *)rawFrameData;

@end