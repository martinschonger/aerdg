#pragma once

#include <vector>
#include <unordered_set>
#include <queue>
#include <numeric>
#include <cmath>
#include <algorithm>
#include <cassert>
#include <iterator>

#include "union_find.h"
#include "utility.h"
#include "logger.h"

namespace crep
{
	typedef std::priority_queue<std::pair<int32_t, int32_t>, std::vector<std::pair<int32_t, int32_t>>, util::comp_custom> pq_cluster;

	class cluster_state
	{
	public:
		cluster_state() = default;
		cluster_state(
			const int32_t& num_clusters, 
			const int32_t& cluster_size, 
			const int32_t& num_nodes, 
			const double& augmentation = 1.0, 
			bool is_logging_enabled_vertex_edge_spells = false, 
			bool is_logging_enabled_cluster_dynamics = false);
		~cluster_state() = default;

		cluster_state& operator=(const cluster_state& other)
		{
			num_clusters = other.num_clusters;
			cluster_size = other.cluster_size;
			num_nodes = other.num_nodes;
			augmentation = other.augmentation;
			actual_cluster_size = other.actual_cluster_size;
			state = other.state; //should be copy assignment
			pq = other.pq; //same as above
			nodes_to_clusters = other.nodes_to_clusters;
			return *this;
		}

		void add_node(int32_t cluster_id = -1, const int32_t& num_reserve = 100);
		void migrate(union_find& c, const int32_t& root, int32_t target_cluster, std::vector<int32_t>& used_by_comp_set);

		std::vector<int32_t> nodes_to_clusters;

		bool is_logging_enabled_vertex_edge_spells;
		bool is_logging_enabled_cluster_dynamics;

		int32_t get_actual_cluster_size() const;

		int32_t get_current_cluster_size(const int32_t& cluster_id) const;
		int32_t get_corresponding_cluster_size(const int32_t& node) const;

		int32_t get_cluster(const int32_t& node_id) const;

		pq_cluster& get_pq();

		std::vector<std::unordered_set<int32_t>>& get_state();

	private:
		int32_t num_clusters;
		int32_t cluster_size;
		int32_t num_nodes;
		double augmentation;
		int32_t actual_cluster_size;
		std::vector<std::unordered_set<int32_t>> state;

		//first: cluster size, second: cluster number
		pq_cluster pq{ util::compare_custom };

		void migrate_helper(union_find& c, const int32_t& component_root, const int32_t& target_cluster);
	};
}

// Copyright (c) 2019 Martin Schonger