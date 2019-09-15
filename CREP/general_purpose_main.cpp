#include "general_purpose_main.h"

namespace crep
{
	int32_t calc_cost(const std::vector<int32_t>& par_sol, comp_graph& g)
	{
		int32_t cur_cost = 0;
		for (int32_t n1 = 0; n1 < g.get_base_graph().get_adj_naged().size(); n1++)
		{
			for (int32_t n2 = 0; n2 < g.get_base_graph().get_adj_naged()[n1].size(); n2++)
			{
				if (par_sol[n1] != par_sol[g.get_base_graph().get_adj_naged()[n1][n2].target])
				{
					cur_cost += g.get_base_graph().get_adj_naged()[n1][n2].weight;
				}
			}
		}
		cur_cost /= 2;
		return cur_cost;
	}

	bool backtrack(
		std::vector<int32_t>& par_sol, 
		std::vector<int32_t>& cluster_utilization, 
		int32_t cur_node, 
		const std::vector<int32_t> order, 
		const std::vector<int32_t> order_reverse_map, 
		int32_t max_cluster_id_used, 
		int32_t& cur_best_cost, 
		int32_t& acc_cost, 
		const int32_t& num_clusters, 
		const int32_t& cluster_size, 
		comp_graph& g)
	{
		//std::cerr << std::string(cur_node, ' ') << "." << std::endl;

		if (cur_node >= g.base_size())
		{
			//calc communication cost
			int32_t cur_cost = calc_cost(par_sol, g); //TODO: maybe this is no longer required because of the accumulated cost value

			if (cur_cost < cur_best_cost)
			{
				std::cerr << cur_best_cost << " -> " << cur_cost << std::endl;
				cur_best_cost = cur_cost;
				return true;
			}
			else
			{
				return false;
			}
		}

		int32_t max_considered_cluster = max_cluster_id_used;
		if (max_considered_cluster + 1 < num_clusters)
		{
			++max_considered_cluster;
		}

		bool improved = false;
		for (int32_t m = 0; m <= max_considered_cluster; m++)
		{
			if (cluster_utilization[m] < cluster_size)
			{
				//feasibility check
				int32_t cur_cost2 = 0;
				for (int32_t n2 = 0; n2 < g.get_base_graph().get_adj_naged()[order[cur_node]].size(); n2++)
				{
					if (order_reverse_map[g.get_base_graph().get_adj_naged()[order[cur_node]][n2].target] < cur_node && par_sol[g.get_base_graph().get_adj_naged()[order[cur_node]][n2].target] != m)
					{
						cur_cost2 += g.get_base_graph().get_adj_naged()[order[cur_node]][n2].weight;
					}
				}

				if (acc_cost + cur_cost2 >= cur_best_cost)
				{
					continue; //pruning
				}

				acc_cost += cur_cost2;


				int32_t tmp_node_placement = par_sol[order[cur_node]];

				par_sol[order[cur_node]] = m;
				++cluster_utilization[m];
				if (backtrack(
					par_sol, 
					cluster_utilization, 
					cur_node + 1, 
					order, 
					order_reverse_map, 
					std::max(max_cluster_id_used, m), 
					cur_best_cost, 
					acc_cost, 
					num_clusters, 
					cluster_size, 
					g))
				{
					improved = true;
				}
				else
				{
					par_sol[order[cur_node]] = tmp_node_placement;
				}
				--cluster_utilization[m];

				acc_cost -= cur_cost2;
			}
		}

		if (improved)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
}


int main(int argc, char* argv[])
{
	/*crep::int_ num_batches2 = 1000;
	crep::int_ batch_size2 = 100000;
	auto ds2 = std::make_unique<crep::data_mongodb>(num_batches2, batch_size2);
	for (crep::int_ i = 0; i < num_batches2; i++)
	{
		std::vector<std::pair<crep::id_, crep::id_>> vec(batch_size2);
		for (crep::int_ j = 0; j < batch_size2; j++)
		{
			vec[j] = ds2->next();
		}

		std::string config_filename = crep::get_project_root() + "mdb_offline/";
		config_filename += std::to_string(i) + ".txt";
		std::cerr << "[INFO] writing '" << config_filename << "'." << std::endl;
		crep::write_vector(config_filename, vec);
	}

	std::cerr << "[INFO] done" << std::endl;
	exit(0);*/


	//get data
	int32_t num_batches = 100;
	int32_t batch_size = 1;

	auto ds = std::make_unique<crep::data_mongodb>("facebook", "clusterA", "timestamp", "srcrack", "dstrack", num_batches, batch_size);


	//build graph
	crep::comp_graph g;

	while (ds->has_next())
	{
		auto [src, dst] = ds->next();
		if (src >= g.base_size())
		{
			g.add_vertex();
		}
		if (dst >= g.base_size())
		{
			g.add_vertex();
		}

		g.change_weight(src, dst, 1);
	}

	std::cerr << "size = " << g.base_size() << std::endl;


	//set up brute-force
	int32_t cluster_size = 8; //16; //32
	int32_t num_clusters = 15; //35; //61

	int64_t objval = -1;
	double total_time = -1;
#if defined(__GNUC__)
	auto metis_sol = static_partition(g, num_clusters, cluster_size, objval, total_time);
	crep::print_vector(metis_sol, "s_sol");
#endif
	std::cerr << "sbc = " << objval << std::endl;


	std::vector<int32_t> partial_solution(g.base_size(), -1);
	partial_solution[0] = 0; //partial_solution[0] corresponds to the cluster on which node 0 is placed
	std::vector<int32_t> cluster_utilization(num_clusters, 0);
	cluster_utilization[0] = 1;
	int32_t cur_node = 1;
	int32_t max_cluster_id_used = 0;
	int32_t cur_best_cost = objval + 1; //the optimal solution is at least as good as the approximation by METIS
	std::cerr << "cbc = " << cur_best_cost << std::endl;
	int32_t acc_cost = 0;

	auto order = crep::dfs(g.get_base_graph(), 0); //consider nodes in order determined by DFS
	std::vector<int32_t> order_reverse_map(order.size(), -1);
	for (int32_t idx = 0; idx < order.size(); idx++)
	{
		order_reverse_map[order[idx]] = idx;
	}

	if (backtrack(
		partial_solution, 
		cluster_utilization, 
		cur_node, 
		order, 
		order_reverse_map, 
		max_cluster_id_used, 
		cur_best_cost, 
		acc_cost, 
		num_clusters, 
		cluster_size, 
		g))
	{
		std::cerr << "cbc = " << cur_best_cost << std::endl;
		crep::print_vector(partial_solution, "p_sol");
		crep::print_vector(cluster_utilization, "c_util");
	}

	return 0;
}

// Copyright (c) 2019 Martin Schonger