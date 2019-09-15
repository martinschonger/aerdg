#pragma once

#include <map>
#include <queue>

#include "brp_alg.h"
#include "comp_graph.h"
#include "charikar.h"
#include "optimization.h"

#include "debug.h"

namespace crep
{
	class brp_crep : public brp_alg
	{
	public:
		brp_crep(
			const std::string& uid,
			const real_& alpha,
			const size_& num_vertices,
			const size_& num_partitions,
			const size_& partition_size,
			const real_& augmentation,
			std::unique_ptr<comp_mig_alg> cm_alg)
			: brp_alg(uid, alpha, num_vertices, num_partitions, partition_size, augmentation, std::move(cm_alg)) {}

		~brp_crep() = default;

		void init() override
		{
			this->g = comp_graph(this->num_v);

			util::CUR_TIME = 0;
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

			if (p_alg->get_cur_partition(src) == p_alg->get_cur_partition(dst)) //according to CREP paper
			{
				return;
			}

			if (!g.change_weight(src, dst, 1))
			{
				return;
			}

			/*if (p_alg->get_cur_partition(src) == p_alg->get_cur_partition(dst)) //according to CREP paper
			{
				return;
			}*/

			bool found_mergeable = false;

			/*bool precond = check_precondition_mergeable(g.get_meta_graph(), g.get_comps().f(src), alpha);
			if (precond)
			{
			}

			if (!found_mergeable && precond)
			{
				//TODO remove at least the subgraph found by check_precond
				std::cerr << std::to_string(util::CUR_TIME) << " [WARNING] Charikar missed a mergeable subgraph. (src = " << src << ", dst = " << dst << ")" << std::endl;
			}*/

			///res = charikar::exec(found_mergeable, g.get_meta_graph(), g.get_comps().parent, alpha, false);
			auto res = get_mig_vtxs(g.get_meta_graph(), g.get_comps().f(src), g.get_comps().f(dst));

			//test: merge/migrate nodes immediately after a request
			/*found_mergeable = true;
			res = std::vector<int32_t>(g.get_base_graph().v(), -1);
			res[g.get_comps().f(src)] = 0;
			res[g.get_comps().f(dst)] = 0;
			max_component_size_before_delete = clusters.get_actual_cluster_size();*/
			//end test

			if (res.size() >= 2)
			{
				//merge or delete returned subgraph
				id_ new_root = -1;
				id_ i = res[0];
				for (int_ m = 1; m < res.size(); m++)
				{
					id_ j = res[m];
					new_root = g.merge(i, j);
				}

				std::vector<id_> miggy(g.get_comps().get_size(new_root));
				int_ idx = 0;
				auto cur = new_root;
				do
				{
					miggy[idx++] = cur;
					cur = g.get_comps().next[cur];
				} while (cur != new_root);

				int_ num_migrated = 0;
				if (miggy.size() <= this->p_alg->get_p_size())
				{
					//migrate
					migrate(g.get_base_graph(), miggy);
					//std::cerr << util::CUR_TIME << " merged:  " << miggy.size() << ", (" << new_root << "), (src=" << src << ", dst=" << dst << ")" << std::endl;
				}
				else
				{
					//split component
					std::string log_nodes_list = "";
					g.delete_comp(new_root, log_nodes_list, false);
					//std::cerr << util::CUR_TIME << " deleted: " << miggy.size() << ", (" << new_root << "), (src=" << src << ", dst=" << dst << ")" << std::endl;
				}
			}

			serve_request(src, dst);
		}

	private:
		comp_graph g;
	};
}

// Copyright (c) 2019 Martin Schonger