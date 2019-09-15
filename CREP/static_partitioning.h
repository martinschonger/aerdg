/*
Facilitate staic (and experimentally also adaptive) partitioning.
*/

#pragma once

#include <vector>
#include <map>
#include <algorithm>

#include "comp_graph.h"
#include "data_source.h"
#include "utility.h"
#include "debug.h"
#include "assign_initial.h"

#include "../lib/parmetis-4.0.3/metis/include/metis.h"
#include "../lib/parmetis-4.0.3/metis/libmetis/metislib.h"

#include "../lib/parmetis-4.0.3/include/parmetis.h"

namespace crep
{
	//Represents a graph object as taken by METIS/ParMETIS.
	//See their manual for details.
	struct gk_graph_t_64
	{
		int64_t nvtxs;                /*!< The number of vertices in the graph */
		ssize_t* xadj;                /*!< The ptr-structure of the adjncy list */
		int64_t* adjncy;              /*!< The adjacency list of the graph */
		int64_t* iadjwgt;             /*!< The integer edge weights */
		double* fadjwgt;              /*!< The floating point edge weights */
		int64_t* ivwgts;              /*!< The integer vertex weights */
		double* fvwgts;               /*!< The floating point vertex weights */
		int64_t* ivsizes;             /*!< The integer vertex sizes */
		double* fvsizes;              /*!< The floating point vertex sizes */
		int64_t* vlabels;             /*!< The labels of the vertices */
	};

	//Convert a graph (in our format) to a representation readable by METIS/ParMETIS.
	//See their manual for details.
	gk_graph_t_64 crep_graph_to_metis_graph(comp_graph& g)
	{
		int64_t n = g.meta_size();

		std::map<int32_t, int64_t> g_to_gk;
		std::map<int64_t, int32_t> gk_to_g;
		int64_t cur = 0;
		for (int32_t i = 0; i < g.base_size(); i++)
		{
			if (g.get_comps().parent[i] == i)
			{
				//Only consider a vertex that corresponds to a logical communiation component?
				//Mostly, we will encounter singleton components at this point.
				g_to_gk.emplace(i, cur);
				gk_to_g.emplace(cur++, i);
			}
		}


		gk_graph_t_64 gk;

		gk.nvtxs = n;
		gk.xadj = new int64_t[n + 1];
		cur = 0;
		for (int32_t i = 0; i < n; i++)
		{
			gk.xadj[i] = cur;
			int32_t cur_node = gk_to_g.find(i)->second;
			auto& cur_adj = g.get_meta_graph().get_adj_naged(cur_node);
			cur += cur_adj.size();
		}
		gk.xadj[n] = cur;


		gk.adjncy = new int64_t[gk.xadj[n]];
		gk.iadjwgt = new int64_t[gk.xadj[n]];

		cur = 0;
		for (int32_t i = 0; i < n; i++)
		{
			int32_t from = cur;
			int32_t cur_node = gk_to_g.find(i)->second;
			auto& cur_adj = g.get_meta_graph().get_adj_naged(cur_node);
			cur += cur_adj.size();
			for (int32_t j = 0; j < cur_adj.size(); j++)
			{
				gk.adjncy[from + j] = g_to_gk.find(cur_adj[j].target)->second;
				gk.iadjwgt[from + j] = (int64_t)get_aged_value(cur_adj[j].weight, cur_adj[j].modified);
			}
		}

		gk.fadjwgt = NULL;

		gk.ivwgts = new int64_t[n];

		for (int32_t i = 0; i < n; i++)
		{
			int32_t cur_node = gk_to_g.find(i)->second;
			gk.ivwgts[i] = g.get_comps().size[cur_node];
		}

		gk.fvwgts = NULL;

		//Note: these may be set in the future, but can be set to NULL for the current use case.
		gk.ivsizes = NULL;
		gk.fvsizes = NULL;
		gk.vlabels = NULL;

		return gk;
	}

