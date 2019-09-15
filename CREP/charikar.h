/*
Charikar algorithm for alpha-LSP.
*/

#pragma once

#include <iostream>
#include <vector>
#include <queue>
#include <numeric>

#include "typedefs.h"
#include "graph.h"
#include "graph_extended.h"
#include "utility.h"
#include "debug.h"

namespace crep
{
	inline std::vector<real_> charikar_age(bool& found_mergeable, graph_extended& gr, const real_ th, real_& sg_weight, const std::vector<int32_t>& parent = {})
	{
		pq_pair_real pq{ compare_pair_greater_real };

		int_ v = gr.v();
		real_ cur_total_weight = gr.get_total_weight();
		real_ cur_den = graph::calc_property(v, cur_total_weight);

		//Vector that stores whether an element is or is not part of the subgraph,
		//an entry of -1 denotes exclusion
		std::vector<real_> pq_elem(v, -1);

		//Consider all components (!= vertices)
		for (int_ i = 0; i < v; i++)
		{
			if (parent.empty() || i == parent[i])
			{
				pq_elem[i] = gr.get_deg(i);
				pq.emplace(pq_elem[i], i);
			}
		}

		//Remove the vertex of minimum degree as long as the current subgraph has CREP-density smaller than th
		while (!pq.empty() && cur_den < th && v > 0)
		{
			//Remove from the priority all elements with outdated key,
			//then get the next vertex to remove from the current subgraph
			std::pair<real_, id_> smallest_deg;
			while (!pq.empty() && pq_elem[(smallest_deg = pq.top()).second] != smallest_deg.first) //TODO: floating point inaccuracies
			{
				pq.pop();
			}
			if (pq.empty() || pq_elem[smallest_deg.second] < 0)
			{
				//Last element in queue was outdated -> simply exit the loop
				break;
			}
			else
			{
				pq.pop();
			}

			auto cur = smallest_deg.second;
			pq_elem[cur] = -1;
			--v;
			cur_total_weight -= smallest_deg.first;

			//Update all neighbors of v
			for (auto [neighbor_id, neighbor_weight, neighbor_updated] : gr.get_adj_naged(cur))
			{
				if (pq_elem[neighbor_id] >= -1e-8)
				{
					pq_elem[neighbor_id] -= get_aged_value(neighbor_weight, neighbor_updated);
					pq.emplace(pq_elem[neighbor_id], neighbor_id);
				}
			}

			cur_den = graph::calc_property(v, cur_total_weight);
		}

		if (cur_den < th)
		{
			found_mergeable = false;
			return {};
		}
		else
		{
			found_mergeable = true;
			sg_weight = cur_total_weight;
			return pq_elem;
		}
	}
}

// Copyright (c) 2019 Martin Schonger