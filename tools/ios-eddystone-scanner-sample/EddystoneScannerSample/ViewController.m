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

#import "ViewController.h"

#import "ESSBeaconScanner.h"
#import "ESSEddystone.h"

@interface ViewController () <ESSBeaconScannerDelegate> {
  ESSBeaconScanner *_scanner;
}

@end

@implementation ViewController

- (void)viewDidLoad {
  [super viewDidLoad];
  // Do any additional setup after loading the view, typically from a nib.
}

- (void)viewWillAppear:(BOOL)animated {
  [super viewWillAppear:animated];
  _scanner = [[ESSBeaconScanner alloc] init];
  _scanner.delegate = self;
  [_scanner startScanning];
}

- (void)viewDidDisappear:(BOOL)animated {
  [super viewDidDisappear:animated];
  [_scanner stopScanning];
  _scanner = nil;
}

/**
 * Called when an Eddystone is seen for the first time.
 */
- (void)beaconScanner:(ESSBeaconScanner *)scanner didFindBeacon:(ESSBeaconInfo *)beaconInfo {
  NSLog(@"I Saw an Eddystone!: %@", beaconInfo);
}

/**
 * Called when a previously seen Eddystone is no longer detected.
 */
- (void)beaconScanner:(ESSBeaconScanner *)scanner didLoseBeacon:(ESSBeaconInfo *)beaconInfo {
  NSLog(@"I Lost an Eddystone!: %@", beaconInfo);
}

/**
 * Called when an already seen Eddystone is redetected.
 */
- (void)beaconScanner:(ESSBeaconScanner *)scanner didUpdateBeacon:(ESSBeaconInfo *)beaconInfo updatedFrame:(ESSFrameType)frameType {
  //NSLog(@"I Updated an Eddystone!: %@, %lu", beaconInfo, (unsigned long)frameType);
  
  // Log out specific beacon frame data.
  switch (frameType) {
    case kESSEddystoneUIDFrameType: {
      // Lookup & log the friendly data for the UID frame.
      ESSBeaconUID *uidData = [ESSBeaconInfo dataForUIDFrame:beaconInfo.uidFrame];
      NSLog(@"UID FRAME: txPower: %d idNamespace: %@ idInstance: %@", uidData.txPower, uidData.idNamespace, uidData.idInstance);
    }
      break;
      
    case kESSEddystoneURLFrameType: {
      // Lookup & log the friendly data for the UID frame.
      ESSBeaconURL *urlData = [ESSBeaconInfo dataForURLFrame:beaconInfo.urlFrame];
      NSLog(@"URL FRAME: txPower: %d urlString: %@", urlData.txPower, urlData.urlString);
    }
      break;
      
    case kESSEddystoneTLMFrameType: {
      // Lookup & log the friendly data for the TLM frame.
      ESSBeaconTLM *tlmData = [ESSBeaconInfo dataForTLMFrame:beaconInfo.tlmFrame];
      NSLog(@"TLM FRAME: version: %u voltage: %d temperature: %f pduCount: %d uptime: %d", tlmData.version, tlmData.voltage, tlmData.temperature, tlmData.advCount, tlmData.uptime);
    }
      break;
      
    case kESSEddystoneUnknownFrameType:
      // Do Nothing!
      break;
  }
}

@end