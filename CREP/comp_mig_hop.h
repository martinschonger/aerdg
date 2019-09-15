#pragma once

#include "comp_mig_alg.h"
#include "explore_neighborhood.h"
#include "charikar.h"

namespace crep
{
	class comp_mig_hop : public comp_mig_alg
	{
	public:
		comp_mig_hop(
			const int_ num_hops,
			const real_ threshold)
			: num_hops(num_hops), threshold(threshold)
		{}

		~comp_mig_hop() = default;

		std::vector<id_> get_mig_vtxs(graph_extended& g, const id_& src, const id_& dst) override
		{
			graph_extended subg;
			std::map<id_, id_> g_to_subg;
			std::vector<id_> subg_to_g;

			explore_hop(g, src, dst, num_hops, subg, g_to_subg, subg_to_g);

			bool found_mergeable = false;

			std::vector<id_> res_subg = charikar(found_mergeable, subg, threshold);

			if (found_mergeable)
			{
				std::vector<id_> res;
				for (int_ i = 0; i < res_subg.size(); i++)
				{
					if (res_subg[i] != -1)
					{
						res.emplace_back(subg_to_g[i]);
						//res[subg_to_g[i]] = res_subg[i];
					}
				}
				return res;
			}
			else
			{
				return {};
			}
		}

		std::string type_to_string() const override
		{
			return "comp_mig_hop";
		}

		std::string config_to_string() const override
		{
			return "\"\"," + std::to_string(threshold) + "," + std::to_string(num_hops);
		}

	private:
		int_ num_hops;
		real_ threshold;
	};
}

// Copyright (c) 2019 Martin Schonger