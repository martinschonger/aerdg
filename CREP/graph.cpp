/*
Implementation of graph.h.
*/

#include "graph.h"

using namespace std;

crep::graph::graph(int32_t num_nodes, bool enable_logging) : enable_logging(enable_logging)
{
	num_vertices = num_nodes;
	num_actual_vertices = num_vertices;
	total_weight = { 0, util::CUR_TIME_AGE };
	deg.resize(num_nodes, { 0, util::CUR_TIME_AGE });
	adj = vector<vector<edge>>(num_nodes);
}

const vector<vector<crep::edge>>& crep::graph::get_adj_naged() const
{
	return adj;
}

const vector<crep::edge>& crep::graph::get_adj_naged(const int32_t& node) const
{
	return adj[node];
}

void crep::graph::set_edge(int32_t a, int32_t b, real_ weight)
{
	if (a == b) return;

	int32_t shorter = a;
	int32_t other = b;
	if (get_adj_naged(other).size() < get_adj_naged(shorter).size())
	{
		shorter = b;
		other = a;
	}

	//If edge (a,b) exists, remove it
	auto it = find_if(adj[shorter].begin(), adj[shorter].end(),
		[&other](const edge& element) { return element.target == other; });
	if (it != adj[shorter].end())
	{
		auto it2 = find_if(adj[other].begin(), adj[other].end(),
			[&shorter](const edge& element) { return element.target == shorter; });

		modify_aged_value(deg[shorter], -get_aged_value(it->weight, it->modified));
		modify_aged_value(deg[other], -get_aged_value(it->weight, it->modified));
		modify_aged_value(total_weight, -get_aged_value(it->weight, it->modified));

		adj[shorter].erase(it);
		adj[other].erase(it2);
	}

	//Add edge (a,b)
	adj[shorter].emplace_back(other, weight, util::CUR_TIME_AGE);
	adj[other].emplace_back(shorter, weight, util::CUR_TIME_AGE);

	modify_aged_value(deg[shorter], weight);
	modify_aged_value(deg[other], weight);
	modify_aged_value(total_weight, weight);
}

void crep::graph::update_edge(int32_t nodeA, int32_t nodeB, real_ weight_change)
{
	if (nodeA == nodeB) return;

	int32_t shorter = nodeA;
	int32_t other = nodeB;
	if (adj[nodeB].size() < adj[nodeA].size())
	{
		shorter = nodeB;
		other = nodeA;
	}

	auto it = find_if(adj[shorter].begin(), adj[shorter].end(),
		[&other](const edge& element) { return element.target == other; });

	if (it != adj[shorter].end())
	{
		//Edge (nodeA,nodeB) already exists
		auto it2 = find_if(adj[other].begin(), adj[other].end(),
			[&shorter](const edge& element) { return element.target == shorter; });

		modify_aged_value(it->weight, it->modified, weight_change);
		modify_aged_value(it2->weight, it2->modified, weight_change);
	}
	else
	{
		//Add new edge (nodeA,nodeB)
		adj[shorter].emplace_back(other, weight_change, util::CUR_TIME_AGE);
		adj[other].emplace_back(shorter, weight_change, util::CUR_TIME_AGE);
	}

	modify_aged_value(deg[shorter], weight_change);
	modify_aged_value(deg[other], weight_change);
	modify_aged_value(total_weight, weight_change);
}

void crep::graph::inc_edge(int32_t nodeA, int32_t nodeB)
{
	update_edge(nodeA, nodeB, 1);
}

double crep::graph::get_density() const
{
	return calc_density(num_actual_vertices, get_total_weight());
}

crep::real_ crep::graph::get_total_weight() const
{
	return get_aged_value(total_weight);
}

double crep::graph::calc_correct_density(const int32_t& num_v, const real_& total_w)
{
	if (num_v <= 1)
	{
		return 0.0;
	}
	return (2.0 * total_w) / (num_v * (num_v - 1.0));;
}

double crep::graph::calc_density(const int32_t& num_v, const real_& total_w)
{
	if (num_v <= 0)
	{
		return 0.0;
	}
	return (double)total_w / (double)num_v;
}

double crep::graph::calc_property(const int32_t& num_v, const real_& total_w)
{
	return calc_density(num_v - 1, total_w);
}

int32_t crep::graph::v() const
{
	return num_vertices;
}

const vector<crep::areal_>& crep::graph::get_deg_naged() const
{
	return deg;
}

const crep::real_ crep::graph::get_deg(const id_& vtx_id) const
{
	return get_aged_value(deg[vtx_id]);
}

void crep::graph::add_node()
{
	add_nodes(1, 1);
}

