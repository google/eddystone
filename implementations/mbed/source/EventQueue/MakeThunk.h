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
#ifndef EVENTQUEUE_MAKETHUNK_H
#define EVENTQUEUE_MAKETHUNK_H

#include "detail/MemberFunctionAdaptor.h"
#include "detail/FunctionAdaptor.h"
#include "detail/Thunks.h"

namespace eq {

/**
 * Make a thunk from an F.
 * When this function only takes an F then F it is expected that F is already
 * a callable and therefore a kind of thunk.
 * @tparam F The type of callable in input.
 * @param fn the function to turn into a thunk.
 * @return fn
 */
template<typename F>
const F& make_thunk(const F& fn) {
	return fn;
}

/**
 * Bind fn and arg0 into a thunk.
 * @tparam F the type of the function to bind. It can be a function pointer,
 * a function like object or a pointer to a member function.
 * @tparam Arg0 The type of the first argument of F.
 * @param fn the function to bind.
 * @param arg0 the first argument to bind.
 * @return a thunk binding F and arg0.
 */
template<typename F, typename Arg0>
detail::Thunk_1<typename detail::FunctionAdaptor<F>::type, Arg0>
make_thunk(const F& fn, const Arg0& arg0) {
	typedef typename detail::FunctionAdaptor<F>::type fn_adaptor_t;
	return detail::Thunk_1<fn_adaptor_t, Arg0>(
		fn_adaptor_t(fn),
		arg0
	);
}

/**
 * Bind fn, arg0 and arg1 into a thunk.
 * @tparam F the type of the function to bind. It can be a function pointer,
 * a function like object or a pointer to a member function.
 * @tparam Arg0 The type of the first argument of F.
 * @tparam Arg1 The type of the second argument of F.
 * @param fn the function to bind.
 * @param arg0 the first argument to bind.
 * @param arg1 the second argument to bind.
 * @return a thunk binding F, arg0 and arg1.
 */
template<typename F, typename Arg0, typename Arg1>
detail::Thunk_2<typename detail::FunctionAdaptor<F>::type, Arg0, Arg1>
make_thunk(const F& fn, const Arg0& arg0, const Arg1& arg1) {
	typedef typename detail::FunctionAdaptor<F>::type fn_adaptor_t;
	return detail::Thunk_2<fn_adaptor_t, Arg0, Arg1>(
		fn_adaptor_t(fn),
		arg0,
		arg1
	);
}

/**
 * Bind fn, arg0, arg1 and arg2 into a thunk.
 * @tparam F the type of the function to bind. It can be a function pointer,
 * a function like object or a pointer to a member function.
 * @tparam Arg0 The type of the first argument of F.
 * @tparam Arg1 The type of the second argument of F.
 * @tparam Arg2 The type of the third argument of F.
 * @param fn the function to bind.
 * @param arg0 the first argument to bind.
 * @param arg1 the second argument to bind.
 * @param arg1 the third argument to bind.
 * @return a thunk binding F, arg0, arg1 and arg2.
 */
template<typename F, typename Arg0, typename Arg1, typename Arg2>
detail::Thunk_3<typename detail::FunctionAdaptor<F>::type, Arg0, Arg1, Arg2>
make_thunk(const F& fn, const Arg0& arg0, const Arg1& arg1, const Arg2& arg2) {
	typedef typename detail::FunctionAdaptor<F>::type fn_adaptor_t;
	return detail::Thunk_3<fn_adaptor_t, Arg0, Arg1, Arg2>(
		fn_adaptor_t(fn),
		arg0,
		arg1,
		arg2
	);
}

} // namespace eq

#endif /* EVENTQUEUE_MAKETHUNK_H */
