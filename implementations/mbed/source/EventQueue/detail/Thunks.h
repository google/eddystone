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
#ifndef EVENTQUEUE_DETAIL_THUNKS_H_
#define EVENTQUEUE_DETAIL_THUNKS_H_

namespace eq {
namespace detail {

/**
 * Generate a Thunk for a callable of type F with one argument.
 * This class is a function like object containing the function to call and
 * its argument. When it is invoked, F is invoked with the argument passed
 * at construction time.
 * \tparam F the type of the callable.
 * \tparam Arg0 type of the first parameter of F to pass to F.
 */
template<typename F, typename Arg0>
struct Thunk_1 {
	/**
	 * Construct the Thunk and bind its arguments.
	 * \param fn the callable, it will be invoked with arg0.
	 * \param arg0 The first argument to pass to fn when this object is called.
	 * \note member function should be adapted by using FunctionAdaptor
	 */
	Thunk_1(const F& fn, const Arg0& arg0) :
		_fn(fn), _arg0(arg0) {
	}

	/**
	 * Apply arg0 to fn.
	 */
	void operator()() const {
		_fn(_arg0);
	}

private:
	mutable F _fn;
	mutable Arg0 _arg0;
};

/**
 * Generate a Thunk for a callable of type F with two arguments.
 * This class is a function like object containing the function to call and
 * its arguments. When it is invoked, F is invoked with the arguments passed
 * at construction time.
 * \tparam F the type of the callable.
 * \tparam Arg0 type of the first parameter to pass to F.
 * \tparam Arg1 type of the second parameter to pass to F.
 */
template<typename F, typename Arg0, typename Arg1>
struct Thunk_2 {
	/**
	 * Construct the Thunk and bind its arguments.
	 * \param fn the callable, it will be invoked with arg0 and arg1.
	 * \param arg0 The first argument to pass to fn when this object is called.
	 * \param arg1 The second argument to pass to fn when this object is called.
	 * \note member function should be adapted by using FunctionAdaptor
	 */
	Thunk_2(const F& fn, const Arg0& arg0, const Arg1& arg1) :
		_fn(fn),
		_arg0(arg0),
		_arg1(arg1) {
	}

	/**
	 * Apply arg0 and arg1 to fn.
	 */
	void operator()() const {
		_fn(_arg0, _arg1);
	}

private:
	mutable F _fn;
	mutable Arg0 _arg0;
	mutable Arg1 _arg1;
};

/**
 * Generate a Thunk for a callable of type F with three arguments.
 * This class is a function like object containing the function to call and
 * its arguments. When it is invoked, F is invoked with the arguments passed
 * at construction time.
 * \tparam F the type of the callable.
 * \tparam Arg0 type of the first parameter to pass to F.
 * \tparam Arg1 type of the second parameter to pass to F.
 * \tparam Arg2 type of the third parameter to pass to F.
 */
template<typename F, typename Arg0, typename Arg1, typename Arg2>
struct Thunk_3 {
	/**
	 * Construct the Thunk and bind its arguments.
	 * \param fn the callable, it will be invoked with arg0, arg1 and arg2.
	 * \param arg0 The first argument to pass to fn when this object is called.
	 * \param arg1 The second argument to pass to fn when this object is called.
	 * \param arg2 The third argument to pass to fn when this object is called.
	 * \note member function should be adapted by using FunctionAdaptor
	 */
	Thunk_3(const F& fn, const Arg0& arg0, const Arg1& arg1, const Arg2& arg2) :
		_fn(fn),
		_arg0(arg0),
		_arg1(arg1),
		_arg2(arg2){
	}

	/**
	 * Apply arg0, arg1 and arg2 to fn.
	 */
	void operator()() const {
		_fn(_arg0, _arg1, _arg2);
	}

private:
	mutable F _fn;
	mutable Arg0 _arg0;
	mutable Arg1 _arg1;
	mutable Arg2 _arg2;
};

} // namespace detail
} // namespace eq

#endif /* EVENTQUEUE_DETAIL_THUNKS_H_ */
