/*
Primary implementation of the base module.
*/

#pragma once

#include <map>
#include <queue>

#include "brp_alg.h"
#include "comp_graph.h"
#include "charikar.h"

#include "debug.h"

namespace crep
{
	class brp_crep_age : public brp_alg
	{
	public:
		brp_crep_age(
			const std::string& uid,
			const real_& alpha,
			const size_& num_vertices,
			const size_& num_partitions,
			const size_& partition_size,
			const real_& augmentation,
			std::unique_ptr<comp_mig_alg> cm_alg)
			: brp_alg(uid, alpha, num_vertices, num_partitions, partition_size, augmentation, std::move(cm_alg))
		{}

		~brp_crep_age() = default;

		void init() override
		{
			this->g = comp_graph(this->num_v);

			util::CUR_TIME = 0;
			util::CUR_TIME_AGE = 0;
		}

		const std::vector<id_>& get_current_partitioning()
		{
			return this->p_alg->get_vtx_to_part();
		}

		std::string type_to_string() const override
		{
			return "brp_crep";
		}

	protected:
		void inc_edge_helper(const id_& src, const id_& dst) override
		{
			++util::CUR_TIME;
			if (util::CUR_TIME % time_warp == 0)
			{
				++util::CUR_TIME_AGE;
			}

			//Intra-component request?
			if (!g.change_weight(src, dst, 1.0))
			{
				return;
			}

			//Intra-partition request?
			if (p_alg->get_cur_partition(src) == p_alg->get_cur_partition(dst))
			{
				return;
			}

			bool found_mergeable = false;
			real_ sg_weight = 0;

			//Get mergeable component set
			auto res = get_mig_vtxs(g.get_meta_graph(), g.get_comps().f(src), g.get_comps().f(dst), sg_weight, g.get_comps().parent);

			if (res.size() >= 2)
			{
				//Compute total component weight and total number of nodes affected
				int_ affected_vtxs = 0;
				real_ comp_sum = 0;
				for (int_ m = 0; m < res.size(); ++m)
				{
					id_ i = res[m];
					affected_vtxs += g.get_comps().get_size(i);
					comp_sum += g.get_comp_weight(i);
				}

				//Merge or delete returned subgraph iff (no delete) OR (sg is heavy enough)
				if (affected_vtxs <= p_size || comp_sum == 0 || sg_weight >= theta * comp_sum)
				{
					//Merge components
					id_ new_root = -1;
					id_ i = res[0];
					for (int_ m = 1; m < res.size(); m++)
					{
						id_ j = res[m];
						new_root = g.merge(i, j);
					}

					assert(affected_vtxs == g.get_comps().get_size(new_root));

					//Compute set of all vertices in newly merged component
					std::vector<id_> miggy(affected_vtxs);
					int_ idx = 0;
					auto cur = new_root;
					do
					{
						miggy[idx++] = cur;
						cur = g.get_comps().next[cur];
					} while (cur != new_root);

					int_ num_migrated = 0;
					if (affected_vtxs <= p_size)
					{
						//Migrate
						migrate(g.get_base_graph(), miggy);
					}
					else
					{
						//Delete component and split into singletons
						std::string log_nodes_list = "";
						g.delete_comp(new_root, log_nodes_list, false);
					}
				}
			}

			serve_request(src, dst);
		}

	private:
		//Maintain the communication network as graph
		comp_graph g;
	};
}

// Copyright (c) 2019 Martin Schonger