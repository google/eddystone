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

#import <CoreBluetooth/CoreBluetooth.h>

#import "ESSBeaconScanner.h"
#import "ESSEddystone.h"
#import "ESSTimer.h"

static const char *const kBeaconsOperationQueueName = "kESSBeaconScannerBeaconsOperationQueueName";

static NSString *const kESSEddystoneServiceID = @"FEAA";

static NSString *const kSeenCacheBeaconInfo = @"beacon_info";
static NSString *const kSeenCacheOnLostTimer = @"on_lost_timer";

/**
 *=-----------------------------------------------------------------------------------------------=
 * Private Additions to ESSBeaconScanner
 *=-----------------------------------------------------------------------------------------------=
 */
@interface ESSBeaconScanner () <CBCentralManagerDelegate> {
  CBCentralManager *_centralManager;
  dispatch_queue_t _beaconOperationsQueue;
  
  /**
   * Beacons we've seen already. If we see an Eddystone and notice that we've seen it before, we
   * won't fire a beaconScanner:didFindBeacon:, but instead will fire a
   * beaconScanner:didUpdateBeacon: for those listening (it's optional).
   */
  NSMutableDictionary *_seenEddystoneCache;
  
  BOOL _shouldBeScanning;
}

@end

/**
 *=-----------------------------------------------------------------------------------------------=
 * Implementation for ESSBeaconScanner
 *=-----------------------------------------------------------------------------------------------=
 */
@implementation ESSBeaconScanner

- (instancetype)init {
  if ((self = [super init]) != nil) {
    _onLostTimeout = 15.0;
    _seenEddystoneCache = [NSMutableDictionary dictionary];
    _beaconOperationsQueue = dispatch_queue_create(kBeaconsOperationQueueName, NULL);
    _centralManager = [[CBCentralManager alloc] initWithDelegate:self
                                                           queue:_beaconOperationsQueue];
  }
  
  return self;
}

- (void)startScanning {
  dispatch_async(_beaconOperationsQueue, ^{
    if (_centralManager.state != CBCentralManagerStatePoweredOn) {
      NSLog(@"CBCentralManager state is %ld, cannot start or stop scanning",
            (long)_centralManager.state);
      _shouldBeScanning = YES;
    } else {
      NSLog(@"Starting to scan for Eddystones");
      NSArray *services = @[
                            [CBUUID UUIDWithString:kESSEddystoneServiceID]
                            ];
      
      // We do not want multiple discoveries of the same beacon to be coalesced into one.
      // (Unfortunately this is ignored when we are in the background.)
      NSDictionary *options = @{ CBCentralManagerScanOptionAllowDuplicatesKey : @YES };
      [_centralManager scanForPeripheralsWithServices:services options:options];
    }
  });
}

- (void)stopScanning {
  _shouldBeScanning = NO;
  [_centralManager stopScan];
  [self clearRemainingTimers];
}

- (void)centralManagerDidUpdateState:(CBCentralManager *)central {
  if (central.state == CBCentralManagerStatePoweredOn && _shouldBeScanning) {
    [self startScanning];
  }
}

// This will be called from the |beaconsOperationQueue|.
- (void)centralManager:(CBCentralManager *)central
 didDiscoverPeripheral:(CBPeripheral *)peripheral
     advertisementData:(NSDictionary *)advertisementData
                  RSSI:(NSNumber *)RSSI {
  
  NSDictionary *serviceData = advertisementData[CBAdvertisementDataServiceDataKey];
  
  
  // Build a list of Eddystone sightings. Note: as an Eddystone broadcasts up to 3 frame types
  // (UID, URL & TLM), we incrementally update the list of sighted Eddystones as more frames are
  // seen over time.
  
  ESSBeaconInfo *beaconInfo = _seenEddystoneCache[peripheral.identifier][kSeenCacheBeaconInfo];
  
  if(!beaconInfo) {
    beaconInfo = [[ESSBeaconInfo alloc] init];
  }
  beaconInfo.identifier = peripheral.identifier;
  beaconInfo.RSSI = RSSI;
  
  CBUUID *eddystoneServiceUUID = [ESSBeaconInfo eddystoneServiceID];
  NSData *eddystoneServiceData = serviceData[eddystoneServiceUUID];
  
  ESSFrameType frameType = [ESSBeaconInfo frameTypeForFrame:serviceData];
  
  switch (frameType) {
    case kESSEddystoneUIDFrameType:
      // Update the beacon's UID data.
      beaconInfo.uidFrame = eddystoneServiceData;
      break;
      
    case kESSEddystoneURLFrameType:
      // Update the beacon's URL data.
      beaconInfo.urlFrame = eddystoneServiceData;
      break;
      
    case kESSEddystoneTLMFrameType:
      // Update the beacon's TLM data.
      beaconInfo.tlmFrame = eddystoneServiceData;
      break;
      
    default:
      // Unsupported frame type, stop processing the frame.
      NSLog(@"Unsupported frame type (%d) detected. Ignorning.", (int)frameType);
      return;
  }
  
  // If we haven't seen this Eddystone before, fire a beaconScanner:didFindBeacon: and mark it
  // as seen.
  if (!_seenEddystoneCache[peripheral.identifier]) {
    if ([_delegate respondsToSelector:@selector(beaconScanner:didFindBeacon:)]) {
      [_delegate beaconScanner:self didFindBeacon:beaconInfo];
    }
    
    ESSTimer *onLostTimer = [ESSTimer scheduledTimerWithDelay:_onLostTimeout
                                                      onQueue:dispatch_get_main_queue()
                                                        block:
                             ^(ESSTimer *timer) {
                               ESSBeaconInfo *lostBeaconInfo =
                               _seenEddystoneCache[peripheral.identifier][kSeenCacheBeaconInfo];
                               if (lostBeaconInfo) {
                                 if ([_delegate respondsToSelector:@selector(beaconScanner:didLoseBeacon:)]) {
                                   [_delegate beaconScanner:self didLoseBeacon:lostBeaconInfo];
                                 }
                                 [_seenEddystoneCache removeObjectForKey:peripheral.identifier];
                               }
                             }];
    
    _seenEddystoneCache[peripheral.identifier] = @{
                                                 kSeenCacheBeaconInfo: beaconInfo,
                                                 kSeenCacheOnLostTimer: onLostTimer,
                                                 };
  } else {
    // Reset the onLost timer.
    [_seenEddystoneCache[peripheral.identifier][kSeenCacheOnLostTimer] reschedule];
    
    // Otherwise, we've seen it, so fire a beaconScanner:didUpdateBeacon: instead.
    if ([_delegate respondsToSelector:@selector(beaconScanner:didUpdateBeacon: updatedFrame:)]) {
      [_delegate beaconScanner:self didUpdateBeacon:beaconInfo updatedFrame:frameType];
    }
  }
}

- (void)clearRemainingTimers {
  for(id key in _seenEddystoneCache) {
    id sighting = [_seenEddystoneCache objectForKey:key];
    [sighting[kSeenCacheOnLostTimer] cancel];
  }
  
  _seenEddystoneCache = nil;
}

@end