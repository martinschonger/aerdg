/*
Basic global types.
*/

#pragma once

#include <cstdint>
#include <queue>

namespace crep
{
	typedef int64_t id_;
	typedef int64_t size_;
	typedef int64_t int_;
	typedef double real_;
	typedef int64_t tstamp_;

	//Type for values subject to aging: aged real
	typedef struct
	{
		real_ val;
		tstamp_ modified;
	} areal_;

	//Priority queue for pairs (key, value)
	typedef bool (*compare_pair)(const std::pair<int_, id_>&, const std::pair<int_, id_>&);
	typedef std::priority_queue<std::pair<int_, id_>, std::vector<std::pair<int_, id_>>, compare_pair> pq_pair;
	inline bool compare_pair_greater(const std::pair<int_, id_>& a, const std::pair<int_, id_>& b)
	{
		return a.first > b.first;
	};
	inline bool compare_pair_less(const std::pair<int_, id_>& a, const std::pair<int_, id_>& b)
	{
		return a.first < b.first;
	};

	//Priority queue for pairs (aged key, value)
	typedef bool (*compare_pair_real)(const std::pair<real_, id_>&, const std::pair<real_, id_>&);
	typedef std::priority_queue<std::pair<real_, id_>, std::vector<std::pair<real_, id_>>, compare_pair_real> pq_pair_real;
	inline bool compare_pair_greater_real(const std::pair<real_, id_>& a, const std::pair<real_, id_>& b)
	{
		return a.first > b.first;
	};
	inline bool compare_pair_less_real(const std::pair<real_, id_>& a, const std::pair<real_, id_>& b)
	{
		return a.first < b.first;
	};
}

// Copyright (c) 2019 Martin Schonger