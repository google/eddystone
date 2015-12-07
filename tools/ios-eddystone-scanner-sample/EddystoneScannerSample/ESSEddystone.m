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
 * 41dBm is the signal loss that occurs over 1 meter.
 */
static int const txPowerAt1Meter = -41;

/**
 * Eddystones can have different frame types. Within the frames, these are (some of) the possible
 * values. See the Eddystone spec for complete details.
 */
static const uint8_t kEddystoneUIDFrameTypeID = 0x00;
static const uint8_t kEddystoneURLFrameTypeID = 0x10;
static const uint8_t kEddystoneTLMFrameTypeID = 0x20;

/**
 *=-----------------------------------------------------------------------------------------------=
 * ESSBeaconUID
 *=-----------------------------------------------------------------------------------------------=
 */
@implementation ESSBeaconUID : NSObject

/**
 * So that whenever you convert this to a string, you get something useful.
 */
- (NSString *)description {
  return [NSString stringWithFormat:@"ESSBeaconUID: txPower=%d idNamespace=%@ idInstance=%@", self.txPower, self.idNamespace, self.idInstance];
}

@end

/**
 *=-----------------------------------------------------------------------------------------------=
 * ESSBeaconURL
 *=-----------------------------------------------------------------------------------------------=
 */
@implementation ESSBeaconURL : NSObject

/**
 * So that whenever you convert this to a string, you get something useful.
 */
- (NSString *)description {
  return [NSString stringWithFormat:@"ESSBeaconURL: txPower=%d urlString=%@", self.txPower, self.urlString];
}

@end

/**
 *=-----------------------------------------------------------------------------------------------=
 * ESSBeaconTLM
 *=-----------------------------------------------------------------------------------------------=
 */
@implementation ESSBeaconTLM : NSObject

/**
 * So that whenever you convert this to a string, you get something useful.
 */
- (NSString *)description {
  return [NSString stringWithFormat:@"ESSBeaconTLM: version=%u voltage=%d temperature=%f advCount=%d uptime=%d", self.version, self.voltage, self.temperature, self.advCount, self.uptime];
}

@end

/**
 *=-----------------------------------------------------------------------------------------------=
 * ESSBeaconInfo
 *=-----------------------------------------------------------------------------------------------=
 */
@implementation ESSBeaconInfo

/**
 * Initialise the beacon info with predefined raw data.
 */
- (instancetype)initWithFrameData:(NSNumber *)RSSI
                              uid:(NSData *)uid
                              url:(NSData *)url
                              tlm:(NSData *)tlm {
  if ((self = [super init]) != nil) {
    _RSSI = RSSI;
    _uidFrame = [uid copy];
    _urlFrame = [url copy];
    _tlmFrame = [tlm copy];
  }
  
  return self;
}

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
      } else if (frameType == kEddystoneURLFrameTypeID) {
        return kESSEddystoneURLFrameType;
      } else if (frameType == kEddystoneTLMFrameTypeID) {
        return kESSEddystoneTLMFrameType;
      }
    }
  }

  return kESSEddystoneUnknownFrameType;
}

/**
 * So that whenever you convert this to a string, you get something useful.
 */
- (NSString *)description {
  return [NSString stringWithFormat:@"Eddystone, RSSI: %@, \nUID: %@, \nURL: %@, \nTLM: %@",
          _RSSI,
          [ESSBeaconInfo dataForUIDFrame:_uidFrame],
          [ESSBeaconInfo dataForURLFrame:_urlFrame],
          [ESSBeaconInfo dataForTLMFrame:_tlmFrame]];
}

/**
 * Convenience method to save everybody from creating these things all the time.
 */
+ (CBUUID *)eddystoneServiceID {
  static CBUUID *_singleton;
  static dispatch_once_t oncePredicate;

  dispatch_once(&oncePredicate, ^{
    _singleton = [CBUUID UUIDWithString:kEddystoneServiceID];
  });

  return _singleton;
}

/**
 * Calculate the friendly encoded data for the UID frame. May be null if the UID data is not available.
 */
