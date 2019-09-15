/*
Disjoint-set data structure with (doubly-)linked list of elements for fast iteration of sets/components.
*/

#pragma once

#include <algorithm>
#include <vector>
#include <numeric>

namespace crep
{
	//template <typename T>
	class union_find
	{
	public:
		union_find(int32_t num_elements = 0);
		~union_find();

		//Get the root of element 'a'
		int32_t f(int32_t a);

		//Merge the sets containing the elements 'a_in' and 'b_in'
		void u(int32_t a_in, int32_t b_in);

		//Add 'num_actual_add_elems' to the structure and reserve 'num_add_elems' additional space 
		//if 'num_actual_add_elems' cannot be accomodated by the vectors
		void add_elems(const int32_t num_add_elems, const int32_t num_actual_add_elems = 1);

		//Get the size of the set containing 'a'
		int32_t get_size(const int32_t a);

		//Reset the vector entries for one specific element.
		//Note: does not check for consistency of other sizes, ...
		void reset(const int32_t a);

		//Reset the entire disjoint-set structure
		void reset();


		//Parent elements
		std::vector<int32_t> parent;

		//Size of sets, an entry at position i is correct iff (parent[i] == i) holds
		std::vector<int32_t> size;

		//Stores pointers to the next element in the current set
		std::vector<int32_t> next;

		//Optional: use prev pointers if bidirectional traversal is needed
		//std::vector<int32_t> prev;
	};
}

// Copyright (c) 2019 Martin Schonger