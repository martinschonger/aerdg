/*
Implementation of the update module that calls Charikar on the entire graph.
*/

#pragma once

#include "comp_mig_alg.h"
#include "explore_neighborhood.h"
#include "charikar.h"

namespace crep
{
	class comp_mig_age : public comp_mig_alg
	{
	public:
		comp_mig_age(
			const real_ threshold)
			: threshold(threshold)
		{}

		~comp_mig_age() = default;

		std::vector<id_> get_mig_vtxs(graph_extended& g, const id_& src, const id_& dst, real_& sg_weight, const std::vector<int32_t>& parent = {}) override
		{
			bool found_mergeable = false;

			//Call charikar on 'g'
			auto res_tmp = charikar_age(found_mergeable, g, threshold, sg_weight, parent);

			if (found_mergeable)
			{
				std::vector<id_> res;
				for (int_ i = 0; i < res_tmp.size(); i++)
				{
					if (res_tmp[i] >= -1e-8)
					{
						//Add vertex to result
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
			return "comp_mig_age";
		}

		std::string config_to_string() const override
		{
			return "\"\"," + std::to_string(threshold) + ",";
		}

	private:
		//Threshold on the CREP-density for a component to be considered mergeable
		real_ threshold;
	};
}

// Copyright (c) 2019 Martin Schonger