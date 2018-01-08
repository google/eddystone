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
#ifndef EVENTQUEUE_DETAIL_THUNK_IMPL_H_
#define EVENTQUEUE_DETAIL_THUNK_IMPL_H_

#include <new>
#include "ThunkVTableGenerator.h"

namespace eq {

/**
 * Thunk constructor Implementation.
 * Due to the way templates and forwarding work in C++, it was not possible to
 * provide this implementation in Thunk.h
 */
template<typename F>
Thunk::Thunk(const F& f) :
	_storage(),
	_vtable(&detail::ThunkVTableGenerator<F>::vtable) {
	typedef  __attribute__((unused)) char F_is_too_big_for_the_Thunk[sizeof(F) <= sizeof(_storage) ? 1 : -1];
	new(_storage.get_storage(0)) F(f);
}

/**
 * Specialization for function pointers.
 * This overload will be chosen when the tyope in input is a reference to a function.
 * @param  f The function to transform in Thunk.
 */
inline Thunk::Thunk(void (*f)()) :
	_storage(),
	_vtable(&detail::ThunkVTableGenerator<void(*)()>::vtable) {
	typedef void(*F)();
	typedef  __attribute__((unused)) char F_is_too_big_for_the_Thunk[sizeof(F) <= sizeof(_storage) ? 1 : -1];
	new(_storage.get_storage(0)) F(f);
}

/**
 * Thunk empty constructor Implementation.
 * Due to the way templates and forwarding work in C++, it was not possible to
 * provide this implementation in Thunk.h
 */
inline Thunk::Thunk() :
	_storage(),
	_vtable(&detail::ThunkVTableGenerator<void(*)()>::vtable) {
	typedef void(*F)();
	typedef  __attribute__((unused)) char F_is_too_big_for_the_Thunk[sizeof(F) <= sizeof(_storage) ? 1 : -1];
	new(_storage.get_storage(0)) F(empty_thunk);
}

} // namespace eq

#endif /* EVENTQUEUE_DETAIL_THUNK_IMPL_H_ */
