/*
Extends graph.h by three vectors for storing additional information about vertices.
*/

#pragma once

#include "graph.h"

namespace crep
{
	class graph_extended : public graph
	{
	public:
		graph_extended(int32_t num_nodes = 0, bool enable_logging = false) : graph(num_nodes, enable_logging)
		{
			migrated = std::vector<int32_t>(num_nodes);
			visited = std::vector<int32_t>(num_nodes);
			included = std::vector<int32_t>(num_nodes);
		}

		virtual ~graph_extended() = default;

		void add_nodes(const int32_t num_additional_nodes, const int32_t num_actual_add_nodes = 1) override
		{
			graph::add_nodes(num_additional_nodes, num_actual_add_nodes);

			if (migrated.capacity() < migrated.size() + num_actual_add_nodes)
			{
				migrated.reserve(migrated.size() + num_additional_nodes);
				visited.reserve(visited.size() + num_additional_nodes);
				included.reserve(included.size() + num_additional_nodes);
			}

			migrated.resize(migrated.size() + num_actual_add_nodes);
			visited.resize(visited.size() + num_actual_add_nodes);
			included.resize(included.size() + num_actual_add_nodes);
		}

		//Stores the time point when a vertex was last visited.
		//This is extensively used by the strategies defined in explore_neighborhood.h.
		std::vector<int32_t> visited;

		//These were used while experimenting with differnt computations of the target partition (eviction, etc.), 
		//but are currently not needed.
		std::vector<int32_t> migrated;
		std::vector<int32_t> included;
	};
}

// Copyright (c) 2019 Martin Schonger