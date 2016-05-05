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

#import "ESSEddystone.h"

#import <CoreBluetooth/CoreBluetooth.h>

/**
 * The Bluetooth Service ID for Eddystones.
 */
static NSString *const kEddystoneServiceID = @"FEAA";

/**
 * Eddystones can have different frame types. Within the frames, these are (some of) the possible
 * values. See the Eddystone spec for complete details.
 */
static const uint8_t kEddystoneUIDFrameTypeID = 0x00;
static const uint8_t kEddystoneURLFrameTypeID = 0x10;
static const uint8_t kEddystoneTLMFrameTypeID = 0x20;
static const uint8_t kEddystoneEIDFrameTypeID = 0x30;

// Note that for these Eddystone structures, the endianness of the individual fields is big-endian,
// so you'll want to translate back to host format when necessary.
// Note that in the Eddystone spec, the beaconID (UID) is divided into 2 fields, a 10 byte namespace
// and a 6 byte instance id. However, since we ALWAYS use these in combination as a 16 byte
// beaconID, we'll have our structure here reflect that.
typedef struct __attribute__((packed)) {
  uint8_t frameType;
  int8_t  txPower;
  uint8_t beaconID[16];
  uint8_t RFU[2];
} ESSEddystoneUIDFrameFields;

typedef struct __attribute__((packed)) {
  uint8_t frameType;
  int8_t  txPower;
  uint8_t beaconID[8];
} ESSEddystoneEIDFrameFields;

// Test equality, ensuring that nil is equal to itself.
static inline BOOL IsEqualOrBothNil(id a, id b) {
  return ((a == b) || (a && b && [a isEqual:b]));
}

/**
 *=-----------------------------------------------------------------------------------------------=
 * ESSBeaconID
 *=-----------------------------------------------------------------------------------------------=
 */
@implementation ESSBeaconID

/**
 * This property is orginally declared in a superclass (NSObject), so cannot be auto-synthesized.
 */
@synthesize hash = _hash;

- (instancetype)initWithType:(ESSBeaconType)beaconType
                    beaconID:(NSData *)beaconID {
  self = [super init];
  if (self) {
    _beaconType = beaconType;
    _beaconID = [beaconID copy];
    _hash = 31 * self.beaconType + [self.beaconID hash];
  }
  return self;
}

/**
 * So that whenever you convert this to a string, you get something useful.
 */
- (NSString *)description {
  if (self.beaconType == kESSBeaconTypeEddystone) {
    return [NSString stringWithFormat:@"ESSBeaconID: beaconID=%@", self.beaconID];
  } else if (self.beaconType == kESSBeaconTypeEddystoneEID) {
    return [NSString stringWithFormat:@"ESSBeaconID (EID): beaconID=%@", self.beaconID];
  } else {
    return [NSString stringWithFormat:@"ESSBeaconID with invalid type %lu",
            (unsigned long)self.beaconType];
  }
}

- (BOOL)isEqual:(id)object {
  if (object == self) {
    return YES;
  }
  if (!self
      || !object
      || !([self isKindOfClass:[object class]] || [object isKindOfClass:[self class]])) {
    return NO;
  }

  ESSBeaconID *other = (ESSBeaconID *)object;
  return ((self.beaconType == other.beaconType) &&
          IsEqualOrBothNil(self.beaconID, other.beaconID));
}

- (id)copyWithZone:(NSZone *)zone {
  // Immutable object: 'copy' by reusing the same instance.
  return self;
}

@end


/**
 *=-----------------------------------------------------------------------------------------------=
 * ESSBeaconInfo
 *=-----------------------------------------------------------------------------------------------=
 */
@implementation ESSBeaconInfo

/**
 * Given the advertising frames from CoreBluetooth for a device with the Eddystone Service ID,
 * figure out what type of frame it is.
 */
