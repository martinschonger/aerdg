/*
Implementation of disjoint-set data structure declared in union_find.h.
*/

#include "union_find.h"

using namespace std;

crep::union_find::union_find(int32_t num_elements)
{
	parent = vector<int32_t>(num_elements);
	iota(parent.begin(), parent.end(), 0);
	size = vector<int32_t>(num_elements, 1);
	next = vector<int32_t>(parent);
}

crep::union_find::~union_find()
{}

int32_t crep::union_find::f(int32_t a)
{
	//Compute the root of 'a'
	int32_t root = a;
	int32_t parent_tmp;
	while (true)
	{
		parent_tmp = parent[root];
		if (parent_tmp == root)
		{
			break;
		}
		root = parent_tmp;
	}

	//Compress path
	int32_t current = a;
	int32_t next_elem;
	while (current != root)
	{
		next_elem = parent[current];
		parent[current] = root;
		current = next_elem;
	}

	return root;
}

void crep::union_find::u(int32_t a_in, int32_t b_in)
{
	int32_t a = f(a_in);
	int32_t b = f(b_in);

	if (a == b)
	{
		return;
	}

	//Update parent and size entries
	int32_t a_size = size[a];
	int32_t b_size = size[b];
	if (a_size < b_size)
	{
		int32_t tmp = a;
		a = b;
		b = tmp;
	}
	parent[b] = a;
	size[a] = a_size + b_size;

	//Update next pointers
	int32_t tmp = next[a_in];
	next[a_in] = next[b_in];
	next[b_in] = tmp;
}

void crep::union_find::add_elems(const int32_t num_add_elems, const int32_t num_actual_add_elems)
{
	if (parent.capacity() < parent.size() + num_actual_add_elems)
	{
		parent.reserve(parent.size() + num_add_elems);
		size.reserve(size.size() + num_add_elems);
	}

	if (num_actual_add_elems > 1)
	{
		parent.resize(parent.size() + num_actual_add_elems);
		iota(parent.begin() + (parent.size() - num_actual_add_elems), parent.end(), (parent.size() - num_actual_add_elems));
		next.resize(next.size() + num_actual_add_elems);
		iota(next.begin() + (next.size() - num_actual_add_elems), next.end(), (next.size() - num_actual_add_elems));
		size.resize(size.size() + num_actual_add_elems);
		fill(size.begin() + (size.size() - num_actual_add_elems), size.end(), 1);
	}
	else
	{
		parent.emplace_back(parent.size());
		next.emplace_back(next.size());
		size.emplace_back(1);
	}
}

int32_t crep::union_find::get_size(const int32_t a)
{
	return size[f(a)];
}

void crep::union_find::reset(const int32_t a)
{
	parent[a] = a;
	next[a] = a;
	size[a] = 1;
}

void crep::union_find::reset()
{
	for (int32_t i = 0; i < parent.size(); i++)
	{
		reset(i);
	}
}

// Copyright (c) 2019 Martin Schonger