/*
Strategies for exploring local neighborhoods with single vertices or edges as seed.
*/

#pragma once

#include <vector>
#include <map>
#include <limits>

#include "typedefs.h"
#include "graph_extended.h"

namespace crep
{
	//Helper function for dynamically constructing a graph structure.
	//If 'id' has not yet been added to the 'subg', add a corresponding vertex to 'subg'.
	//The mapping of the vertices of original graph and subgraph is represented by 'g_to_subg' and 'subg_to_g'
	inline id_ get_sid(
		const id_& id,
		graph_extended& subg,
		std::map<id_, id_>& g_to_subg,
		std::vector<id_>& subg_to_g)
	{
		auto subg_id_iter = g_to_subg.find(id);
		if (subg_id_iter != g_to_subg.end())
		{
			//'id' has already been added to 'subg'
			return subg_id_iter->second;
		}
		else
		{
			//Add a vertex to 'subg' which corresponds to 'id' in the original graph
			int_ subg_id = g_to_subg.size();
			g_to_subg.emplace(id, subg_id);
			subg_to_g.emplace_back(id);
			subg.add_nodes(32, 1);
			return subg_id;
		}
	}

	//Compute the (max-depth)-hop neighborhood of edge (src,dst).
	//The result is returned via 'subg', 'g_to_subg' and 'subg_to_g'
	inline void explore_hop(
		graph_extended& g,
		const id_& src,
		const id_& dst,
		const int_ max_depth,
		graph_extended& subg,
		std::map<id_, id_>& g_to_subg,
		std::vector<id_>& subg_to_g)
	{
		//Store the seen but not visited vertices along with their minimal distance (in hops) from the edge (src,dst)
		std::queue<std::pair<id_, int_>> q;
		q.emplace(src, 0);
		q.emplace(dst, 0);

		while (!q.empty())
		{
			auto [cur, depth] = q.front();
			q.pop();

			if (g.visited[cur] != util::CUR_TIME)
			{
				//Mark vertex as visited
				g.visited[cur] = util::CUR_TIME;

				//If not already present, add a vertex corresponding to 'cur' to the result
				id_ cur_id = get_sid(cur, subg, g_to_subg, subg_to_g);

				//Consider all direct neighbors of 'cur'
				for (const auto& e : g.get_adj_naged(cur))
				{
					if (g.visited[e.target] == util::CUR_TIME)
					{
						//If not already present, add a vertex corresponding to 'e.target' to the result
						id_ target_id = get_sid(e.target, subg, g_to_subg, subg_to_g);

						//Add the edge if both endpoints are in 'subg'
						subg.set_edge(cur_id, target_id, get_aged_value(e.weight, e.modified));
					}
					else if (depth < max_depth)
					{
						//Mark vertex 'e.target' as seen if depth limit permits it
						q.emplace(e.target, depth + 1);
					}
				}
			}
		}
	}

	//Compute greedily the max_num_vtxs neighbors of (src,dst) based on the degree of a vertex in the base_graph.
	//The result is returned via 'subg', 'g_to_subg' and 'subg_to_g'
	inline void explore_greedy_deg(
		graph_extended& g,
		const id_& src,
		const id_& dst,
		const int_ max_num_vtxs,
		graph_extended& subg,
		std::map<id_, id_>& g_to_subg,
		std::vector<id_>& subg_to_g)
	{
		//Store the seen but not visited vertices along with their degree
		pq_pair_real pq{ compare_pair_less_real };
		pq.emplace(std::numeric_limits<real_>::max(), src);
		pq.emplace(std::numeric_limits<real_>::max(), dst);

		while (!pq.empty() && subg.v() < max_num_vtxs)
		{
			auto [deg, cur] = pq.top();
			pq.pop();

			if (g.visited[cur] != util::CUR_TIME)
			{
				//Mark vertex as visited
				g.visited[cur] = util::CUR_TIME;

				//If not already present, add a vertex corresponding to 'cur' to the result
				id_ cur_id = get_sid(cur, subg, g_to_subg, subg_to_g);

				//Consider all direct neighbors of 'cur'
				for (const auto& e : g.get_adj_naged(cur))
				{
					if (g.visited[e.target] != util::CUR_TIME)
					{
						//Mark vertex as seen if not already visited
						pq.emplace(g.get_deg(e.target), e.target);
					}
					else if (g.visited[e.target] == util::CUR_TIME)
					{
						//If not already present, add a vertex corresponding to 'e.target' to the result
						id_ target_id = get_sid(e.target, subg, g_to_subg, subg_to_g);

						//Add the edge if both endpoints are in 'subg'
						subg.set_edge(cur_id, target_id, get_aged_value(e.weight, e.modified));
					}
				}
			}
		}
	}

