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
#ifndef EVENTQUEUE_DETAIL_ALIGNEDSTORAGE_H_
#define EVENTQUEUE_DETAIL_ALIGNEDSTORAGE_H_

#include <cstddef>

namespace eq {

class AnonymousDeclaration;

/**
 * Provide aligned raw storage for holding a type T.
 * This class is useful to delay the construction of objects while reserving
 * space in memory for them.
 * For instance, it can be use with static or global variable which can not
 * be constructed before main. It can also be used in class member which
 * do not have or can't be constructed at parent construction time.
 *
 * Once the storage has been reserved, it is possible explicitly construct the
 * object of T by using placement new syntax.
 * @code
 * AlignedStorage<Foo> foo;
 * new (foo.get_storage()) Foo(...);
 * @endcode
 * Then it is possible to get a reference to the object by calling the member
 * function get.
 * @code
 * foo.get().doSomething();
 * @endcode
 * Once the object needs to be destroyed it is possible to call the destructor
 * directly
 * @code
 * foo.get().~Foo();
 * @endcode
 * After this point, their is no instance of T in the storage and the function
 * get remains unusable until an object is again initialised  in the storage.
 */
template<typename T>
class AlignedStorage {
public:
	/**
	 * Initialisation of the storage, does **not** zeroed its memory.
	 */
	AlignedStorage() {}
	/**
	 * Provide the raw pointer to the address of the storage.
	 */
    void* get_storage() {
    	return data;
    }

	/**
	 * Provide the raw pointer to the const address of the storage.
	 */
    const void* get_storage() const {
    	return data;
    }

    /**
     * Return a reference to the element T in this storage.
     */
    T& get() {
    	return *static_cast<T*>(get_storage());
    }

    /**
     * Return a reference to the const element of T in this storage.
     */
    const T& get() const {
    	return *static_cast<const T*>(get_storage());
    }

private:
    // it doesn't make sense to allow copy construction of copy assignement for
    // this kind of object.
    AlignedStorage(const AlignedStorage&);
    AlignedStorage& operator=(const AlignedStorage&);
    // storage. Can be improved by metaprogramming to be the best fit.
	union {
        char char_storage;
        short int short_int_storage;
        int int_storage;
        long int long_int_storage;
        float float_storage;
        double double_storage;
        long double long_double_storage;
		void* pointer_storage;
        AnonymousDeclaration (*function_pointer_storage)(AnonymousDeclaration);
        AnonymousDeclaration* AnonymousDeclaration::*data_member_storage ;
        AnonymousDeclaration (AnonymousDeclaration::*function_member_storage)(AnonymousDeclaration);
		char data[sizeof(T)];
	};
};

/**
 * Provide aligned raw storage for holding an array of type T.
 * This is a specialisation of AlignedStorage for arrays of T.
 * With this class, it is possible to reserve space for a given number of
 * elements of type T and delay their construction to a latter point. This
 * feature can be really useful when building generic container which
 * embed memory for their elements. Instead of default constructing them,
 * the construction of an element can be made when it is really needed, by
 * copy. It is the same for the destruction, only objects which have been
 * constructed needs to be destructed.
 * Those properties improve generic containers because only the operations
 * which have to be made are made. It also allow generic container to hold
 * types which are not DefaultConstructible.
 *
 * Once the storage has been reserved, it is possible explicitly construct an
 * object of T at a given index by using placement new syntax.
 * @code
 * AlignedStorage<Foo[10]> foo;
 * //construct object at index 0 then at index 1.
 * new (foo.get_storage(0)) Foo(...);
 * new (foo.get_storage(1)) Foo(...);
 * @endcode
 * Then it is possible to get a reference to an object at a given index by
 * calling the member function get.
 * @code
 * // do something with object at index 1
 * foo.get(1).doSomething();
 * @endcode
 * Once the object needs to be destroyed it is possible to call the destructor
 * directly
 * @code
 * // destroy object at index 1.
 * foo.get(1).~Foo();
 * @endcode
 * After this point, their is no instance of T at index 1 in the storage and
 * trying to use the object at this index will lead to undefined behavior until
 * an object is again initialised at this index.
 */
template<typename T, std::size_t ArraySize>
struct AlignedStorage<T[ArraySize]> {
	/**
	 * Initialisation of the storage, does **not** zeroed its memory.
	 */
	AlignedStorage() {}

	/**
	 * Return raw pointer to the address of element at a given index
	 */
    void* get_storage(std::size_t index) {
    	return &get(index);
    }

	/**
	 * const version of void* get_storage(std::size_t).
	 */
    const void* get_storage(std::size_t index) const {
    	return &get(index);
    }

	/**
	 * Return reference to the element stored atindex.
	 */
    T& get(std::size_t index) {
    	return reinterpret_cast<T*>(data)[index];
    }

	/**
	 * const version of T& get(std::size_t).
	 */
    const T& get(std::size_t index) const {
    	return reinterpret_cast<const T*>(data)[index];
    }

private:
    // it doesn't make sense to allow copy construction of copy assignement for
    // this kind of object.
    AlignedStorage(const AlignedStorage&);
    AlignedStorage& operator=(const AlignedStorage&);
    // storage. Can be improved by metaprogramming to be the best fit.
	union {
        char char_storage;
        short int short_int_storage;
        int int_storage;
        long int long_int_storage;
        float float_storage;
        double double_storage;
        long double long_double_storage;
		void* pointer_storage;
        AnonymousDeclaration (*function_pointer_storage)(AnonymousDeclaration);
        AnonymousDeclaration* AnonymousDeclaration::*data_member_storage ;
        AnonymousDeclaration (AnonymousDeclaration::*function_member_storage)(AnonymousDeclaration);
		char data[sizeof(T[ArraySize])];
	};
};

} // namespace eq

#endif /* EVENTQUEUE_DETAIL_ALIGNEDSTORAGE_H_ */
