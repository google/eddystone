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
#ifndef EVENTQUEUE_DETAIL_FUNCTIONADAPTOR_H_
#define EVENTQUEUE_DETAIL_FUNCTIONADAPTOR_H_

#include "MemberFunctionAdaptor.h"

namespace eq {
namespace detail {

/**
 * In C++, several types can be used as function:
 *   - function pointer
 *   - member functions
 *   - function like object
 * While function pointer and function like object can be used with the function
 * call syntax, the function call syntax can't be applied for function pointers.
 * This meta function yield takes a callable type F in input and as a result
 * return a type which can be constructed from F and used with the function call
 * syntax.
 *
 * \code
 * class Foo;
 *
 * Foo foo;
 * typedef void (Foo::*foo_function_t)();
 * foo_function_t foo_function = &Foo::some_function;
 *
 * //The following will fail:
 * //foo_function(foo)
 *
 * typedef FunctionAdaptor<foo_function_t>::type foo_function_adaptor_t;
 * foo_function_adaptor_t foo_function_adapted(foo_function);
 * foo_function_adapted(foo);
 *
 * \endcode
 *
 * \tparam F The type of the object to adapt.
 */
template<typename F>
struct FunctionAdaptor {
	/**
	 * Common case (function pointer and function like object).
	 * Yield itself, no addaptation needed.
	 */
	typedef F type;
};

/**
 * Partial specializetion for member function with no arguments
 */
template<typename T>
struct FunctionAdaptor<void(T::*)()> {
	/**
	 * Yield a member function adaptor.
	 */
	typedef MemberFunctionAdaptor0<T> type;
};

/**
 * Partial specializetion for member function with one argument
 */
template<typename T, typename Arg0>
struct FunctionAdaptor<void(T::*)(Arg0)> {
	/**
	 * Yield a member function adaptor.
	 */
	typedef MemberFunctionAdaptor1<T, Arg0> type;
};

/**
 * Partial specializetion for member function with two arguments
 */
template<typename T, typename Arg0, typename Arg1>
struct FunctionAdaptor<void(T::*)(Arg0, Arg1)> {
	/**
	 * Yield a member function adaptor.
	 */
	typedef MemberFunctionAdaptor2<T, Arg0, Arg1> type;
};

} // namespace detail
} // namespace eq

#endif /* EVENTQUEUE_DETAIL_FUNCTIONADAPTOR_H_ */
