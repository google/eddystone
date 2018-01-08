/*
 * Copyright (c) 2016, ARM Limited, All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef EVENTQUEUE_EVENTQUEUEMINAR_H_
#define EVENTQUEUE_EVENTQUEUEMINAR_H_

#include <minar/minar.h>
#include "EventQueue.h"

namespace eq {

class EventQueueMinar: public EventQueue {

public:
	/// Construct an empty event queue
	EventQueueMinar()  { }

	virtual ~EventQueueMinar() { }

	virtual bool cancel(event_handle_t event_handle) {
        return minar::Scheduler::cancelCallback(event_handle);
	}

private:

	virtual event_handle_t do_post(const function_t& fn, ms_time_t ms_delay = 0, bool repeat = false) {
        // convert ms to minar time
        minar::tick_t tick = minar::milliseconds(ms_delay);

        // convert thunk to minar FunctionPointerBind
        mbed::util::Event func(
            mbed::util::FunctionPointer1<void, function_t>(
                free_func_thunk_call
            ).bind(fn)
        );

        if (ms_delay == 0) {
            return minar::Scheduler::postCallback(func).getHandle();
        }

        if (repeat == false) {
            return minar::Scheduler::postCallback(func).delay(tick).tolerance(0).getHandle();
        } else {
            return minar::Scheduler::postCallback(func).period(tick).tolerance(0).getHandle();
        }
	}

	// due to design limitations in function pointer classes, it is not possible
	// to use reference here ... 
    static void free_func_thunk_call(function_t fn) {
        fn();
    }
};

} // namespace eq

#endif /* EVENTQUEUE_EVENTQUEUEMINAR_H_ */
