#include "cluster_state.h"

crep::cluster_state::cluster_state(
	const int32_t& num_clusters, const int32_t& cluster_size, const int32_t& num_nodes, const double& augmentation, 
	bool is_logging_enabled_vertex_edge_spells, bool is_logging_enabled_cluster_dynamics)
	: num_clusters(num_clusters), cluster_size(cluster_size), num_nodes(num_nodes), augmentation(augmentation), 
	is_logging_enabled_vertex_edge_spells(is_logging_enabled_vertex_edge_spells), is_logging_enabled_cluster_dynamics(is_logging_enabled_cluster_dynamics)
{
	nodes_to_clusters = std::vector<int32_t>(num_nodes);
	state = std::vector<std::unordered_set<int32_t>>(num_clusters);
	actual_cluster_size = (int32_t) std::floor(augmentation * (double)cluster_size);

	//assume initial assignment to clusters
	for (int32_t i = 0; i < num_clusters; i++)
	{
		state[i].reserve(cluster_size); //todo: maybe use actual_cluster_size instead
		if ((i + 1) * cluster_size <= num_nodes)
		{
			//state[i] = std::unordered_set<int32_t>(cluster_size);
			//auto tmp_iter = state[i].begin();
			//std::advance(tmp_iter, cluster_size);
			//std::iota(state[i].begin(), tmp_iter, i * cluster_size);
			
			for (int32_t j = i * cluster_size; j < (i + 1) * cluster_size; j++)
			{
				state[i].emplace(j);

				if (is_logging_enabled_vertex_edge_spells)
				{
					if (j < 1923) //TODO
					{
						util::cluster_assignment[j] = log_cluster_assignment_record(util::CUR_TIME, i, j, 1, 1); //TODO check if node is really the component root
					}
				}
			}
			pq.emplace(cluster_size, i);
		}
		else if (i * cluster_size < num_nodes)
		{
			//state[i] = std::unordered_set<int32_t>(num_nodes - (i * cluster_size));
			//auto tmp_iter = state[i].begin();
			//std::advance(tmp_iter, num_nodes - (i * cluster_size));
			//std::iota(state[i].begin(), tmp_iter, i * cluster_size);
			
			for (int32_t j = i * cluster_size; j < num_nodes; j++)
			{
				state[i].emplace(j);

				if (is_logging_enabled_vertex_edge_spells)
				{
					if (j < 1923) //TODO
					{
						util::cluster_assignment[j] = log_cluster_assignment_record(util::CUR_TIME, i, j, 1, 1); //TODO check if node is really the component root
					}
				}
			}
			pq.emplace(num_nodes - (i * cluster_size), i);
		}
		else
		{
			pq.emplace(0, i);
		}

		if (is_logging_enabled_cluster_dynamics)
		{
			util::cluster[i].first = util::CUR_TIME;
			util::cluster[i].second = state[i].size();
		}
	}

	int32_t i = 0;
	for (; i < std::floor((double)num_nodes / cluster_size); i++)
	{
		auto tmp_from = nodes_to_clusters.begin();
		std::advance(tmp_from, i * cluster_size);
		auto tmp_to = nodes_to_clusters.begin();
		std::advance(tmp_to, (i + 1) * cluster_size);
		std::fill(tmp_from, tmp_to, i);
	}
	if (num_nodes % cluster_size != 0)
	{
		auto tmp_from = nodes_to_clusters.begin();
		std::advance(tmp_from, i * cluster_size);
		auto tmp_to = nodes_to_clusters.begin();
		std::advance(tmp_to, i * cluster_size + (num_nodes % cluster_size));
		std::fill(tmp_from, tmp_to, i);
	}

}

void crep::cluster_state::add_node(int32_t cluster_id, const int32_t& num_reserve)
{
	assert((int32_t)nodes_to_clusters.size() == num_nodes);
	
	if ((int32_t)nodes_to_clusters.capacity() <= num_nodes) //reserve num_reserve additional space
	{
		nodes_to_clusters.reserve((size_t)num_nodes + num_reserve);
	}

	if (cluster_id == -1) //assume that the node is on the first (i.e. smallest cluster_id) cluster with sufficient space
	{					  //TODO: maybe check if there is some cluster with sufficient space if there was no augmentation
		cluster_id = 0;
		for (auto&& cl : state)
		{
			if ((int32_t)cl.size() < cluster_size)
			{
				break;
			}
			++cluster_id;
		}
	}

	assert(cluster_id < num_clusters);

	nodes_to_clusters.emplace_back(cluster_id);
	state[cluster_id].emplace(num_nodes);

	if (is_logging_enabled_vertex_edge_spells)
	{
		if (util::cluster_assignment[num_nodes].onset != -1)
		{
			std::cerr << "[CHECK ME]" << std::endl;
			std::cerr << util::cluster_assignment.size() << ", " << num_nodes << std::endl;
		}
		util::cluster_assignment[num_nodes] = log_cluster_assignment_record(util::CUR_TIME, cluster_id, num_nodes, 1, 1);
	}

	++num_nodes;

	if (is_logging_enabled_cluster_dynamics)
	{
		if (util::cluster[cluster_id].first != -1)
		{
			log::logger_instance.log("cluster", 
				std::to_string(util::cluster[cluster_id].first), 
				std::to_string(util::CUR_TIME), 
				std::to_string(cluster_id), 
				std::to_string(util::cluster[cluster_id].second));
		}
		util::cluster[cluster_id].first = util::CUR_TIME;
		util::cluster[cluster_id].second = state[cluster_id].size();
	}

	pq.emplace(state[cluster_id].size(), cluster_id);
}