+ (ESSFrameType)frameTypeForFrame:(NSData *)frameData {
  // It's an Eddystone ADV frame. Now check if it's a UID (ID) or TLM (telemetry) frame.
  if (frameData) {
    uint8_t frameType;
    if ([frameData length] > 1) {
      frameType = ((uint8_t *)[frameData bytes])[0];
      switch (frameType) {
        case kEddystoneUIDFrameTypeID:
          return kESSEddystoneUIDFrameType;
        case kEddystoneURLFrameTypeID:
          return kESSEddystoneURLFrameType;
        case kEddystoneTLMFrameTypeID:
          return kESSEddystoneTelemetryFrameType;
        case kEddystoneEIDFrameTypeID:
          return kESSEddystoneEIDFrameType;
      }
    }
  }

  return kESSEddystoneUnknownFrameType;
}

+ (NSURL *)parseURLFromFrameData:(NSData *)URLFrameData {
  NSAssert([ESSBeaconInfo frameTypeForFrame:URLFrameData] == kESSEddystoneURLFrameType,
           @"This should be a URL frame, but it's not. Whooops");

  if (!(URLFrameData.length > 0)) {
    return nil;
  }
  
  unsigned char urlFrame[20];
  [URLFrameData getBytes:&urlFrame length:URLFrameData.length];
  
  NSString *urlScheme = [self getURLScheme:*(urlFrame+2)];
  
  NSString *urlString = urlScheme;
  for (int i = 0; i < URLFrameData.length - 3; i++) {
    urlString = [urlString stringByAppendingString:[self getEncodedString:*(urlFrame + i + 3)]];
  }
  
  return [NSURL URLWithString:urlString];
}

- (instancetype)initWithBeaconID:(ESSBeaconID *)beaconID
                         txPower:(NSNumber *)txPower
                            RSSI:(NSNumber *)RSSI
                       telemetry:(NSData *)telemetry {
  if ((self = [super init]) != nil) {
    _beaconID = beaconID;
    _txPower = txPower;
    _RSSI = RSSI;
    _telemetry = [telemetry copy];
  }

  return self;
}

+ (instancetype)beaconInfoForUIDFrameData:(NSData *)UIDFrameData
                                telemetry:(NSData *)telemetry
                                     RSSI:(NSNumber *)RSSI {
  NSAssert([ESSBeaconInfo frameTypeForFrame:UIDFrameData] == kESSEddystoneUIDFrameType,
           @"This should be a UID frame, but it's not. Whooops");

  // Make sure this frame has the correct frame type identifier
  uint8_t frameType;
  [UIDFrameData getBytes:&frameType length:1];
  if (frameType != kEddystoneUIDFrameTypeID) {
    return nil;
  }

  ESSEddystoneUIDFrameFields uidFrame;

  if ([UIDFrameData length] == sizeof(ESSEddystoneUIDFrameFields)
      || [UIDFrameData length] == sizeof(ESSEddystoneUIDFrameFields) - sizeof(uidFrame.RFU)) {

    [UIDFrameData getBytes:&uidFrame length:(sizeof(ESSEddystoneUIDFrameFields)
                                             - sizeof(uidFrame.RFU))];

    NSData *beaconIDData = [NSData dataWithBytes:&uidFrame.beaconID
                                          length:sizeof(uidFrame.beaconID)];

    ESSBeaconID *beaconID = [[ESSBeaconID alloc] initWithType:kESSBeaconTypeEddystone
                                                     beaconID:beaconIDData];
    if (beaconID == nil) {
      return nil;
    }

    return [[ESSBeaconInfo alloc] initWithBeaconID:beaconID
                                           txPower:@(uidFrame.txPower)
                                              RSSI:RSSI
                                         telemetry:telemetry];
  } else {
    return nil;
  }
}

