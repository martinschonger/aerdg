/*
Represents an undirected weighted graph with adjacency lists, supports aging edge weights.
*/

#pragma once

#include <vector>
#include <algorithm>
#include <stack>
#include <numeric>

#include "typedefs.h"
#include "utility.h"
#include "logger.h"

namespace crep
{
	//Directed edge
	class edge
	{
	public:
		edge() = default;
		edge(id_ t, real_ w, tstamp_ u) : target(t), weight(w), modified(u) {}
		~edge() = default;

		//Target vertex
		id_ target;

		//Actual value at time 'modified'
		real_ weight;

		//Time of last weight modification
		tstamp_ modified;
	};

	//Undirected weighted graph
	class graph
	{
	public:
		graph(int32_t num_nodes = 0, bool enable_logging = false);
		virtual ~graph() = default;

		//Get the entire adjacency structure
		const std::vector<std::vector<edge>>& get_adj_naged() const;

		//Get all incident edges of 'node'
		const std::vector<edge>& get_adj_naged(const int32_t& node) const;

		//Get aged density
		double get_density() const;

		//Get the total aged weight of the graph
		real_ get_total_weight() const;

		//Get the number of vertices
		int32_t v() const;

		//Get all weighted degrees
		const std::vector<areal_>& get_deg_naged() const;

		//Get the aged degree of 'vtx_id'
		const real_ get_deg(const id_& vtx_id) const;

		//Get the aged edge weight w(a,b)
		real_ get_edge_weight(const int32_t& a, const int32_t& b) const;


		//Set the egde weight w(a,b) to 'weight'
		void set_edge(int32_t a, int32_t b, real_ weight);

		//Change w(nodeA,nodeB) by 'weight_change'
		void update_edge(int32_t nodeA, int32_t nodeB, real_ weight_change);

		//Increment w(nodeA,nodeB) by +1
		void inc_edge(int32_t nodeA, int32_t nodeB);

		//Add a single vertex to the graph
		virtual void add_node();

		//Add 'num_actual_add_nodes' vertices to the graph.
		//If 'adj' and 'deg' do not have enough capacity left, reserve 'num_additional_nodes' additional space for each vector.
		virtual void add_nodes(const int32_t num_additional_nodes, const int32_t num_actual_add_nodes = 1);

		//Remove edge (nodeA,nodeB) (and (nodeB,nodeA)) if it exists
		real_ remove_edge(const int32_t nodeA, const int32_t nodeB);

		//Remove only the directed edge from 'from' to 'to'.
		//This is a convenience function for improved efficiency of 'change_edges'.
		void remove_edge_directed(const int32_t from, const int32_t to);

		//Change all incident edges of 'from' to 'to'
		void change_edges(const int32_t from, const int32_t to);

		//Remove all edges going from or to 'a'
		void remove_adjacient_edges(const int32_t& a);

		//Remove all edges and update all degrees
		void reset_edges();

		//Modify the total weight of the graph, which is subject to aging
		void modify_total_weight(const real_& delta)
		{
			modify_aged_value(total_weight, delta);
		}

		//Actual number of vertices, used for meta_graph where it corresponds to the number of components
		int32_t num_actual_vertices = 0;

		bool enable_logging;

		//Compute the edge density
		static double calc_correct_density(const int32_t& num_v, const real_& total_w);

		//Compute the unscaled average degree, which is equal to 0.5 * avg_degree
		static double calc_density(const int32_t& num_v, const real_& total_w);

		//Compute the CREP-density
		static double calc_property(const int32_t& num_v, const real_& total_w);

		//Depth first search, legacy code
		friend std::vector<int32_t> dfs(const crep::graph& g, int32_t s);

	private:
		int32_t num_vertices = 0;
		areal_ total_weight;

		//Adjacency structure
		std::vector<std::vector<edge>> adj;

		//Weighted vertex degrees
		std::vector<areal_> deg;
	};

	//Depth first search, legacy code
	extern std::vector<int32_t> dfs(const crep::graph& g, int32_t s);
}

// Copyright (c) 2019 Martin Schonger