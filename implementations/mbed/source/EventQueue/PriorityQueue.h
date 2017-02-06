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
#ifndef EVENTQUEUE_STACKPRIORITYQUEUE_H_
#define EVENTQUEUE_STACKPRIORITYQUEUE_H_

#include <cstddef>
#include "AlignedStorage.h"

namespace eq {

/**
 * Priority queue of Ts.
 * Ts are ordered from the smaller to the bigger ( < ).
 * Elements in the queue are mutable (this is a design choice).
 * After a mutation the function update should be called to ensure that the
 * queue is still properly sorted.
 * @tparam T type of elements in this queue
 * @param capacity Number of elements that this queue can contain
 */
template<typename T, std::size_t Capacity>
class PriorityQueue {

public:
	/**
	 * Type of the nodes in this queue.
	 */
	struct Node {
		AlignedStorage<T> storage;		/// storage for the T
		Node* next;						/// pointer to the next node
	};

	/**
	 * Iterator for elements of the queue.
	 */
	class Iterator {
		friend PriorityQueue;

		/// Construct an iterator from a Node.
		/// This constructor is private and can only be invoked from the PriorityQueue.
		Iterator(Node* current) :
			_current(current) {
		}

	public:

		/// Indirection operator.
		/// return a reference to the inner T
		T& operator*() {
			return _current->storage.get();
		}

		/// Const version of indirection operator.
		/// return a reference to the inner T
		const T& operator*() const {
			return _current->storage.get();
		}

		/// dereference operator.
		/// Will invoke the operation on the inner T
		T* operator->() {
			return &(_current->storage.get());
		}

		/// const dereference operator.
		/// Will invoke the operation on the inner T
		const T* operator->() const {
			return &(_current->storage.get());
		}

		/// pre incrementation to the next T in the list
		Iterator& operator++() {
			_current = _current->next;
			return *this;
		}

		/// post incrementation to the next T in the list
		Iterator operator++(int) {
			Iterator tmp(*this);
			_current = _current->next;
			return tmp;
		}

		/// Equality operator
		friend bool operator==(const Iterator& lhs, const Iterator& rhs) {
			return lhs._current == rhs._current;
		}

		/// Unequality operator
		friend bool operator!=(const Iterator& lhs, const Iterator& rhs) {
			return !(lhs == rhs);
		}

		/// return the internal node.
		Node* get_node() {
			return _current;
		}

	private:
		Node* _current;
	};

	typedef Iterator iterator;

	/// Construct an empty priority queue.
	PriorityQueue() : nodes(), free_nodes(NULL), head(NULL), used_nodes_count(0) {
		initialize();
	}

	/// Copy construct a priority queue.
	/// The queue will have the same content has other.
	PriorityQueue(const PriorityQueue& other) :
		nodes(), free_nodes(NULL), head(NULL), used_nodes_count(0) {
		initialize();
		copy(other);
	}

	/// destroy a priority queue.
	~PriorityQueue() {
		clear();
	}

	/// Copy assignemnent from another priority queue.
	/// The content of the queue will be destroyed then the content from
	/// other will be copied.
	PriorityQueue& operator=(const PriorityQueue& other) {
		if (&other == this) {
			return *this;
		}
		copy(other);
		return *this;
	}

	/// Push a new element to the queue.
	/// It will be added before the first element p in the queue where
	/// element < p == true.
	/// @return An iterator to the inserted element.
	iterator push(const T& element) {
		if (full()) {
			return NULL;
		}

		// get a free node
		Node* new_node = free_nodes;
		free_nodes = free_nodes->next;
		new_node->next = NULL;

		++used_nodes_count;

		// copy content
		new (new_node->storage.get_storage()) T(element);

		// if there is no node in the queue, just link the head
		// to the new node and return
		if (head == NULL) {
			head = new_node;
			return new_node;
		}

		// if the new node has an higher priority than the node in head
		// just link it as the head
		if (element < head->storage.get()) {
			new_node->next = head;
			head = new_node;
			return new_node;
		}

		// insert the node after head
		insert_after(head, new_node);

		return new_node;
	}

	/// pop the head of the queue.
	bool pop() {
		if (!head) {
			return false;
		}

		Node* target = head;
		target->storage.get().~T();
		head = target->next;
		target->next = free_nodes;
		free_nodes = target;
		--used_nodes_count;
		return true;
	}

