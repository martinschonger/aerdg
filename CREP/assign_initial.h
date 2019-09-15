/*
Strategies for computing an initial partitioning.
*/

#pragma once

#include <vector>
#include <algorithm>
#include <numeric>
#include <random>

#include "typedefs.h"

namespace crep
{
	//Fill the partitions one by one, starting from partition 0
	void assign_initial_default(const size_& num_v, const size_& num_p, const int_& p_size, std::vector<int_>& ps, std::vector<id_>& vtp)
	{
		size_ cur_vtx = 0;
		for (size_ i = 0; i < num_p && cur_vtx < num_v; i++)
		{
			for (size_ j = 0; j < p_size && cur_vtx < num_v; j++)
			{
				vtp[cur_vtx] = i;
				++ps[i];
				++cur_vtx;
			}
		}
	}

	//Randomly distribute all vertices across all available partitions using the random engine g
	void assign_initial_random(const size_& num_v, const size_& num_p, const int_& p_size, std::vector<int_>& ps, std::vector<id_>& vtp, std::default_random_engine& g)
	{
		std::fill(ps.begin(), ps.end(), 0);
		std::fill(vtp.begin(), vtp.end(), -1);

		std::vector<id_> v(num_v);
		std::iota(v.begin(), v.end(), 0);
		std::shuffle(v.begin(), v.end(), g);

		id_ cur_part = -1;
		for (int_ i = 0; i < num_v; i++)
		{
			if (i % p_size == 0)
			{
				++cur_part;
			}

			vtp[v[i]] = cur_part;
			++ps[cur_part];
		}
	}
}

// Copyright (c) 2019 Martin Schonger