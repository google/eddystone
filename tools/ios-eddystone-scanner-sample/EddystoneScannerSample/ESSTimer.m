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

#import "ESSTimer.h"

/**
 *=-----------------------------------------------------------------------------------------------=
 * Private Additions to ESSTimer
 *=-----------------------------------------------------------------------------------------------=
 */
@interface ESSTimer ()

@property(nonatomic, copy, readwrite) ESSTimerBlock block;
@property(nonatomic, copy, readwrite) void(^wrappedBlock)();
@property(nonatomic, strong, readwrite) dispatch_queue_t queue;
@property(nonatomic, assign, readwrite) NSTimeInterval delay;

// The dispatch source used for the timer.
@property(nonatomic, strong, readwrite) dispatch_source_t source;

@property(nonatomic, assign) BOOL suspended;

@end

/**
 *=-----------------------------------------------------------------------------------------------=
 * Implementation for ESSTimer
 *=-----------------------------------------------------------------------------------------------=
 */
@implementation ESSTimer

- (ESSTimer *)initWithDelay:(NSTimeInterval)delay
                    onQueue:(dispatch_queue_t)queue
                      block:(ESSTimerBlock)block {
  self = [super init];
  if (self) {
    _block = block;
    _queue = queue;
    _delay = delay;

    __block ESSTimer *blockSelf = self;

    _wrappedBlock = ^{
      // This is a one-shot timer - ensure we don't call its block after it has been canceled.
      if (!dispatch_source_testcancel(blockSelf.source)) {
        dispatch_source_cancel(blockSelf.source);
        blockSelf.block(blockSelf);
      }
    };

    _source = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, _queue);
  }
  return self;
}

+ (ESSTimer *)timerWithDelay:(NSTimeInterval)delay
                     onQueue:(dispatch_queue_t)queue
                       block:(ESSTimerBlock)block {
  return [[self alloc] initWithDelay:delay onQueue:queue block:block];

}

+ (ESSTimer *)scheduledTimerWithDelay:(NSTimeInterval)delay
                              onQueue:(dispatch_queue_t)queue
                                block:(ESSTimerBlock)block {
  ESSTimer *timer = [self timerWithDelay:delay onQueue:queue block:block];
  [timer schedule];
  return timer;
}

- (void)schedule {
  [self reschedule];
  dispatch_source_set_event_handler(_source, _wrappedBlock);
  dispatch_resume(_source);
}

- (void)reschedule {
  dispatch_time_t start = dispatch_time(DISPATCH_TIME_NOW, (int64_t)(_delay * NSEC_PER_SEC));
  // Leeway is 10% of timer delay
  dispatch_source_set_timer(_source, start, DISPATCH_TIME_FOREVER,
                            (uint64_t)(_delay / 10) * NSEC_PER_SEC);
}

- (void)suspend {
  dispatch_suspend(_source);
}

- (void)resume {
  dispatch_resume(_source);
}

- (void)cancel {
  dispatch_source_cancel(_source);
}

@end
