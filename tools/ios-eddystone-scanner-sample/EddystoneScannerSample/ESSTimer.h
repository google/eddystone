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

@class ESSTimer;

typedef void(^ESSTimerBlock)(ESSTimer *timer);

/**
 *=-----------------------------------------------------------------------------------------------=
 * ESSTimer
 *=-----------------------------------------------------------------------------------------------=
 * A Timer class, much like NSTimer, but implemented using dispatch queues and sources.
 */
@interface ESSTimer : NSObject

@property(nonatomic, copy, readonly) ESSTimerBlock block;
@property(nonatomic, strong, readonly) dispatch_queue_t queue;
@property(nonatomic, assign, readonly) NSTimeInterval delay;

- (ESSTimer *)initWithDelay:(NSTimeInterval)delay
                    onQueue:(dispatch_queue_t)queue
                      block:(ESSTimerBlock)block;

+ (ESSTimer *)timerWithDelay:(NSTimeInterval)delay
                     onQueue:(dispatch_queue_t)queue
                       block:(ESSTimerBlock)block;

+ (ESSTimer *)scheduledTimerWithDelay:(NSTimeInterval)delay
                              onQueue:(dispatch_queue_t)queue
                                block:(ESSTimerBlock)block;

/**
 * Schedule this timer so that it will fire after the specified delay.
 */
- (void)schedule;

/**
 * Reschedule the timer, so it will fire after the specified delay from the current time, rather
 * than from when it was initially scheduled or last rescheduled.
 * This should not be called before |schedule|, though it will have no effect.
 */
- (void)reschedule;

/**
 * Suspend this timer, so that it will not fire unless and until it has been resumed.
 */
- (void)suspend;

/**
 * Resume the timer to allow it to fire.
 */
- (void)resume;

/**
 * Cancel the timer, preventing it from ever firing again.
 */
- (void)cancel;

@end
