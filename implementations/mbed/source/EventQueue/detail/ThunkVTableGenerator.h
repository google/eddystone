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
#ifndef EVENTQUEUE_DETAIL_THUNKVTABLEGENERATOR_H_
#define EVENTQUEUE_DETAIL_THUNKVTABLEGENERATOR_H_

// imported from Thunk.h

namespace eq {
namespace detail {

/**
 * Thunk VTable Generator.
 * This class generate the vtable of a type F for a Thunk.
 * \tparam F The type of the callable for which the Thunk vtable should be
 * generated.
 */
template<typename F>
struct ThunkVTableGenerator {
	typedef Thunk thunk_t;

	/**
	 * Implementation of destructor for Thunk holding an F.
	 * @param self The thunk to destroy
	 */
	static void destroy(thunk_t& self) {
		get_ptr(self)->~F();
	}

	/**
	 * Implementation of copy (used by copy constructor and copy assignment)
	 * for a Thunk holding an F.
	 * @param dest The thunk receiving the copy.
	 * @param self The thunk to copy.
	 */
	static void copy(thunk_t& dest, const thunk_t& self) {
		new (get_ptr(dest)) F(*get_ptr(self));
		dest._vtable = self._vtable;
	}

	/**
	 * Implementation of call operator for a Thunk holding an F.
	 * @param self The thunk containing the F to call.
	 */
	static void call(const thunk_t& self) {
		(*get_ptr(self))();
	}

	/**
	 * The Thunk vtable for an F.
	 */
	static const ThunkVTable vtable;

private:
	/**
	 * Accessor to the pointer to F contained in the Thunk.
	 */
	static F* get_ptr(thunk_t& thunk) {
		return static_cast<F*>(thunk._storage.get_storage(0));
	}

	/**
	 * Accessor to the const pointer to F contained in the const Thunk.
	 */
	static const F* get_ptr(const thunk_t& thunk) {
		return static_cast<const F*>(thunk._storage.get_storage(0));
	}
};

/**
 * Instantiation of the Thunk vtable of F.
 */
template<typename F>
const ThunkVTable ThunkVTableGenerator<F>::vtable = {
		ThunkVTableGenerator<F>::destroy,
		ThunkVTableGenerator<F>::copy,
		ThunkVTableGenerator<F>::call
};

} // namespace detail
} // namespace eq

#endif /* EVENTQUEUE_DETAIL_THUNKVTABLEGENERATOR_H_ */
