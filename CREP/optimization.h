#pragma once

#include <vector>
#include <unordered_map>
#include <queue>

#include "typedefs.h"
#include "charikar.h"
#include "debug.h"

namespace crep
{
	template <typename T>
	void flex_resize(std::vector<T>& v, int32_t min_size, const T fill_value, const int32_t increment = 32)
	{
		if (min_size <= (int32_t)v.size())
		{
			return;
		}

		if ((int32_t)v.capacity() < min_size)
		{
			v.reserve(v.capacity() + increment);
		}
		int32_t old_size = v.size();
		v.resize(min_size);
		auto tmp_iter = v.begin();
		std::advance(tmp_iter, old_size);
		std::fill(tmp_iter, v.end(), fill_value);
	}

	void dfs(const std::vector<std::vector<edge>>& adj, int32_t s, std::unordered_map<int32_t, int32_t>& m, int32_t& cur_id, graph& gr)
	{
		std::vector<bool> visited(32, false);
		std::stack<int32_t> stack;

		m.emplace(s, cur_id++);
		gr.add_node();
		flex_resize(visited, cur_id, false);

		stack.push(s);

		while (!stack.empty())
		{
			s = stack.top();
			stack.pop();

			auto search_iter = m.find(s);
			if (search_iter != m.end())
			{
				if (!visited[search_iter->second])
				{
					visited[search_iter->second] = true;
				}
				else
				{
					continue;
				}
			}
			else
			{
				//should not happen
			}

			for (auto i = adj[s].begin(); i != adj[s].end(); ++i)
			{
				auto search_iter2 = m.find(i->target);
				if (search_iter2 != m.end())
				{
					if (!visited[search_iter2->second])
					{
						//add edge
						gr.update_edge(search_iter->second, search_iter2->second, 1); //FIXME: i->weight, set_edge

						stack.push(i->target);
					}
				}
				else
				{
					m.emplace(i->target, cur_id++);
					gr.add_node();
					flex_resize(visited, cur_id, false);

					//add edge
					gr.update_edge(search_iter->second, cur_id - 1, 1); //FIXME: i->weight, set_edge

					stack.push(i->target);
				}
			}
		}
	}

	bool check_precondition_mergeable(const graph& gr, const int32_t& a, const double& alpha)
	{
		std::unordered_map<int32_t, int32_t> m;
		int32_t cur_id = 0;
		graph connected_component;

		dfs(gr.get_adj(), a, m, cur_id, connected_component);

		bool found_mergeable = false;
		std::vector<int32_t> parent_tmp(connected_component.v());
		std::iota(parent_tmp.begin(), parent_tmp.end(), 0);
		auto ignored = charikar_old(found_mergeable, connected_component, parent_tmp, alpha);

		return found_mergeable;
	}

	id_ get_sid(const id_& id, graph& subg, std::map<id_, id_>& g_to_subg, std::vector<id_>& subg_to_g)
	{
		auto subg_id_iter = g_to_subg.find(id);
		if (subg_id_iter != g_to_subg.end())
		{
			return subg_id_iter->second;
		}
		else
		{
			int_ subg_id = g_to_subg.size();
			g_to_subg.emplace(id, subg_id);
			subg_to_g.emplace_back(id);
			subg.add_node();
			return subg_id;
		}
	}

	void resize_visited(std::vector<bool>& visited, const id_& id)
	{
		visited.resize(std::max((long long int)visited.size(), (long long int)(id + 1)), false);
	}

	void bfs_direct_nb(
		const graph& g, 
		const id_& src, 
		const id_& dst, 
		graph& subg, 
		std::map<id_, id_>& g_to_subg, 
		std::vector<id_>& subg_to_g)
	{
		std::vector<bool> visited;
		std::queue<std::pair<id_, int_>> q;
		q.emplace(src, 0);
		q.emplace(dst, 0);

		while (!q.empty())
		{
			auto [cur, depth] = q.front();
			q.pop();

			id_ cur_id = get_sid(cur, subg, g_to_subg, subg_to_g);
			resize_visited(visited, cur_id);
			if (!visited[cur_id])
			{
				visited[cur_id] = true;

				for (const auto& e : g.get_adj(cur))
				{
					if (depth < 1)
					{
						id_ target_id = get_sid(e.target, subg, g_to_subg, subg_to_g);
						resize_visited(visited, target_id);
						if (!visited[target_id])
						{
							q.emplace(e.target, depth + 1);
							subg.set_edge(cur_id, target_id, e.weight);
						}
					}
					else if (depth == 1)
					{
						if (g_to_subg.find(e.target) != g_to_subg.end())
						{
							id_ target_id = get_sid(e.target, subg, g_to_subg, subg_to_g);
							resize_visited(visited, target_id);
							if (visited[target_id])
							{
								subg.set_edge(cur_id, target_id, e.weight);
							}
						}
					}
				}
			}
		}
	}

	std::vector<int32_t> run_charikar_on_direct_nb(const graph& g, const id_& src, const id_& dst, const double& alpha, bool& found_mergeable)
	{
		graph subg;
		std::map<id_, id_> g_to_subg;
		std::vector<id_> subg_to_g;

		bfs_direct_nb(g, src, dst, subg, g_to_subg, subg_to_g);

		std::vector<int32_t> parent_tmp(subg.v());
		std::iota(parent_tmp.begin(), parent_tmp.end(), 0);
		auto res_tmp = charikar_old(found_mergeable, subg, parent_tmp, alpha);

		//res.id to g.id
		if (!found_mergeable)
		{
			return {};
		}

		std::vector<int32_t> res(g.v(), -1);
		for (int_ i = 0; i < res_tmp.size(); i++)
		{
			if (res_tmp[i] != -1)
			{
				res[subg_to_g[i]] = res_tmp[i];
			}
		}

		return res;
	}
}

// Copyright (c) 2019 Martin Schonger