	//Compute a static partitioning given a graph 'g' and the number of desired partitions
	std::vector<int64_t> static_partition(
		comp_graph& g,
		const int32_t& num_partitions,
		const int32_t& max_partition_size,
		int64_t& objval,
		double& total_time)
	{
		auto gk = crep_graph_to_metis_graph(g);

		/* Taken from the METIS manual:
		METIS_API(int) METIS_PartGraphRecursive(idx_t *nvtxs, idx_t *ncon, idx_t *xadj,
				  idx_t *adjncy, idx_t *vwgt, idx_t *vsize, idx_t *adjwgt,
				  idx_t *nparts, real_t *tpwgts, real_t *ubvec, idx_t *options,
				  idx_t *edgecut, idx_t *part);
		*/
		int64_t num_partitions2 = num_partitions;
		int64_t ncon = 1;

		//Allowed imbalance (uncomment below to enable)
		double ubvec = 1.5;

		int64_t* partitions = new int64_t[gk.nvtxs];

		auto begin_time = hrc::now();

		int64_t status = METIS_PartGraphRecursive(
			&gk.nvtxs,
			&ncon,
			gk.xadj,
			gk.adjncy,
			gk.ivwgts,
			gk.ivsizes,
			gk.iadjwgt,
			&num_partitions2,
			NULL,
			NULL,//&ubvec, //Note: here we can specify imbalance for the static partitioning
			NULL,
			&objval,
			partitions);

		auto end_time = hrc::now();
		total_time = std::chrono::duration_cast<dur>(end_time - begin_time).count();

		std::vector<int64_t> node_to_partition(partitions, partitions + gk.nvtxs);

		delete[] partitions;
		delete[] gk.xadj;
		delete[] gk.adjncy;
		delete[] gk.iadjwgt;
		delete[] gk.ivwgts;


#ifdef CREP_DEBUG_LOG_VERTEX_EDGE_SPELLS
		std::string tmp = "\"[";
		for (auto el : node_to_partition)
		{
			tmp += std::to_string(el) + ",";
		}
		tmp.pop_back();
		tmp += "]\"";

		crep::log::logger_instance.add_identifier("static_part",
			crep::get_project_root() + "log/" + crep::log::logger_instance.tstamp + "__static_partitioning" + ".csv",
			tmp);
#endif // CREP_DEBUG_LOG_VERTEX_EDGE_SPELLS


		return node_to_partition;
	}

	//Compute a static partitioning given a request sequence
	std::vector<int64_t> run_static_partition(
		std::unique_ptr<data_source>& ds,
		const int32_t& max_num_vertices,
		const int32_t& num_partitions,
		const int32_t& max_partition_size,
		int64_t& objval,
		double& total_time)
	{
		//Construct the graph
		comp_graph g(max_num_vertices);
		while (ds->has_next())
		{
			auto [src, dst] = ds->next();
			g.change_weight(src, dst, 1);
		}

		//Run the partitioning routine
		std::vector<int64_t> node_to_partition = static_partition(g, num_partitions, max_partition_size, objval, total_time);

		return node_to_partition;
	}


	//Experimental: repartition a graph 'g' given some existing partitioning 'part'
	void adaptive_partition(
		comp_graph& g,
		std::vector<int64_t>& part,
		const int32_t& num_partitions,
		const int32_t& max_partition_size,
		const double& alpha,
		int64_t& objval,
		double& total_time)
	{
		auto gk = crep_graph_to_metis_graph(g);

		int64_t num_partitions2 = num_partitions;

		int64_t ncon = 1;

		double* tpwgts = new double[ncon * num_partitions2]; //Note: fix if ncon != 1
		std::fill(tpwgts, tpwgts + (ncon * num_partitions2), 1.0 / num_partitions2);

		int64_t* vtxdist = new int64_t[2];
		vtxdist[0] = 0;
		vtxdist[1] = g.base_size();

		int64_t wgtflag = 1;
		int64_t numflag = 0;

		int64_t* partitions = new int64_t[gk.nvtxs];
		std::copy(part.begin(), part.end(), partitions);

		//Allowed imbalance. Note: needs finetuning
		double ubvec = 2.1;

		//Trade-off factor. Note: needs finetuning
		double itr = 1.0 / (1.0 * alpha);
		int64_t options = 0;


		auto mpi_com_tmp = MPI_COMM_WORLD;

		auto begin_time = hrc::now();

		int64_t status = ParMETIS_V3_AdaptiveRepart(
			vtxdist,
			gk.xadj,
			gk.adjncy,
			NULL,
			NULL,//vsize,
			gk.iadjwgt,
			&wgtflag,
			&numflag,
			&ncon,
			&num_partitions2,
			tpwgts,
			&ubvec,
			&itr,
			&options,
			&objval,
			partitions,
			&mpi_com_tmp);

		auto end_time = hrc::now();
		total_time += std::chrono::duration_cast<dur>(end_time - begin_time).count();

		std::vector<int64_t> node_to_partition(partitions, partitions + gk.nvtxs);
		std::copy(node_to_partition.begin(), node_to_partition.end(), part.begin());


		delete[] partitions;
		delete[] gk.xadj;
		delete[] gk.adjncy;
		delete[] gk.iadjwgt;
		delete[] gk.ivwgts;

		delete[] vtxdist;
		delete[] tpwgts;
	}

