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
#ifndef EVENTQUEUE_DETAIL_THUNKVTABLE_H_
#define EVENTQUEUE_DETAIL_THUNKVTABLE_H_

namespace eq {

// forward declaration of the Thunk class
class Thunk;

namespace detail {

/**
 * This POD is used as a vtable by Thunk implementation.
 * Thunk is a value type for all type nullary callable and therefore standard
 * polymorphism is not suitable for that use case.
 * Instead, the vtable is generated for each type contained in a thunk.
 * This structure is the prototype of such vtable.
 * \note see ThunkVTableGenerator for implementation and the generation of
 * Thunk vtables.
 */
struct ThunkVTable {
	typedef Thunk thunk_t;

	/**
	 * destroy a thunk (act like a destructor).
	 */
	void (* const destroy)(thunk_t& self);

	/**
	 * Copy self into dest.
	 * It is expected that dest is empty.
	 */
	void (* const copy)(thunk_t& dest, const thunk_t& self);

	/**
	 * Synthetized call for the inner object of the thunk_t.
	 */
	void (* const call)(const thunk_t& self);
};

} // namespace detail
} // namespace eq

#endif /* EVENTQUEUE_DETAIL_THUNKVTABLE_H_ */
