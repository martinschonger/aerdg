#pragma once

#include <set>

#include "comp_graph.h"
#include "data_source.h"
#include "charikar.h"

//#include "optimization.h"

namespace crep
{
	struct log_options
	{
		bool time = false;
		bool vertex_edge_spells = false; //cluster_assignment and network_dynamics
		bool cluster_dynamics = false;
		bool events = false;
		//bool component_dynamics = false;
	};

	void init_logging(const log_options& log_opts, const int32_t& num_clusters)
	{
		if (log_opts.time)
		{
			crep::log::logger_instance.add_identifier("time", crep::get_log_dir() + log::logger_instance.tstamp + "__time_measurement" + ".csv", "graph_size,time");
		}

		if (log_opts.vertex_edge_spells)
		{
			size_t log_num_max_vertices = 1923;

			log::logger_instance.add_identifier("net", 
				crep::get_log_dir() + log::logger_instance.tstamp + "__edge_spells" + ".csv", 
				"onset,terminus,tail vertex.id,head vertex.id,weight");
			util::edge_ontime = std::vector<std::vector<std::pair<int32_t, int32_t>>>(
				log_num_max_vertices, std::vector<std::pair<int32_t, int32_t>>(log_num_max_vertices, { -1, -1 }));


			log::logger_instance.add_identifier("cluster_assign", 
				crep::get_log_dir() + log::logger_instance.tstamp + "__vertex_spells" + ".csv", 
				"onset,terminus,vertex.id,cluster,component_root,component_size,is_component_root");
			util::cluster_assignment = std::vector<log_cluster_assignment_record>(log_num_max_vertices);
		}

		if (log_opts.cluster_dynamics)
		{
			log::logger_instance.add_identifier("cluster", 
				crep::get_log_dir() + log::logger_instance.tstamp + "__cluster_dynamics" + ".csv", 
				"onset,terminus,vertex.id,size");
			util::cluster = std::vector<std::pair<int32_t, int32_t>>(num_clusters, { -1, -1 });
		}

		if (log_opts.events)
		{
			crep::log::logger_instance.add_identifier("event", 
				crep::get_log_dir() + log::logger_instance.tstamp + "__events" + ".csv", 
				"time,type,num_nodes_base,num_nodes_meta,nodes,root,trigger_src,trigger_dst");
		}
	}