	//Repartition the graph induced by a request sequence every 'num_skip_requests'.
	//Note: num_skip_requests should divide (ds->get_num_batches() * ds->get_batch_size()) without remainder
	void run_adaptive_partition(
		std::unique_ptr<data_source>& ds,
		const int32_t& num_partitions,
		const int32_t& max_partition_size,
		const int32_t& num_skip_requests,
		const double& alpha,
		const int32_t max_num_vertices,
		int_& com_cost,
		real_& mig_cost,
		double& total_time,
		std::vector<id_> vtp = {})
	{
		comp_graph g(max_num_vertices);

		total_time = 0.0;
		double dummy_time = 0.0;
		int64_t dummy_objval = 0;

		if (vtp.empty())
		{
			std::vector<int_> ps(num_partitions, 0);
			vtp = std::vector<id_>(max_num_vertices, -1);
			assign_initial_default(max_num_vertices, num_partitions, max_partition_size, ps, vtp);
		}

		std::vector<int64_t> node_to_partition(vtp);
		auto prev_part(node_to_partition);

		dummy_objval = 0.0;

		com_cost = 0;
		mig_cost = 0;

		int32_t num_iterations = std::ceil((ds->get_num_batches() * ds->get_batch_size()) / num_skip_requests);
		for (int32_t i = 0; i < num_iterations; i++)
		{
			g.reset_edges();

			//Add next 'num_skip_requests' requests to the graph
			int32_t tmp = num_skip_requests;
			std::vector<std::pair<id_, id_>> to_process;
			to_process.reserve(num_skip_requests);
			while (ds->has_next() && (--tmp) >= 0)
			{
				auto [src, dst] = ds->next();
				g.change_weight(src, dst, 1);

				to_process.emplace_back(src, dst);
			}

			//Run the repartitioning routine.
			//Note: maybe only transition to new partitioning only if it would be an improvement
			adaptive_partition(g, node_to_partition, num_partitions, max_partition_size, alpha, dummy_objval, total_time);

			//Compute communication cost
			for (auto el : to_process)
			{
				if (node_to_partition[el.first] != node_to_partition[el.second])
				{
					++com_cost;
				}
			}

			//Compute migration cost
			for (int_ i = 0; i < node_to_partition.size(); i++)
			{
				if (node_to_partition[i] != prev_part[i])
				{
					mig_cost += alpha;
				}
			}

			prev_part = node_to_partition;
		}
	}

