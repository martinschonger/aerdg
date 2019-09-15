#pragma once

#include "comp_mig_alg.h"
#include "explore_neighborhood.h"
#include "charikar.h"

namespace crep
{
	class comp_mig_full : public comp_mig_alg
	{
	public:
		comp_mig_full(
			const real_ threshold)
			: threshold(threshold)
		{}

		~comp_mig_full() = default;

		std::vector<id_> get_mig_vtxs(graph_extended& g, const id_& src, const id_& dst) override
		{
			bool found_mergeable = false;

			std::vector<int_> res_tmp = charikar(found_mergeable, g, threshold);

			if (found_mergeable)
			{
				std::vector<id_> res;
				for (int_ i = 0; i < res_tmp.size(); i++)
				{
					if (res_tmp[i] != -1)
					{
						res.emplace_back(i);
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
			return "comp_mig_full";
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