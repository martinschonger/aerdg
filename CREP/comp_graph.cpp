/*
Implementation of comp_graph.h.
*/

#include "comp_graph.h"

crep::comp_graph::comp_graph(int_ num_vertices, bool enable_logging) : enable_logging(enable_logging)
{
	base_graph = graph_extended(num_vertices);
	meta_graph = graph_extended(num_vertices, enable_logging);
	comps = union_find(num_vertices);

	comp_weights.resize(num_vertices, { 0, util::CUR_TIME_AGE });
}

int32_t crep::comp_graph::base_size() const
{
	return base_graph.v();
}

int32_t crep::comp_graph::meta_size() const
{
	return meta_graph.num_actual_vertices;
}

void crep::comp_graph::add_vertex(const int32_t& num_reserve)
{
	base_graph.add_nodes(num_reserve);
	meta_graph.add_nodes(num_reserve);
	comps.add_elems(num_reserve);

	comp_weights.resize(comp_weights.size() + num_reserve, { 0, util::CUR_TIME_AGE });
}

bool crep::comp_graph::change_weight(const int32_t& a, const int32_t& b, const real_& delta_w)
{
	//Get components of both vertices
	int32_t root_a = comps.f(a);
	int32_t root_b = comps.f(b);

	if (root_a != root_b)
	{
		//Inter-component update (-> modify edge weight)
		base_graph.update_edge(a, b, delta_w);
		meta_graph.update_edge(root_a, root_b, delta_w);
		return true;
	}
	else
	{
		//Intra-component update (-> modify component weight)
		modify_aged_value(comp_weights[root_a], delta_w);
		return false;
	}
}

int32_t crep::comp_graph::merge(const int32_t& a, const int32_t& b)
{
	//Get components of both vertices
	int32_t comp_a = comps.f(a);
	int32_t comp_b = comps.f(b);
	assert(comp_a != comp_b);

	//Merge components
	comps.u(a, b);
	int32_t root = comps.f(a);

	//Update meta_graph
	//Remove the potential edge between the two original components
	auto w_meta = meta_graph.remove_edge(comp_a, comp_b);
	int32_t from = comp_a, to = root;
	if (from == to)
	{
		from = comp_b;
	}

	//Change all edges incident to the deleted component 'from' to the persisting component 'to'.
	// => this is why 'two' graphs are required
	meta_graph.change_edges(from, to);

	meta_graph.num_actual_vertices--;

	//Update component weights
	modify_aged_value(comp_weights[to], w_meta + get_comp_weight(from));
	comp_weights[from] = { 0, util::CUR_TIME_AGE };

	//Maintain correct base_graph:
	// delete all edges with one endpoint in comp_a and the other endpoint in comp_b
	int32_t cur_a = a, cur_b;
	do //Note: It might be possible to optimize these loops into one loop. However, this would not be trivial.
	{
		cur_b = b;
		do
		{
			w_meta -= base_graph.remove_edge(cur_a, cur_b);
			cur_b = comps.next[cur_b];
		} while (cur_b != b);
		cur_a = comps.next[cur_a];
	} while (cur_a != a);

	assert(std::fabs(w_meta) <= 1e-8);

	return root;
}

void crep::comp_graph::delete_comp(const int32_t& comp, std::string& log_nodes, const bool log_me)
{
	assert(comp == comps.f(comp));

	//Skip components of size 1
	if (comps.next[comp] == comp)
	{
		return;
	}

	//Update meta_graph: delete all edges from comp
	meta_graph.remove_adjacient_edges(comp);

	//Update component weight
	comp_weights[comp] = { 0, util::CUR_TIME_AGE };

	//Create singleton components using edges from base_graph.
	// => this is why 'two' graphs are required
	int32_t cur = comp;
	do
	{
		for (auto [other, w, updated] : base_graph.get_adj_naged(cur))
		{
			assert(comps.f(other) != comp);
			assert(comps.f(other) != cur);

			meta_graph.update_edge(cur, comps.f(other), get_aged_value(w, updated));
		}

		meta_graph.num_actual_vertices++;

		cur = comps.next[cur];
	} while (cur != comp);
	meta_graph.num_actual_vertices--;

	//Reset union-find structure for all affected vertices
	cur = comp;
	int32_t tmp;
	do
	{
		tmp = cur;

		cur = comps.next[cur];
		comps.reset(tmp);
	} while (cur != comp);

	//Nothing changes for base_graph
}

crep::graph_extended& crep::comp_graph::get_base_graph()
{
	return base_graph;
}

crep::graph_extended& crep::comp_graph::get_meta_graph()
{
	return meta_graph;
}

crep::union_find& crep::comp_graph::get_comps()
{
	return comps;
}

void crep::comp_graph::reset_edges()
{
	base_graph.reset_edges();
	meta_graph.reset_edges();
	comps.reset();
}

// Copyright (c) 2019 Martin Schonger