	//Experimental: repartition the graph induced by a request sequence every 'num_skip_requests' from back to front.
	//Note: num_skip_requests should divide (ds->get_num_batches() * ds->get_batch_size()) without remainder
	void run_adaptive_partition_from_back(
		std::unique_ptr<data_source>& ds,
		const int32_t& num_partitions,
		const int32_t& max_partition_size,
		const int32_t& num_skip_requests,
		const double& alpha,
		const int32_t max_num_vertices,
		int_& com_cost,
		real_& mig_cost,
		double& total_time)
	{
		comp_graph g(max_num_vertices);
		total_time = 0.0;

		int_ total_num_requests = ds->get_num_batches() * ds->get_batch_size();

		std::vector<id_> src_vec(total_num_requests, -1);
		std::vector<id_> dst_vec(total_num_requests, -1);
		for (int_ i = 0; i < total_num_requests; i++)
		{
			auto [src, dst] = ds->next();
			g.change_weight(src, dst, 1);
			src_vec[i] = src;
			dst_vec[i] = dst;
		}

		//Compute initial static partitioning
		double dummy_time = 0.0;
		int64_t dummy_objval = 0;
		std::vector<int64_t> node_to_partition = static_partition(g, num_partitions, max_partition_size, dummy_objval, dummy_time);
		auto prev_part(node_to_partition);
		dummy_objval = 0.0;

		com_cost = 0;
		mig_cost = 0;

		int32_t num_iterations = std::ceil(total_num_requests / num_skip_requests); //ceil function should not be necessary
		for (int32_t i = num_iterations - 1; i >= 0; --i)
		{
			for (int_ j = (i + 2) * num_skip_requests - 1; j >= (i + 1) * num_skip_requests && j < total_num_requests; --j)
			{
				g.change_weight(src_vec[j], dst_vec[j], -1);
			}

			if (i != num_iterations - 1)
			{
				adaptive_partition(g, node_to_partition, num_partitions, max_partition_size, alpha, dummy_objval, total_time);
			}

			//Compute communication cost
			for (int_ j = (i + 1) * num_skip_requests - 1; j >= i * num_skip_requests; --j)
			{
				if (node_to_partition[src_vec[j]] != node_to_partition[dst_vec[j]])
				{
					++com_cost;
				}
			}

			//Compute migration cost
			for (int_ i = 0; i < node_to_partition.size(); i++)
			{
				if (node_to_partition[i] != prev_part[i])
				{
					mig_cost += alpha;
				}
			}

			prev_part = node_to_partition;
		}

		//Note: the below may be enabled if desired
		//Compute migration cost of the first phase
		/*std::vector<int64_t> node_to_partition2(g.base_size(), -1);
		int_ tmp_counter = 0;
		id_ cur_partition = 0;
		for (int_ i = 0; i < g.base_size(); i++)
		{
			if (tmp_counter == max_partition_size)
			{
				tmp_counter = 0;
				++cur_partition;
			}
			node_to_partition2[i] = cur_partition;
			++tmp_counter;
		}

		for (int_ i = 0; i < node_to_partition.size(); i++)
		{
			if (node_to_partition[i] != node_to_partition2[i])
			{
				mig_cost += alpha;
			}
		}*/
	}


	//Experimental: repartitioning using ParMETIS' ParMETIS_V3_AdaptiveRepart, intended to be run after every request.
	//The existing partitioning is specified via 'part', the crucial trade-off factor is specified via 'itr'.
	//Note: this routine is very similar to adaptive_partition.
	void alt_partition(
		comp_graph& g,
		std::vector<int64_t>& part,
		const int32_t& num_partitions,
		const int32_t& max_partition_size,
		int64_t& objval,
		double& total_time,
		double itr)
	{
		auto gk = crep_graph_to_metis_graph(g);

		int64_t num_partitions2 = num_partitions;

		int64_t ncon = 1;

		double* tpwgts = new double[ncon * num_partitions2]; //Note: fix if ncon != 1
		std::fill(tpwgts, tpwgts + (ncon * num_partitions2), 1.0 / num_partitions2);

		int64_t* vtxdist = new int64_t[2];
		vtxdist[0] = 0;
		vtxdist[1] = g.base_size();

		int64_t wgtflag = 1;
		int64_t numflag = 0;

		int64_t* partitions = new int64_t[gk.nvtxs];
		std::copy(part.begin(), part.end(), partitions);

		//Allowed imbalance. Note: needs finetuning
		double ubvec = 2.1;

		int64_t options = 0;


		auto mpi_com_tmp = MPI_COMM_WORLD;

		auto begin_time = hrc::now();

		int64_t status = ParMETIS_V3_AdaptiveRepart(
			vtxdist,
			gk.xadj,
			gk.adjncy,
			NULL,
			NULL,//vsize,
			gk.iadjwgt,
			&wgtflag,
			&numflag,
			&ncon,
			&num_partitions2,
			tpwgts,
			&ubvec,
			&itr,
			&options,
			&objval,
			partitions,
			&mpi_com_tmp);

		auto end_time = hrc::now();
		total_time += std::chrono::duration_cast<dur>(end_time - begin_time).count();

		std::vector<int64_t> node_to_partition(partitions, partitions + gk.nvtxs);
		std::copy(node_to_partition.begin(), node_to_partition.end(), part.begin());


		delete[] partitions;
		delete[] gk.xadj;
		delete[] gk.adjncy;
		delete[] gk.iadjwgt;
		delete[] gk.ivwgts;

		delete[] vtxdist;
		delete[] tpwgts;
	}

