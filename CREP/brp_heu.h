#pragma once

#include <map>
#include <random>
#include <queue>

#include "brp_alg.h"
#include "graph_extended.h"
#include "explore_neighborhood.h"
#include "charikar.h"

#include "debug.h"

namespace crep
{
	class brp_heu : public brp_alg
	{
	public:
		static dur time_explore;
		brp_heu(
			const std::string& uid,
			const real_& alpha, 
			const size_& num_vertices, 
			const size_& num_partitions, 
			const size_& partition_size,
			const real_& augmentation,
			std::unique_ptr<comp_mig_alg> cm_alg)
			: brp_alg(uid, alpha, num_vertices, num_partitions, partition_size, augmentation, std::move(cm_alg))
		{

		}
		
		~brp_heu() = default;
		
		void init() override
		{
			this->g = graph_extended(this->num_v);

			util::CUR_TIME = 0;

			std::random_device rd;
			this->gen = std::mt19937(rd());
			this->dis = std::uniform_real_distribution<>(0.0, 1.0);
		}

		const std::vector<id_>& get_current_partitioning()
		{
			return this->p_alg->get_vtx_to_part();
		}

		std::string type_to_string() const override
		{
			return "brp_heu";
		}

	protected:
		void inc_edge_helper(const id_& src, const id_& dst) override
		{
			++util::CUR_TIME;

			if (this->p_alg->get_cur_partition(src) == this->p_alg->get_cur_partition(dst))
			{
				return; //TODO maybe remove
			}

			g.inc_edge(src, dst);

			auto to_migrate = get_mig_vtxs(g, src, dst);

			if (to_migrate.size() >= 2)
			{
				if (to_migrate.size() <= this->p_alg->get_p_size())
				{
					migrate(g, to_migrate);
				}

				//remove edges
				for (size_ i = 0; i < to_migrate.size(); i++)
				{
					for (size_ j = i + 1; j < to_migrate.size(); j++)
					{
						g.remove_edge(to_migrate[i], to_migrate[j]);
					}
				}
			}


			/*real_ den = (real_)total_weight / ((real_)num_vtxs - 1.0);

			boost::math::skew_normal_distribution<> myskewnorm2(this->alpha, 1, 1);
			real_ mig_prob = std::max(0.005, boost::math::cdf(myskewnorm2, den));
			if (mig_prob < dis(gen))
			{
				return;
			}*/

			//perform migrations
			//int_ num_migrated = this->p_alg->migrate(g, incl_vtx);

			//this->mig_cost += this->alpha * num_migrated;

			serve_request(src, dst);
		}

	private:
		graph_extended g;

		std::mt19937 gen;
		std::uniform_real_distribution<> dis;

	};
}

// Copyright (c) 2019 Martin Schonger