	/// If the content of an element is updated is updated after the insertion
	/// then, the list can be in an unordered state.
	/// This function help; it update the position of an iterator in the list.
	void update(iterator it) {
		Node* target = it.get_node();
		Node* hint = head;

		if (target == NULL) {
			return;
		}

		// remove the node from the list
		if (target == head) {
			// if it is the only node in the list, just return
			// it is not needed to update its position
			if (target->next == NULL) {
				return;
			}

			// if the order is already correct, just return
			if (target->storage.get() < target->next->storage.get()) {
				return;
			}

			// otherwise remove the node from the list
			// and update the hint
			head = target->next;
			hint = head;
		} else {
			bool node_found = false;
			for (Node* current = head; current != NULL; current = current->next) {
				if (current->next == target) {
					// check if it is needed to move the node
					if (current->storage.get() < target->storage.get()) {
						if (target->next == NULL) {
							return;
						}

						if (target->storage.get() < target->next->storage.get()) {
							return;
						}

						// there is no need to iterate again the whole list
						// just mark the hint has the current node
						hint = current;
					}

					// remove the node from the list and break out of the loop
					current->next = target->next;
					node_found = true;
					break;
				}
			}

			// the node in parameter doesn't belong to this queue
			if (!node_found) {
				return;
			}
		}

		// insert the node after hint
		insert_after(hint, target);
	}

	/// return an iterator to the begining of the queue.
	iterator begin() {
		return head;
	}

	/// return an iterator to the end of the queue.
	/// @note can't be dereferenced
	iterator end() {
		return NULL;
	}

	/// erase an iterator from the list
	bool erase(iterator it) {
		return erase(it.get_node());
	}

	/// erase a node from the list
	bool erase(Node* n) {
		if (n == NULL) {
			return false;
		}

		if (head == n) {
			return pop();
		}

		Node* current = head;
		while (current->next) {
			if (current->next == n) {
				current->next = n->next;
				n->storage.get().~T();
				n->next = free_nodes;
				free_nodes = n;
				--used_nodes_count;
				return true;
			}
			current = current->next;
		}
		return false;
	}

	/**
	 * Indicate if the queue is empty or not.
	 * @return true if the queue is empty and false otherwise.
	 * @invariant the queue remains untouched.
	 */
	bool empty() const {
		return head == NULL;
	}

	/**
	 * Indicate if the true is full or not.
	 * @return true if the queue is full and false otherwise.
	 * @invariant the queue remains untouched.
	 */
	bool full() const {
		return free_nodes == NULL;
	}

	/**
	 * Indicate the number of elements in the queue.
	 * @return the number of elements currently held by the queue.
	 * @invariant the queue remains untouched.
	 */
	std::size_t size() const {
		return used_nodes_count;
	}

	/**
	 * Expose the capacity of the queue in terms of number of elements the
	 * queue can hold.
	 * @return the capacity of the queue.
	 * @invariant this function should always return Capacity.
	 */
	std::size_t capacity() const {
		return Capacity;
	}

	/**
	 * Clear the queue from all its elements.
	 */
	void clear() {
		while (head) {
			head->storage.get().~T();
			Node* tmp = head;
			head = head->next;
			tmp->next = free_nodes;
			free_nodes = tmp;
		}
		used_nodes_count = 0;
	}

private:
	void initialize() {
		/// link all the nodes together
		for (std::size_t i = 0; i < (Capacity - 1); ++i) {
			nodes[i].next = &nodes[i + 1];
		}
		/// the last node does not have a next node
		nodes[Capacity - 1].next = NULL;
		/// set all the nodes as free
		free_nodes = nodes;
	}

	void copy(const PriorityQueue& other) {
		if (empty() == false) {
			clear();
		}

		Node *to_copy = other.head;
		Node *previous = NULL;
		while (to_copy) {
			// pick a free node
			Node* new_node = free_nodes;
			free_nodes = free_nodes->next;
			new_node->next = NULL;

			// copy content
			new (new_node->storage.get_storage()) T(to_copy->storage.get());

			// link into the queue or update head then update previous pointer
			if (previous) {
				previous->next = new_node;
			} else {
				head = new_node;
			}
			previous = new_node;

			// update the node to copy
			to_copy = to_copy->next;
		}
		used_nodes_count = other.used_nodes_count;
	}

	void insert_after(Node* prev, Node* to_insert) {
		for (; prev != NULL; prev = prev->next) {
			if (prev->next == NULL || to_insert->storage.get() < prev->next->storage.get()) {
				to_insert->next = prev->next;
				prev->next = to_insert;
				break;
			}
		}
	}

	Node nodes[Capacity];         //< Nodes of the queue
	Node *free_nodes;             //< entry point for the list of free nodes
	Node *head;                   //< head of the queue
	std::size_t used_nodes_count; // number of nodes used
};

} // namespace eq

#endif /* EVENTQUEUE_STACKPRIORITYQUEUE_H_ */
