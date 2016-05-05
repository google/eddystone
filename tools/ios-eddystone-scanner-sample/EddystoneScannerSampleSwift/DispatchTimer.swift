// Copyright 2015-2016 Google Inc. All rights reserved.
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

import UIKit

///
/// DispatchTimer
///
/// Much like an NSTimer from Cocoa, but implemented using dispatch queues instead.
///
class DispatchTimer: NSObject {

  /// Type for the handler block executed when a dispatch timer fires.
  ///
  /// :param: timer The timer which triggered this block
  typealias TimerHandler = (DispatchTimer) -> Void

  private let timerBlock: TimerHandler
  private let queue: dispatch_queue_t
  private let delay: NSTimeInterval

  private var wrappedBlock: (() -> Void)?
  private let source: dispatch_source_t

  init(delay: NSTimeInterval, queue: dispatch_queue_t, block: TimerHandler) {
    timerBlock = block
    self.queue = queue
    self.delay = delay

    self.source = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, self.queue)

    super.init()

    let wrapper = { () -> Void in
      if dispatch_source_testcancel(self.source) == 0 {
        dispatch_source_cancel(self.source)
        self.timerBlock(self)
      }
    }

    self.wrappedBlock = wrapper
  }

  class func scheduledDispatchTimer(delay: NSTimeInterval,
    queue: dispatch_queue_t,
    block: TimerHandler) -> DispatchTimer {
      let dt = DispatchTimer(delay: delay, queue: queue, block: block)
      dt.schedule()
      return dt
  }

  func schedule() {
    self.reschedule()
    dispatch_source_set_event_handler(self.source, self.wrappedBlock)
    dispatch_resume(self.source)
  }

  func reschedule() {
    let start = dispatch_time(DISPATCH_TIME_NOW, Int64(self.delay * Double(NSEC_PER_SEC)))

    // Leeway is 10% of timer delay
    dispatch_source_set_timer(self.source, start, DISPATCH_TIME_FOREVER,
      UInt64((self.delay / 10.0) * Double(NSEC_PER_SEC)))
  }

  func suspend() {
    dispatch_suspend(self.source)
  }

  func resume() {
    dispatch_resume(self.source)
  }

  func cancel() {
    dispatch_source_cancel(self.source)
  }

}
