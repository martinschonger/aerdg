#pragma once

#include "comp_mig_alg.h"
#include "explore_neighborhood.h"
#include "charikar.h"

namespace crep
{
	class comp_mig_cc : public comp_mig_alg
	{
	public:
		comp_mig_cc(
			const real_ threshold)
			: threshold(threshold)
		{}

		~comp_mig_cc() = default;

		std::vector<id_> get_mig_vtxs(graph_extended& g, const id_& src, const id_& dst) override
		{
			graph_extended subg;
			std::map<id_, id_> g_to_subg;
			std::vector<id_> subg_to_g;

			explore_cc(g, src, subg, g_to_subg, subg_to_g);

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
			return "comp_mig_cc";
		}

		std::string config_to_string() const override
		{
			return "\"\"," + std::to_string(threshold) + ",";
		}

	private:
		real_ threshold;
	};
}

// Copyright (c) 2019 Martin Schonger