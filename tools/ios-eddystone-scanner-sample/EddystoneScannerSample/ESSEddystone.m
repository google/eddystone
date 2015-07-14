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
static const uint8_t kEddystoneTLMFrameTypeID = 0x20;

// Note that for these Eddystone structures, the endianness of the individual fields is big-endian,
// so you'll want to translate back to host format when necessary.
// Note that in the Eddystone spec, the beaconID (UID) is divided into 2 fields, a 10 byte namespace
// and a 6 byte instance id. However, since we ALWAYS use these in combination as a 16 byte
// beaconID, we'll have our structure here reflect that.
typedef struct __attribute__((packed)) {
  uint8_t frameType;
  int8_t  txPower;
  uint8_t zipBeaconID[16];
} ESSEddystoneUIDFrameFields;

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
+ (ESSFrameType)frameTypeForFrame:(NSDictionary *)advFrameList {
  NSData *frameData = advFrameList[[self eddystoneServiceID]];

  // It's an Eddystone ADV frame. Now check if it's a UID (ID) or TLM (telemetry) frame.
  if (frameData) {
    uint8_t frameType;
    if ([frameData length] > 1) {
      frameType = ((uint8_t *)[frameData bytes])[0];

      if (frameType == kEddystoneUIDFrameTypeID) {
        return kESSEddystoneUIDFrameType;
      } else if (frameType == kEddystoneTLMFrameTypeID) {
        return kESSEddystoneTelemetryFrameType;
      }
    }
  }

  return kESSEddystoneUnknownFrameType;
}

+ (NSData *)telemetryDataForFrame:(NSDictionary *)advFrameList {
  // NOTE: We assume that you've already called [ESSBeaconInfo frameTypeForFrame] to confirm that
  //       this actually IS a telemetry frame.
  NSAssert([ESSBeaconInfo frameTypeForFrame:advFrameList] == kESSEddystoneTelemetryFrameType,
           @"This should be a TLM frame, but it's not. Whooops");
  return advFrameList[[self eddystoneServiceID]];
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
  // Make sure this frame has the correct frame type identifier
  uint8_t frameType;
  [UIDFrameData getBytes:&frameType length:1];
  if (frameType != kEddystoneUIDFrameTypeID) {
    return nil;
  }

  ESSEddystoneUIDFrameFields uidFrame;

  if ([UIDFrameData length] != sizeof(ESSEddystoneUIDFrameFields)) {
    return nil;
  }
  [UIDFrameData getBytes:&uidFrame length:sizeof(ESSEddystoneUIDFrameFields)];

  NSData *beaconIDData = [NSData dataWithBytes:&uidFrame.zipBeaconID
                                        length:sizeof(uidFrame.zipBeaconID)];
  ESSBeaconID *beaconID = [[ESSBeaconID alloc] initWithType:kESSBeaconTypeEddystone
                                                   beaconID:beaconIDData];
  if (beaconID == nil) {
    return nil;
  }

  return [[ESSBeaconInfo alloc] initWithBeaconID:beaconID
                                         txPower:@(uidFrame.txPower)
                                            RSSI:RSSI
                                       telemetry:telemetry];
}

- (NSString *)description {
  return [NSString stringWithFormat:@"Eddystone, id: %@, RSSI: %@, txPower: %@",
      _beaconID, _RSSI, _txPower];
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

@end