	//Experimental: dynamic partitioning, i.e. consider rebalancing after every request
	void run_alt_partition(
		std::unique_ptr<data_source>& ds,
		const int32_t& num_partitions,
		const int32_t& max_partition_size,
		const double& alpha,
		const int32_t max_num_vertices,
		int_& com_cost,
		real_& mig_cost,
		double& total_time,
		std::vector<id_> vtp = {})
	{
		comp_graph g(max_num_vertices);

		total_time = 0.0;
		double dummy_time = 0.0;
		int64_t dummy_objval = 0;

		if (vtp.empty())
		{
			std::vector<int_> ps(num_partitions, 0);
			vtp = std::vector<id_>(max_num_vertices, -1);
			assign_initial_default(max_num_vertices, num_partitions, max_partition_size, ps, vtp);
		}

		std::vector<int64_t> node_to_partition(vtp);
		auto prev_part(node_to_partition);

		dummy_objval = 0.0;

		com_cost = 0;
		mig_cost = 0;

		double cc_since_last_mig = 0;
		double last_mc = 0;
		double min_itr = 0.0001;
		double max_itr = 1000000;

		double itr = min_itr;

		int32_t num_iterations = ds->get_num_batches() * ds->get_batch_size();

		util::CUR_TIME = 0;
		util::CUR_TIME_AGE = 0;

		for (int32_t i = 0; i < num_iterations; i++)
		{
			//Process the next request
			int32_t tmp = 1;
			while (ds->has_next() && (--tmp) >= 0)
			{
				++util::CUR_TIME;
				if (util::CUR_TIME % time_warp == 0)
				{
					++util::CUR_TIME_AGE;
				}

				auto [src, dst] = ds->next();
				g.change_weight(src, dst, 1.0);

				if (node_to_partition[src] != node_to_partition[dst])
				{
					//Note: simply setting itr = 1.0/alpha sometimes leads to better results, still very inconsistent
					if (last_mc != 0 && cc_since_last_mig != 0)
					{
						itr = cc_since_last_mig / last_mc;
					}
					else if (last_mc != 0)
					{
						itr = 1.0 / alpha;
					}
					else if (cc_since_last_mig != 0)
					{
						itr = cc_since_last_mig;
					}
					else
					{
						itr = min_itr;
					}

					//Repartitioning
					alt_partition(g, node_to_partition, num_partitions, max_partition_size, dummy_objval, total_time, itr);

					//Compute migration cost
					double tmp_mc = 0;
					for (int_ i = 0; i < node_to_partition.size(); i++)
					{
						if (node_to_partition[i] != prev_part[i])
						{
							mig_cost += alpha;
							tmp_mc += alpha;
						}
					}
					if (tmp_mc != 0)
					{
						//Note: calling g.reset_edges() here (instead of below) leads to better results (also makes sense intuitively)
						last_mc = tmp_mc;
						cc_since_last_mig = 0;
					}

					//Compute communication cost
					if (node_to_partition[src] != node_to_partition[dst])
					{
						++com_cost;
						++cc_since_last_mig;
					}

					//Reset the graph
					g.reset_edges();
				}
			}

			prev_part = node_to_partition;
		}
	}
}

// Copyright (c) 2019 Martin Schonger