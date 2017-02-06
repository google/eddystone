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
#ifndef EVENTQUEUE_DETAIL_MEMBERFUNCTIONADAPTOR_H_
#define EVENTQUEUE_DETAIL_MEMBERFUNCTIONADAPTOR_H_

namespace eq {
namespace detail {

/**
 * Adaptor for member function without argument.
 * It wrap member function into a function like object to make it usable like
 * a regular function.
 * \tparam T the type the class/struct holding the member function.
 * \code
 * struct Foo {
 * 	void fn();
 * };
 *
 * Foo foo;
 * MemberFunctionAdaptor0<Foo> fn_adapted(&Foo::fn);
 *
 *  fn_adapted(foo); // work
 *  fn_adapted(&foo); // work
 * \endcode
 */
template<typename T>
struct MemberFunctionAdaptor0 {
	/**
	 * Construct a member function adaptor.
	 * \param fn The member function to addapt.
	 */
	MemberFunctionAdaptor0(void (T::*fn)()) :
		_fn(fn) {
	}

	/**
	 * Call operator for pointer of T
	 */
	void operator()(T* self) const {
		(self->*_fn)();
	}

	/**
	 * Call operator for reference of T
	 */
	void operator()(T& self) const {
		(self.*_fn)();
	}

private:
	void (T::* const _fn)();
};


/**
 * Adaptor for member function with one argument.
 * It wrap member function into a function like object to make it usable like
 * a regular function.
 * \tparam T the type the class/struct holding the member function.
 * \code
 * struct Foo {
 * 	void fn(int);
 * };
 *
 * Foo foo;
 * MemberFunctionAdaptor1<Foo> fn_adapted(&Foo::fn);
 *
 *  fn_adapted(foo, 42); // work
 *  fn_adapted(&foo, 42); // work
 * \endcode
 */
template<typename T, typename Arg0>
struct MemberFunctionAdaptor1 {
	/**
	 * Construct a member function adaptor.
	 * \param fn The member function to addapt.
	 */
	MemberFunctionAdaptor1(void (T::*fn)(Arg0)) :
		_fn(fn) {
	}

	/**
	 * Call operator for pointer of T
	 */
	void operator()(T* self, Arg0 arg0) const {
		(self->*_fn)(arg0);
	}

	/**
	 * Call operator for reference of T
	 */
	void operator()(T& self, Arg0 arg0) const {
		(self.*_fn)(arg0);
	}

private:
	void (T::* const _fn)(Arg0);
};


/**
 * Adaptor for member function with two arguments.
 * It wrap member function into a function like object to make it usable like
 * a regular function.
 * \tparam T the type the class/struct holding the member function.
 * \code
 * struct Foo {
 * 	void fn(int, const char*);
 * };
 *
 * Foo foo;
 * MemberFunctionAdaptor2<Foo> fn_adapted(&Foo::fn);
 *
 *  fn_adapted(foo, 42, "toto"); // work
 *  fn_adapted(&foo, 42, "toto"); // work
 * \endcode
 */
template<typename T, typename Arg0, typename Arg1>
struct MemberFunctionAdaptor2 {
	/**
	 * Construct a member function adaptor.
	 * \param fn The member function to addapt.
	 */
	MemberFunctionAdaptor2(void (T::*fn)(Arg0, Arg1)) : _fn(fn) { }

	/**
	 * Call operator for pointer of T
	 */
	void operator()(T* self, Arg0 arg0, Arg1 arg1) const {
		(self->*_fn)(arg0, arg1);
	}

	/**
	 * Call operator for reference of T
	 */
	void operator()(T& self, Arg0 arg0, Arg1 arg1) const {
		(self.*_fn)(arg0, arg1);
	}

private:
	void (T::* const _fn)(Arg0, Arg1);
};

} // namespace detail
} // namespace eq

#endif /* EVENTQUEUE_DETAIL_MEMBERFUNCTIONADAPTOR_H_ */
