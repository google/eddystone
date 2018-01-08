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
#ifndef BLE_API_SOURCE_MBEDCLASSICEVENTQUEUE_H_
#define BLE_API_SOURCE_MBEDCLASSICEVENTQUEUE_H_

#include <cmsis.h>
#include "PriorityQueue.h"
#include "Ticker.h"
#include "Timer.h"
#include <stdio.h>
#include "Thunk.h"
#include "MakeThunk.h"
#include "EventQueue.h"

#include <util/CriticalSectionLock.h>
typedef ::mbed::util::CriticalSectionLock CriticalSection;

namespace eq {

template<std::size_t EventCount>
class EventQueueClassic: public EventQueue {

	/// Describe an event.
	/// An event is composed of a function f to execute after a a time t.
	/// Optionnaly, the event can be periodic and in this case the function f
	/// is executed after each period p.
	struct Event {
		/// construct an event
		/// @param f The function to execute when this event occur
		/// @param ms_remaining_time remaining time before this event occurence
		/// @param ms_repeat_period If the event is periodic, this parameter is the
		/// period between to occurence of this event.
		Event(const function_t& f, ms_time_t ms_remaining_time, ms_time_t ms_repeat_period = 0) :
			_f(f),
			_ms_remaining_time(ms_remaining_time),
			_ms_repeat_period(ms_repeat_period) {
		}

		/// call the inner function within an event
		void operator()() {
			_f();
		}

		/// return a reference to the inner function
		const function_t& get_function() const {
			return _f;
		}

		/// comparison operator used by the priority queue.
		/// comaprare remaining time between two events
		friend bool operator<(const Event& lhs, const Event& rhs) {
			return lhs._ms_remaining_time < rhs._ms_remaining_time;
		}

		/// return the time remaining when this event was inserted into the priority queue.
		ms_time_t get_ms_remaining_time() const {
			return _ms_remaining_time;
		}

		/// update the remaining time for this event
		void set_ms_remaining_time(ms_time_t new_remaining_time) {
			_ms_remaining_time = new_remaining_time;
		}

		/// If an event is periodic, return the time between two occurence
		ms_time_t get_ms_repeat_period() const {
			return _ms_repeat_period;
		}

	private:
		function_t _f;
		ms_time_t _ms_remaining_time;
		const ms_time_t _ms_repeat_period;
	};

	/// type of the internal queue
	typedef PriorityQueue<Event, EventCount> priority_queue_t;

	/// iterator for the queue type
	typedef typename priority_queue_t::iterator q_iterator_t;

	/// node type in the queue
	typedef typename priority_queue_t::Node q_node_t;

public:
	/// Construct an empty event queue
	EventQueueClassic() :
		_events_queue(), _ticker(), _timer(), _timed_event_pending(false) {
	}

	virtual ~EventQueueClassic() { }

	virtual bool cancel(event_handle_t event_handle) {
		CriticalSection critical_section;
		bool success = _events_queue.erase(static_cast<q_node_t*>(event_handle));
		if (success) {
			// update the timers and events remaining time
			updateTime();
		}
		return success;
	}

	void dispatch() {
		while(true) {
			function_t f;
			// pick a task from the queue/ or leave
			{
				CriticalSection cs;
				q_iterator_t event_it = _events_queue.begin();
				if(event_it != _events_queue.end() && event_it->get_ms_remaining_time() == 0) {
					f = event_it->get_function();
					// if the event_it should be repeated, reschedule it
					if (event_it->get_ms_repeat_period()) {
						reschedule_event(event_it);
					} else {
						_events_queue.pop();
					}
				} else {
					break;
				}
			}
			f();
		}
	}

private:

	void update_ticker(ms_time_t ms_delay) {
		_timed_event_pending = true;
		_ticker.detach();
		_ticker.attach(this, &EventQueueClassic::updateTime, ((float) ms_delay / 1000));
	}

	void update_ticker(q_node_t* ref, ms_time_t ms_delay) {
		// look if the node inserted is the first node with a delay
		for (q_iterator_t it = _events_queue.begin(); it != _events_queue.end(); ++it) {
			if(it->get_ms_remaining_time()) {
				if (it.get_node() == ref) {
					// update the ticker to ms_delay if the first event
					// with a delay is the one inserted
					update_ticker(ms_delay);
				}
				break;
			}
		}
	}

	void update_events_remaining_time(ms_time_t elapsed_time) {
		bool ticker_updated = false;

		for (q_iterator_t it = _events_queue.begin();
		     it != _events_queue.end(); ++it) {
			ms_time_t remaining_time = it->get_ms_remaining_time();
			if(remaining_time) {
				if(remaining_time <= elapsed_time) {
					it->set_ms_remaining_time(0);
				} else {
					it->set_ms_remaining_time(remaining_time - elapsed_time);
					if (!ticker_updated) {
						update_ticker(it->get_ms_remaining_time());
						_timer.start();
						ticker_updated = true;
					}
				}
			}
		}
	}

	void updateTime() {
		CriticalSection critical_section;
		ms_time_t elapsed_time = _timer.read_ms();
		_timed_event_pending = false;
		_timer.stop();
		_timer.reset();
		_ticker.detach();
		update_events_remaining_time(elapsed_time);
	}

	void reschedule_event(q_iterator_t& event_it) {
		ms_time_t ms_period = event_it->get_ms_repeat_period();

		if (_timed_event_pending ==  false) {
			update_ticker(ms_period);
			_timer.start();
			event_it->set_ms_remaining_time(ms_period);
			_events_queue.update(event_it);
		} else {
			int elapsed_time = _timer.read_ms();
			event_it->set_ms_remaining_time(elapsed_time + ms_period);
			_events_queue.update(event_it);
			update_ticker(event_it.get_node(), ms_period);
		}
	}

	virtual event_handle_t do_post(const function_t& fn, ms_time_t ms_delay = 0, bool repeat = false) {
		if(repeat && (ms_delay == 0)) {
			return NULL;
		}

		Event event(fn, ms_delay, repeat ? ms_delay : 0);

		CriticalSection critical_section;
		if (_events_queue.full()) {
			return NULL;
		}

		// there is no need to update timings if ms_delay == 0
		if (!ms_delay) {
			return _events_queue.push(event).get_node();
		}

		// if there is no pending timed event, just add this one and start timers
		if (_timed_event_pending ==  false) {
			update_ticker(ms_delay);
			_timer.start();
			return _events_queue.push(event).get_node();
		}

		int elapsed_time = _timer.read_ms();

		// update remaining time and post the event
		event.set_ms_remaining_time(ms_delay + elapsed_time);
		event_handle_t handle = _events_queue.push(event).get_node();
		update_ticker(static_cast<q_node_t*>(handle), ms_delay);

		return handle;
	}

	priority_queue_t _events_queue;
	mbed::Ticker _ticker;
	mbed::Timer _timer;
	bool _timed_event_pending;
};

} // namespace eq

#endif /* BLE_API_SOURCE_MBEDCLASSICEVENTQUEUE_H_ */