+ (ESSBeaconUID *)dataForUIDFrame:(NSData *)rawFrameData {
  
  // -------------------------------
  // UID FRAME
  // -------------------------------
  // - txPower
  // - 10-byte ID Namespace
  // - 6-byte ID Instance
  // -------------------------------
  
  if(!rawFrameData) {
    return nil;
  }
  
  ESSBeaconUID *uid = [[ESSBeaconUID alloc] init];
  
  // Get the beacon UID frame bytes.
  const char *bytes = [rawFrameData bytes];
  
  // Create empty variables for the extracted frame data.
  NSMutableString *idNamespace = [NSMutableString string];
  NSMutableString *idInstance = [NSMutableString string];
  
  // Extract the UID frame details.
  for(int i = 1; i < [rawFrameData length]; i++) {
    
    if(i == 1) { // Extract txPower from bytes[1]
      
      // Note to developers: the best way to determine the precise value to put into this field
      // is to measure the actual output of your beacon from 1 meter away and then add 41dBm to
      // that. 41dBm is the signal loss that occurs over 1 meter.
      uid.txPower = (int)bytes[i] + txPowerAt1Meter;
      
    } else if(i >= 2 && i <= 11) { // Extract ID Namespace from bytes[2 - 11].
      
      [idNamespace appendString:[NSString stringWithFormat:@"%02hhX", bytes[i]]];
      
    } else if(i >= 12 && i <= 17) { // Extract ID Instance from bytes[12 - 17].

      [idInstance appendString:[NSString stringWithFormat:@"%02hhX", bytes[i]]];
    }
  }
  
  uid.idNamespace = [NSString stringWithFormat:@"%@", idNamespace];
  uid.idInstance = [NSString stringWithFormat:@"%@", idInstance];
  
  return uid;
}

/**
 * Calculate the friendly encoded data for the URL frame. May be null if the UID data is not available.
 */
+ (ESSBeaconURL *)dataForURLFrame:(NSData *)rawFrameData {
  
  if(!rawFrameData) {
    return nil;
  }
  
  // -------------------------------
  // URL FRAME
  // -------------------------------
  // - txPower
  // - URL Scheme
  // - Encoded URL
  // -------------------------------
  
  ESSBeaconURL *url = [[ESSBeaconURL alloc] init];
  
  
  
  // Get the beacon URL frame bytes.
  const char *bytes = [rawFrameData bytes];
  
  // Create empty variables for the extracted frame data.
  NSMutableString *urlString = [NSMutableString string];
  
  // Extract the UID frame details.
  for(int i = 1; i < [rawFrameData length]; i++) {
    
    if(i == 1) { // Extract txPower from bytes[1]
      
      // Note to developers: the best way to determine the precise value to put into this field
      // is to measure the actual output of your beacon from 1 meter away and then add 41dBm to
      // that. 41dBm is the signal loss that occurs over 1 meter.
      url.txPower = (int)bytes[i] + txPowerAt1Meter;
      
    } else if(i == 2) { // Extract the URL Scheme from bytes[2].
      
      switch (bytes[i]) {
        case 0x00:    [urlString appendString:@"http://www."];  break;
        case 0x01:    [urlString appendString:@"https://www."]; break;
        case 0x02:    [urlString appendString:@"http://"];      break;
        case 0x03:    [urlString appendString:@"https://"];     break;
      }
      
    } else if(i >= 3) { // Extract ID Instance from bytes[3 - 20].
      
      switch (bytes[i]) {
        case 0x00:    [urlString appendString:@".com/"];    break;
        case 0x01:    [urlString appendString:@".org/"];    break;
        case 0x02:    [urlString appendString:@".edu/"];    break;
        case 0x03:    [urlString appendString:@".net/"];    break;
        case 0x04:    [urlString appendString:@".info/"];   break;
        case 0x05:    [urlString appendString:@".biz/"];    break;
        case 0x06:    [urlString appendString:@".gov/"];    break;
        case 0x07:    [urlString appendString:@".com"];     break;
        case 0x08:    [urlString appendString:@".org"];     break;
        case 0x09:    [urlString appendString:@".edu"];     break;
        case 0x0a:    [urlString appendString:@".net"];     break;
        case 0x0b:    [urlString appendString:@".info"];    break;
        case 0x0c:    [urlString appendString:@".biz"];     break;
        case 0x0d:    [urlString appendString:@".gov"];     break;
        default:      [urlString appendString:[NSString stringWithFormat:@"%c", bytes[i]]];     break;
      }
    }
  }
  
  url.urlString = [NSString stringWithFormat:@"%@", urlString];
  
  return url;
}