	void run_dynamic_partition(
		std::unique_ptr<data_source>& ds,
		const double& alpha,
		const int32_t& num_clusters,
		const int32_t& cluster_size,
		const double& augmentation,
		const int32_t& num_batches,
		const int32_t& batch_size,
		const log_options& log_opts,
		int64_t& total_cost,
		double& total_time,
		int32_t& num_merges,
		int32_t& num_deletes,
		bool verbose = false)
	{
		int64_t com_cost = 0;
		double migration_cost = 0.0;
		num_merges = 0;
		num_deletes = 0;

		util::CUR_TIME = 0;

		init_logging(log_opts, num_clusters);

		comp_graph g(log_opts.vertex_edge_spells);

		//for (size_t i = 0; i < 5000; i++)
		//{
		//	g.add_vertex();
		//}
		int32_t counter = 0;

		cluster_state clusters(num_clusters, cluster_size, g.base_size(), augmentation, log_opts.vertex_edge_spells, log_opts.cluster_dynamics);

		auto begin_core = hrc::now();
		auto begin_batch = hrc::now();

		while (ds->has_next())
		{
			++util::CUR_TIME;

			if (log_opts.time && (counter % ds->get_batch_size() == 0))
			{
				begin_batch = hrc::now();
			}

			counter++;
			auto [src, dst] = ds->next();
			if (src >= g.base_size())
			{
				g.add_vertex();
				clusters.add_node();
			}
			if (dst >= g.base_size())
			{
				g.add_vertex();
				clusters.add_node();
			}

			//update only if weights change
			if (g.change_weight(src, dst, 1))
			{
				bool found_mergeable = false;
				std::vector<int32_t> res;

				/*bool precond = check_precondition_mergeable(g.get_meta_graph(), g.get_comps().f(src), alpha);
				if (precond)
				{
				}

				if (!found_mergeable && precond)
				{
					//TODO remove at least the subgraph found by check_precond
					std::cerr << std::to_string(util::CUR_TIME) << " [WARNING] Charikar missed a mergeable subgraph. (src = " << src << ", dst = " << dst << ")" << std::endl;
				}*/
				
				
				res = charikar_old(found_mergeable, g.get_meta_graph(), g.get_comps().parent, alpha, false);

				///real_ th = alpha;
				///res = run_charikar_on_direct_nb(g.get_meta_graph(), g.get_comps().f(src), g.get_comps().f(dst), th, found_mergeable);

				int32_t max_component_size_before_delete = cluster_size;

				//test: merge/migrate nodes immediately after a request

				/*found_mergeable = true;
				res = std::vector<int32_t>(g.get_base_graph().v(), -1);
				res[g.get_comps().f(src)] = 0;
				res[g.get_comps().f(dst)] = 0;
				max_component_size_before_delete = clusters.get_actual_cluster_size();*/

				//end test

				if (found_mergeable)
				{
					//merge or delete returned subgraph
					int32_t num_nodes_comp_set = 0;
					int32_t i = 0;
					for (; i < (int32_t)res.size(); i++)
					{
						if (res[i] != -1)
						{
							break;
						}
					}
					num_nodes_comp_set += g.get_comps().get_size(i);
					for (int32_t j = i + 1; j < (int32_t)res.size(); j++)
					{
						if (res[j] != -1)
						{
							num_nodes_comp_set += g.get_comps().get_size(j);
						}
					}


					int32_t new_root = -1;
					i = 0;
					for (; i < (int32_t)res.size(); i++)
					{
						if (res[i] != -1)
						{
							break;
						}
					}

					std::vector<int32_t> used_by_comp_set(num_clusters, 0);
					if (num_nodes_comp_set <= max_component_size_before_delete)
					{
						used_by_comp_set[clusters.nodes_to_clusters[i]] += g.get_comps().get_size(i);
					}

					std::set<int32_t> log_nodes;
					for (int32_t j = i + 1; j < (int32_t)res.size(); j++)
					{
						if (res[j] != -1)
						{
							if (num_nodes_comp_set <= max_component_size_before_delete)
							{
								used_by_comp_set[clusters.nodes_to_clusters[j]] += g.get_comps().get_size(j);
							}
							new_root = g.merge(i, j);
							if (log_opts.events)
							{
								log_nodes.emplace(i);
								log_nodes.emplace(j);
							}
						}
					}

					if (num_nodes_comp_set <= max_component_size_before_delete)
					{
						//find target_cluster
						int32_t min_migration_effort = num_nodes_comp_set;
						int32_t target_cluster = -1;

						for (int32_t m = 0; m < num_clusters; m++)
						{
							if (clusters.get_current_cluster_size(m) - used_by_comp_set[m] + num_nodes_comp_set <= clusters.get_actual_cluster_size())
							{
								if ((num_nodes_comp_set - used_by_comp_set[m]) < min_migration_effort)
								{
									target_cluster = m;
									min_migration_effort = num_nodes_comp_set - used_by_comp_set[m];
								}
							}
						}

						
						//migrate
						clusters.migrate(g.get_comps(), new_root, target_cluster, used_by_comp_set);

						migration_cost += alpha * min_migration_effort;


						++num_merges;
						
						if (log_opts.vertex_edge_spells)
						{
							if (util::cluster_assignment[new_root].onset != -1) //migrate has reset the cluster_assignment entry
							{
								log::logger_instance.log("cluster_assign",
									std::to_string(util::cluster_assignment[new_root].onset),
									std::to_string(util::CUR_TIME),
									std::to_string(new_root),
									std::to_string(util::cluster_assignment[new_root].cur_cluster),
									std::to_string(util::cluster_assignment[new_root].cur_component_root),
									std::to_string(util::cluster_assignment[new_root].cur_component_size),
									std::to_string(util::cluster_assignment[new_root].is_component_root));
							}
							util::cluster_assignment[new_root] = log_cluster_assignment_record(
								util::CUR_TIME, clusters.nodes_to_clusters[new_root], g.get_comps().f(new_root), g.get_comps().get_size(new_root), 1);
						}

						if (verbose) std::cerr << util::CUR_TIME << " merged:  " << num_nodes_comp_set << ", (" << new_root << "), (src=" << src << ", dst=" << dst << ")" << std::endl;
						if (log_opts.events)
						{
							std::string tmp = "\"[";
							for (auto el : log_nodes)
							{
								tmp += std::to_string(el) + ",";
							}
							tmp.pop_back();
							tmp += "]\"";
							//time,type,num_nodes_base,num_nodes_meta,nodes,root,trigger_src,trigger_dst
							log::logger_instance.log("event", 
								std::to_string(util::CUR_TIME), 
								"merge", 
								std::to_string(num_nodes_comp_set), 
								std::to_string(log_nodes.size()), 
								tmp, 
								std::to_string(new_root), 
								std::to_string(src), 
								std::to_string(dst));
						}

						/*if (clusters.nodes_to_clusters[src] != clusters.nodes_to_clusters[dst])
						{
							std::cerr << std::to_string(util::CUR_TIME) << " [WARNING] Found mergeable subgraph but still incurred communication cost. (src = " << src << ", dst = " << dst << ")" << std::endl;
						}*/
					}
					else
					{
						std::string log_nodes_list = log_opts.events ? "\"[" : "";
						g.delete_comp(new_root, /*clusters, */log_nodes_list, log_opts.events);
						++num_deletes;

						if (verbose) std::cerr << util::CUR_TIME << " deleted: " << num_nodes_comp_set << ", (" << new_root << "), (src=" << src << ", dst=" << dst << ")" << std::endl;
						if (log_opts.events)
						{
							log_nodes_list.pop_back();
							log_nodes_list += "]\"";
							log::logger_instance.log("event", 
								std::to_string(util::CUR_TIME), 
								"delete", 
								std::to_string(num_nodes_comp_set), 
								std::to_string(num_nodes_comp_set), 
								log_nodes_list, 
								std::to_string(new_root), 
								std::to_string(src), 
								std::to_string(dst));
						}
					}
				}
				//assert(g.get_base_graph().get_total_weight() == g.get_meta_graph().get_total_weight());
				if (clusters.nodes_to_clusters[src] != clusters.nodes_to_clusters[dst])
				{
					++com_cost;
				}

				if (verbose)
				{
					print_vector(clusters.nodes_to_clusters, "nodes_to_clusters");
				}
			}

			if (log_opts.time && (counter % ds->get_batch_size() == 0))
			{
				auto end_batch = hrc::now();
				auto time_spent_batch = std::chrono::duration_cast<dur>(end_batch - begin_batch);

				if (verbose)
				{
					std::cerr << "num_requests: " << counter << std::endl;
					std::cerr << " graph size: " << g.base_size() << " (" << g.meta_size() << ")" << std::endl;
					std::cerr << " time_spent_batch:  " << time_spent_batch.count() << std::endl;
				}

				log::logger_instance.log("time", std::to_string(g.base_size()), std::to_string(time_spent_batch.count()));
			}

		}

		if (verbose) std::cerr << "graph size: " << g.base_size() << " (" << g.meta_size() << ")" << std::endl;

		if (log_opts.time)
		{
			auto time_spent_core = std::chrono::duration_cast<dur>(hrc::now() - begin_core);
			total_time = time_spent_core.count();

			if (verbose)
			{
				std::cerr << "[dynamic] total time: " << time_spent_core.count() << std::endl;

				std::cerr << "avg_time_per_batch: " << (time_spent_core.count() / ds->get_batch_size()) << std::endl;
				std::cerr << "avg_time_per_request: " << (time_spent_core.count() / counter) << std::endl;


				std::cerr << "charikar (1): " << util::charikar_partial_time_detailed.count() << std::endl;
				std::cerr << "charikar (2): " << util::charikar_partial_time_detailed2.count() << std::endl;
				std::cerr << "charikar (3): " << util::charikar_partial_time_detailed3.count() << std::endl;
			}
		}

		if (log_opts.vertex_edge_spells)
		{
			for (int32_t i = 0; i < (int32_t)util::edge_ontime.size(); i++)
			{
				for (int32_t j = 0; j < (int32_t)util::edge_ontime[i].size(); j++)
				{
					if (util::edge_ontime[i][j].first != -1)
					{
						log::logger_instance.log("net", 
							std::to_string(util::edge_ontime[i][j].first), 
							std::to_string(util::CUR_TIME), 
							std::to_string(i), 
							std::to_string(j), 
							std::to_string(util::edge_ontime[i][j].second));
					}
				}
			}

			for (int32_t i = 0; i < (int32_t)util::cluster_assignment.size(); i++)
			{
				if (util::cluster_assignment[i].onset != -1)
				{
					log::logger_instance.log("cluster_assign",
						std::to_string(util::cluster_assignment[i].onset),
						std::to_string(util::CUR_TIME),
						std::to_string(i),
						std::to_string(util::cluster_assignment[i].cur_cluster),
						std::to_string(util::cluster_assignment[i].cur_component_root),
						std::to_string(util::cluster_assignment[i].cur_component_size),
						std::to_string(util::cluster_assignment[i].is_component_root));
				}
			}
		}

		if (log_opts.cluster_dynamics)
		{
			for (int32_t i = 0; i < (int32_t)util::cluster.size(); i++)
			{
				if (util::cluster[i].first != -1)
				{
					log::logger_instance.log("cluster", 
						std::to_string(util::cluster[i].first), 
						std::to_string(util::CUR_TIME), 
						std::to_string(i), 
						std::to_string(util::cluster[i].second));
				}
			}
		}

		total_cost = com_cost + ceil(migration_cost);
	}

}

// Copyright (c) 2019 Martin Schonger