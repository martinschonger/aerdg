/*
Central data structure for maintaining a view of the communication network: 
 two graphs together with a union-find structure.
*/

#pragma once

#include <cassert>
#include <iostream>

#include "graph.h"
#include "graph_extended.h"
#include "union_find.h"

namespace crep
{
	class comp_graph
	{
	public:
		comp_graph(int_ num_vertices = 0, bool enable_logging = false);
		~comp_graph() = default;

		//Get the size of the base_graph (= number of (physical) communication endpoints)
		int32_t base_size() const;

		//Get the size of the meta_graph (= number of communication components)
		int32_t meta_size() const;

		//Add a single vertex to the graph.
		//If the internal vectors have no capacity left, reserve 'num_reserve' additional space for each of them.
		void add_vertex(const int32_t& num_reserve = 100);

		//Modify the edge weight w(a,b) (and w(b,a)) by 'delta_w'
		bool change_weight(const int32_t& a, const int32_t& b, const real_& delta_w = 1);
		
		//Merge the two components containing vertices 'a' and 'b'
		int32_t merge(const int32_t& a, const int32_t& b);

		//Delete/split the component with root 'comp'
		void delete_comp(const int32_t& comp, std::string& log_nodes, const bool log_me = false);

		graph_extended& get_base_graph();
		graph_extended& get_meta_graph();
		union_find& get_comps();

		//Reset the data structure
		void reset_edges();

		//Get the aged weight of the component with root 'comp_id'
		real_ get_comp_weight(const id_& comp_id)
		{
			return get_aged_value(comp_weights[comp_id]);
		}

		bool enable_logging;

	private:
		//Graph structure where vertices correspond to actual (physical) communication endpoints
		graph_extended base_graph;

		//Graph structure where vertices correspond to logical communication components
		graph_extended meta_graph;

		//Relate physical communication endpoints and logical communication components
		union_find comps;

		//Store the actual weight of each communication component
		std::vector<areal_> comp_weights;
	};
}

// Copyright (c) 2019 Martin Schonger