/**
 * Calculate the friendly encoded data for the TLM frame. May be null if the UID data is not available.
 */
+ (ESSBeaconTLM *)dataForTLMFrame:(NSData *)rawFrameData {
  
  if(!rawFrameData) {
    return nil;
  }
  
  // -------------------------------
  // TLM FRAME
  // -------------------------------
  // - TLM version
  // - Battery voltage 1 mV/bit
  // - Beacon temperature
  // - Advertising PDU count
  // - Time since power-on or reboot
  // -------------------------------
  
  ESSBeaconTLM *tlm = [[ESSBeaconTLM alloc] init];
  
  
  /*
   1	Version	TLM version, value = 0x00
   2	VBATT[0]	Battery voltage, 1 mV/bit
   3	VBATT[1]
   4	TEMP[0]	Beacon temperature
   5	TEMP[1]
   6	ADV_CNT[0]	Advertising PDU count
   7	ADV_CNT[1]
   8	ADV_CNT[2]
   9	ADV_CNT[3]
   10	SEC_CNT[0]	Time since power-on or reboot
   11	SEC_CNT[1]
   12	SEC_CNT[2]
   13	SEC_CNT[3]
   */
  
  // Get the beacon TLM frame bytes.
  const char *bytes = [rawFrameData bytes];
  
  // Create empty variables for the extracted frame data.
  NSMutableString *version = [NSMutableString new];
  NSMutableString *voltage = [NSMutableString new];
  NSMutableString *temperature = [NSMutableString new];
  NSMutableString *advCount = [NSMutableString new];
  NSMutableString *uptime = [NSMutableString new];
  
  for(int i = 1; i < [rawFrameData length]; i++) {
    
    NSString *hexString = [NSString stringWithFormat:@"%02hhX", bytes[i]];
    
    if(i == 1) { // Extract version from bytes[1].
      [version appendString:hexString];
    } else if(i >= 2 && i <= 3) { // Extract voltage from bytes[2 - 3].
      [voltage appendString:hexString];
    } else if(i >= 4 && i <= 5) { // Extract temperature from bytes[4 - 5].
      [temperature appendString:hexString];
    } else if(i >= 6 && i <= 9) { // Extract adv count from bytes[6 - 9].
      [advCount appendString:hexString];
    } else if(i >= 10 && i <= 13) { // Extract uptime from bytes[10 - 13].
      [uptime appendString:hexString];
    }
  }
  
  // Calculate the beacon voltage value.
  unsigned int versionOutVal;
  [[NSScanner scannerWithString:version] scanHexInt:&versionOutVal];
  tlm.version = versionOutVal;
  
  // Calculate the beacon voltage value.
  unsigned int voltageOutVal;
  [[NSScanner scannerWithString:voltage] scanHexInt:&voltageOutVal];
  tlm.voltage = voltageOutVal;
  
  // Calculate the beacon temperature value from the 8.8 fix point notation.
  unsigned int temperatureOutVal;
  [[NSScanner scannerWithString:temperature] scanHexInt:&temperatureOutVal];
  tlm.temperature = (double)temperatureOutVal / (double)256;
  
  // Calculate the beacon advertising PDU count.
  unsigned int advOutVal;
  [[NSScanner scannerWithString:advCount] scanHexInt:&advOutVal];
  tlm.advCount = advOutVal;
  
  // Caculate the beacon up time.
  unsigned int uptimeOutVal;
  [[NSScanner scannerWithString:uptime] scanHexInt:&uptimeOutVal];
  tlm.uptime = uptimeOutVal;
  
  return tlm;
}

- (double)int88toDouble:(unsigned int)val {
  /* no error-checking on range of 'val' */
  return (val>>8) + (val & 0xFF)/100.;
}

@end