+ (instancetype)beaconInfoForEIDFrameData:(NSData *)EIDFrameData
                                telemetry:(NSData *)telemetry
                                     RSSI:(NSNumber *)RSSI {
  NSAssert([ESSBeaconInfo frameTypeForFrame:EIDFrameData] == kESSEddystoneEIDFrameType,
           @"This should be an EID frame, but it's not. Whooops");

  // Make sure this frame has the correct frame type identifier
  uint8_t frameType;
  [EIDFrameData getBytes:&frameType length:1];
  if (frameType != kEddystoneEIDFrameTypeID) {
    return nil;
  }

  ESSEddystoneEIDFrameFields eidFrame;

  if ([EIDFrameData length] == sizeof(ESSEddystoneEIDFrameFields)) {
    [EIDFrameData getBytes:&eidFrame length:sizeof(ESSEddystoneEIDFrameFields)];
    NSData *beaconIDData = [NSData dataWithBytes:&eidFrame.beaconID
                                          length:sizeof(eidFrame.beaconID)];

    ESSBeaconID *beaconID = [[ESSBeaconID alloc] initWithType:kESSBeaconTypeEddystoneEID
                                                     beaconID:beaconIDData];
    if (beaconID == nil) {
      return nil;
    }

    return [[ESSBeaconInfo alloc] initWithBeaconID:beaconID
                                           txPower:@(eidFrame.txPower)
                                              RSSI:RSSI
                                         telemetry:telemetry];
  } else {
    return nil;
  }
}

- (NSString *)description {
  NSString *str = [NSString stringWithFormat:@"Eddystone, id: %@, RSSI: %@, txPower: %@",
                                             _beaconID, _RSSI, _txPower];
  return str;
}

+ (CBUUID *)eddystoneServiceID {
  static CBUUID *_singleton;
  static dispatch_once_t oncePredicate;

  dispatch_once(&oncePredicate, ^{
    _singleton = [CBUUID UUIDWithString:kEddystoneServiceID];
  });

  return _singleton;
}

+ (ESSBeaconInfo *)testBeaconFromBeaconIDString:(NSString *)beaconID {
  NSData *beaconIDData = [ESSBeaconInfo hexStringToNSData:beaconID];
  ESSBeaconID *beaconIDObj = [[ESSBeaconID alloc] initWithType:kESSBeaconTypeEddystone
                                                      beaconID:beaconIDData];

  return [[ESSBeaconInfo alloc] initWithBeaconID:beaconIDObj
                                         txPower:@(-20)
                                            RSSI:@(-100)
                                       telemetry:nil];
}

+ (NSData *)hexStringToNSData:(NSString *)hexString {
  NSMutableData *data = [[NSMutableData alloc] init];
  unsigned char whole_byte;
  char byte_chars[3] = {'\0','\0','\0'};

  int i;
  for (i = 0; i < [hexString length]/2; i++) {
    byte_chars[0] = [hexString characterAtIndex:i * 2];
    byte_chars[1] = [hexString characterAtIndex:i * 2 + 1];
    whole_byte = strtol(byte_chars, NULL, 16);
    [data appendBytes:&whole_byte length:1];
  }

  return data;
}

+ (NSString *)getURLScheme:(char)hexChar {
  switch (hexChar) {
    case 0x00:
      return @"http://www.";
    case 0x01:
      return @"https://www.";
    case 0x02:
      return @"http://";
    case 0x03:
      return @"https://";
    default:
      return nil;
  }
}

+ (NSString *)getEncodedString:(char)hexChar {
  switch (hexChar) {
    case 0x00:
      return @".com/";
    case 0x01:
      return @".org/";
    case 0x02:
      return @".edu/";
    case 0x03:
      return @".net/";
    case 0x04:
      return @".info/";
    case 0x05:
      return @".biz/";
    case 0x06:
      return @".gov/";
    case 0x07:
      return @".com";
    case 0x08:
      return @".org";
    case 0x09:
      return @".edu";
    case 0x0a:
      return @".net";
    case 0x0b:
      return @".info";
    case 0x0c:
      return @".biz";
    case 0x0d:
      return @".gov";
    default:
      return [NSString stringWithFormat:@"%c", hexChar];
  }
}

@end