void crep::graph::add_nodes(const int32_t num_additional_nodes, const int32_t num_actual_add_nodes)
{
	auto new_size = num_vertices + num_additional_nodes;
	num_vertices += num_actual_add_nodes;
	num_actual_vertices += num_actual_add_nodes;

	if (deg.capacity() < num_vertices)
	{
		deg.reserve(new_size);
		adj.reserve(new_size);
	}

	deg.resize(num_vertices, { 0, util::CUR_TIME_AGE });
	adj.resize(num_vertices);
}

void crep::graph::change_edges(const int32_t from, const int32_t to)
{
	if (from == to) return;

	auto& adj_from = get_adj_naged(from);

	//Iterate edges from back to front to enable direct modification
	for (auto rit = adj_from.rbegin(); rit != adj_from.rend(); ++rit)
	{
		auto [other, weight, updated] = *rit;

		remove_edge_directed(other, from);
		if (other != to)
		{
			//Transfer inter-component edges
			update_edge(other, to, get_aged_value(weight, updated));
		}
	}

	adj[from].clear();
	modify_aged_value(total_weight, -get_aged_value(deg[from]));
	deg[from] = { 0, util::CUR_TIME_AGE };
}

void crep::graph::remove_adjacient_edges(const int32_t& a)
{
	for (int32_t i = get_adj_naged(a).size() - 1; i >= 0; i--)
	{
		remove_edge(a, get_adj_naged(a)[i].target);
	}
}

crep::real_ crep::graph::get_edge_weight(const int32_t& a, const int32_t& b) const
{
	real_ w = -1;
	for (size_t i = 0; i < get_adj_naged(a).size(); ++i)
	{
		if (get_adj_naged(a)[i].target == b)
		{
			w = get_aged_value(get_adj_naged(a)[i].weight, get_adj_naged(a)[i].modified);
			break;
		}
	}
	return w;
}

void crep::graph::reset_edges()
{
	std::fill(deg.begin(), deg.end(), areal_{ 0, util::CUR_TIME_AGE });
	for (auto& a : adj)
	{
		a.clear();
	}
	total_weight = { 0, util::CUR_TIME_AGE };
	num_actual_vertices = num_vertices;
}

crep::real_ crep::graph::remove_edge(const int32_t nodeA, const int32_t nodeB)
{
	if (nodeA == nodeB) return 0;

	real_ w = 0;
	auto& adj_a = get_adj_naged(nodeA);

	for (size_t i = 0; i < adj_a.size(); i++)
	{
		if (adj_a[i].target == nodeB)
		{
			w = get_aged_value(adj_a[i].weight, adj_a[i].modified);
			adj[nodeA].erase(adj_a.begin() + i);
			break;
		}
	}

	auto& adj_b = get_adj_naged(nodeB);

	for (size_t i = 0; i < adj_b.size(); i++)
	{
		if (adj_b[i].target == nodeA)
		{
			adj[nodeB].erase(adj_b.begin() + i);
			break;
		}
	}

	//update deg, totalWeight
	modify_aged_value(deg[nodeA], -w);
	modify_aged_value(deg[nodeB], -w);
	modify_aged_value(total_weight, -w);

	return w;
}

void crep::graph::remove_edge_directed(const int32_t from, const int32_t to)
{
	if (from == to) return;

	auto& adj_from = get_adj_naged(from);
	auto it = std::find_if(adj_from.begin(), adj_from.end(), [to](const edge& el) { return el.target == to; });
	if (it != adj_from.end())
	{
		modify_aged_value(deg[from], -get_aged_value(it->weight, it->modified));
		adj[from].erase(it);
	}
}

std::vector<int32_t> crep::dfs(const crep::graph& g, int32_t s)
{
	int32_t cur = 0;
	std::vector<bool> visited(g.v(), false);
	std::vector<int32_t> res(g.v(), -1);
	std::stack<int32_t> stack;

	std::vector<int32_t> order(g.v(), 0);
	std::iota(order.begin(), order.end(), 0);
	order.insert(order.begin(), s);

	for (auto j : order)
	{
		if (visited[j])
		{
			continue;
		}

		stack.push(j);

		while (!stack.empty())
		{
			j = stack.top();
			stack.pop();

			if (visited[j])
			{
				continue;
			}

			visited[j] = true;
			res[cur] = j;
			++cur;

			for (auto i = g.get_adj_naged()[j].begin(); i != g.get_adj_naged()[j].end(); ++i)
			{
				if (!visited[i->target])
				{
					stack.push(i->target);
				}
			}
		}
	}

	return res;
}

// Copyright (c) 2019 Martin Schonger