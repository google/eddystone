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

@class ESSBeaconScanner;

// Delegates to the ESSBeaconScanner should implement this protocol.
@protocol ESSBeaconScannerDelegate <NSObject>

@optional

- (void)beaconScanner:(ESSBeaconScanner *)scanner
        didFindBeacon:(id)beaconInfo;
- (void)beaconScanner:(ESSBeaconScanner *)scanner
        didLoseBeacon:(id)beaconInfo;

- (void)beaconScanner:(ESSBeaconScanner *)scanner
      didUpdateBeacon:(id)beaconInfo;

- (void)beaconScanner:(ESSBeaconScanner *)scanner
           didFindURL:(NSURL *)url;

@end

@interface ESSBeaconScanner : NSObject

@property(nonatomic, weak) id<ESSBeaconScannerDelegate> delegate;

@property(nonatomic, assign) NSTimeInterval onLostTimeout;

- (void)startScanning;
- (void)stopScanning;

@end