	//Compute greedily the max_num_vtxs neighbors of (src,dst) based on the degree of a vertex within to the so-far constructed subgraph.
	//The result is returned via 'subg', 'g_to_subg' and 'subg_to_g'
	//Note: this method has not been fully tested.
	inline void explore_greedy_deg_subg(
		graph_extended& g,
		const id_& src,
		const id_& dst,
		const int_ max_num_vtxs,
		graph_extended& subg,
		std::map<id_, id_>& g_to_subg,
		std::vector<id_>& subg_to_g)
	{
		//Store the seen but not visited vertices along with their degree to 'subg'
		pq_pair_real pq{ compare_pair_less_real };
		pq.emplace(std::numeric_limits<real_>::max(), src);
		pq.emplace(std::numeric_limits<real_>::max(), dst);

		std::map<id_, real_> cur_deg;

		while (!pq.empty() && subg.v() < max_num_vtxs)
		{
			auto [deg, cur] = pq.top();
			pq.pop();

			if (g.visited[cur] != util::CUR_TIME)
			{
				//Mark vertex as visited
				g.visited[cur] = util::CUR_TIME;

				//If not already present, add a vertex corresponding to 'cur' to the result
				id_ cur_id = get_sid(cur, subg, g_to_subg, subg_to_g);

				//Consider all direct neighbors of 'cur'
				for (const auto& e : g.get_adj_naged(cur))
				{
					if (g.visited[e.target] != util::CUR_TIME)
					{
						//If the vertex has not ben visited, check if we have seen it before and 
						//possibly update the current degree (with respect to 'subg')
						auto existing_entry = cur_deg.find(e.target);
						if (existing_entry == cur_deg.end())
						{
							cur_deg.emplace(e.target, get_aged_value(e.weight, e.modified));
						}
						else
						{
							existing_entry->second += get_aged_value(e.weight, e.modified);
						}
						pq.emplace(existing_entry->second, e.target);
					}
					else if (g.visited[e.target] == util::CUR_TIME)
					{
						//If not already present, add a vertex corresponding to 'e.target' to the result
						id_ target_id = get_sid(e.target, subg, g_to_subg, subg_to_g);

						//Add the edge if both endpoints are in 'subg'
						subg.set_edge(cur_id, target_id, get_aged_value(e.weight, e.modified));
					}
				}
			}
		}
	}

	//Compute the connected component of vertex 'src'.
	//The result is returned via 'subg', 'g_to_subg' and 'subg_to_g'
	inline void explore_cc(
		graph_extended& g,
		const id_& src,
		graph_extended& subg,
		std::map<id_, id_>& g_to_subg,
		std::vector<id_>& subg_to_g)
	{
		//Store the seen but not visited vertices
		std::queue<id_> q;
		q.emplace(src);

		while (!q.empty())
		{
			auto cur = q.front();
			q.pop();

			if (g.visited[cur] != util::CUR_TIME)
			{
				//Mark vertex as visited
				g.visited[cur] = util::CUR_TIME;

				//If not already present, add a vertex corresponding to 'cur' to the result
				id_ cur_id = get_sid(cur, subg, g_to_subg, subg_to_g);

				//Consider all direct neighbors of 'cur'
				for (const auto& e : g.get_adj_naged(cur))
				{
					if (g.visited[e.target] != util::CUR_TIME)
					{
						//Mark vertex as seen if not already visited
						q.emplace(e.target);
					}
					else if (g.visited[e.target] == util::CUR_TIME)
					{
						//If not already present, add a vertex corresponding to 'e.target' to the result
						id_ target_id = get_sid(e.target, subg, g_to_subg, subg_to_g);

						//Add the edge if both endpoints are in 'subg'
						subg.set_edge(cur_id, target_id, get_aged_value(e.weight, e.modified));
					}
				}
			}
		}
	}
}

// Copyright (c) 2019 Martin Schonger