void crep::cluster_state::migrate(union_find& c, const int32_t& root, int32_t target_cluster, std::vector<int32_t>& used_by_comp_set)
{
	std::pair<int32_t, int32_t> target_cluster_tmp{ -1, -1 };

	if (target_cluster != -1)
	{
		target_cluster_tmp.first = get_current_cluster_size(target_cluster);
		target_cluster_tmp.second = target_cluster;
	}
	else
	{
		while (!pq.empty() && (int32_t)state[(target_cluster_tmp = pq.top()).second].size() != target_cluster_tmp.first)
		{
			pq.pop();
		}
		if (pq.empty())
		{
			//should not happen
			std::cerr << std::to_string(util::CUR_TIME) << " [ERROR] Did not find a cluster with sufficient free space." << std::endl;
		}
		else
		{
			pq.pop();
		}

		target_cluster = target_cluster_tmp.second;
	}
	

	used_by_comp_set[target_cluster] -= c.get_size(root); //TODO useful?


	migrate_helper(c, root, target_cluster);


	for (int32_t m = 0; m < num_clusters; m++)
	{
		if (used_by_comp_set[m] != 0)
		{
			pq.emplace(get_current_cluster_size(m), m);

			if (is_logging_enabled_cluster_dynamics)
			{
				if (util::cluster[m].first != -1)
				{
					log::logger_instance.log("cluster",
						std::to_string(util::cluster[m].first),
						std::to_string(util::CUR_TIME),
						std::to_string(m),
						std::to_string(util::cluster[m].second));
				}
				util::cluster[m].first = util::CUR_TIME;
				util::cluster[m].second = get_current_cluster_size(m);
			}
		}
	}
}

int32_t crep::cluster_state::get_actual_cluster_size() const
{
	return actual_cluster_size;
}

int32_t crep::cluster_state::get_current_cluster_size(const int32_t& cluster_id) const
{
	return state[cluster_id].size();
}

int32_t crep::cluster_state::get_corresponding_cluster_size(const int32_t& node) const
{
	return get_current_cluster_size(nodes_to_clusters[node]);
}

int32_t crep::cluster_state::get_cluster(const int32_t& node_id) const
{
	return nodes_to_clusters[node_id];
}

crep::pq_cluster& crep::cluster_state::get_pq()
{
	return pq;
}

std::vector<std::unordered_set<int32_t>>& crep::cluster_state::get_state()
{
	return state;
}

void crep::cluster_state::migrate_helper(union_find& c, const int32_t& component_root, const int32_t& target_cluster)
{
	int32_t cur = component_root;
	do
	{
		if (nodes_to_clusters[cur] != target_cluster)
		{
			//TODO: perform actual migration of node cur

			if (is_logging_enabled_vertex_edge_spells)
			{
				if (util::cluster_assignment[cur].onset != -1)
				{
					log::logger_instance.log("cluster_assign",
						std::to_string(util::cluster_assignment[cur].onset),
						std::to_string(util::CUR_TIME),
						std::to_string(cur),
						std::to_string(util::cluster_assignment[cur].cur_cluster),
						std::to_string(util::cluster_assignment[cur].cur_component_root),
						std::to_string(util::cluster_assignment[cur].cur_component_size),
						std::to_string(util::cluster_assignment[cur].is_component_root));
				}
				util::cluster_assignment[cur] = log_cluster_assignment_record();
			}

			//migrate node cur to target_cluster
			state[nodes_to_clusters[cur]].erase(cur);
			nodes_to_clusters[cur] = target_cluster;
			state[target_cluster].emplace(cur);
		}

		cur = c.next[cur]; //TODO
	} while (cur != component_root);
}

// Copyright (c) 2019 Martin Schonger