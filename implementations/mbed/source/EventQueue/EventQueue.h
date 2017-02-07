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
#ifndef EVENTQUEUE_EVENTQUEUE_H_
#define EVENTQUEUE_EVENTQUEUE_H_

#include <stdio.h>
#include "Thunk.h"
#include "MakeThunk.h"

namespace eq {

class EventQueue {

public:
	/// typedef for callable type.
	/// the callable type used should support the same operations
	/// supported by a void(*)() function pointer.
	typedef Thunk function_t;

	/// handle to a posted event which will be executed later.
	/// model after a void* pointer.
	typedef void* event_handle_t;

	/// type used for time
	typedef std::size_t ms_time_t;

	/// Construct an empty event queue
	EventQueue() { }

	virtual ~EventQueue() { }

	/**
	 * Post a callable to the event queue.
	 * It will be executed during the next dispatch cycle.
	 * @param f The callbable to be executed by the event queue.
	 * @return the handle to the event.
	 */
	template<typename F>
	event_handle_t post(const F& fn) {
		return do_post(fn);
	}

	/**
	 * Bind a callable and an argument then post a callable to the event queue.
	 * It will be executed during the next dispatch cycle.
	 * @param f The callbable to be bound with arg0.
	 * @param arg0 The first argument to bind to f.
	 * @return the handle to the event.
	 */
	template<typename F, typename Arg0>
	event_handle_t post(const F& fn, const Arg0& arg0) {
		return do_post(make_thunk(fn, arg0));
	}

	template<typename F, typename Arg0, typename Arg1>
	event_handle_t post(const F& fn, const Arg0& arg0, const Arg1& arg1) {
		return do_post(make_thunk(fn, arg0, arg1));
	}

	template<typename F, typename Arg0, typename Arg1, typename Arg2>
	event_handle_t post(const F& fn, const Arg0& arg0, const Arg1& arg1, const Arg2& arg2) {
		return do_post(make_thunk(fn, arg0, arg1, arg2));
	}

	template<typename F>
	event_handle_t post_in(const F& fn, ms_time_t ms_delay) {
		return do_post(fn, ms_delay);
	}

	template<typename F, typename Arg0>
	event_handle_t post_in(const F& fn, const Arg0& arg0, ms_time_t ms_delay) {
		return do_post(make_thunk(fn, arg0), ms_delay);
	}

	template<typename F, typename Arg0, typename Arg1>
	event_handle_t post_in(const F& fn, const Arg0& arg0, const Arg1& arg1, ms_time_t ms_delay) {
		return do_post(make_thunk(fn, arg0, arg1), ms_delay);
	}

	template<typename F, typename Arg0, typename Arg1, typename Arg2>
	event_handle_t post_in(const F& fn, const Arg0& arg0, const Arg1& arg1, const Arg2& arg2, ms_time_t ms_delay) {
		return do_post(make_thunk(fn, arg0, arg1, arg2), ms_delay);
	}

	template<typename F>
	event_handle_t post_every(const F& fn, ms_time_t ms_delay) {
		return do_post(fn, ms_delay, true);
	}

	template<typename F, typename Arg0>
	event_handle_t post_every(const F& fn, const Arg0& arg0, ms_time_t ms_delay) {
		return do_post(make_thunk(fn, arg0), ms_delay, true);
	}

	template<typename F, typename Arg0, typename Arg1>
	event_handle_t post_every(const F& fn, const Arg0& arg0, const Arg1& arg1, ms_time_t ms_delay) {
		return do_post(make_thunk(fn, arg0, arg1), ms_delay, true);
	}

	template<typename F, typename Arg0, typename Arg1, typename Arg2>
	event_handle_t post_every(const F& fn, const Arg0& arg0, const Arg1& arg1, const Arg2& arg2, ms_time_t ms_delay) {
		return do_post(make_thunk(fn, arg0, arg1, arg2), ms_delay, true);
	}

	virtual bool cancel(event_handle_t event_handle) = 0;

private:
	virtual event_handle_t do_post(const function_t& fn, ms_time_t ms_delay = 0, bool repeat = false) = 0;
};

} // namespace eq

#endif /* EVENTQUEUE_EVENTQUEUE_H_ */
