/*
Implementation of the update module that calls Charikar on the hop neighborhood of the most recent request.
*/

#pragma once

#include "comp_mig_alg.h"
#include "explore_neighborhood.h"
#include "charikar.h"

namespace crep
{
	class comp_mig_hop_age : public comp_mig_alg
	{
	public:
		comp_mig_hop_age(
			const int_ num_hops,
			const real_ threshold)
			: num_hops(num_hops), threshold(threshold)
		{}

		~comp_mig_hop_age() = default;

		std::vector<id_> get_mig_vtxs(graph_extended& g, const id_& src, const id_& dst, real_& sg_weight, const std::vector<int32_t>& parent = {}) override
		{
			graph_extended subg;
			std::map<id_, id_> g_to_subg;
			std::vector<id_> subg_to_g;

			//Construct the (num_hops)-hop neighborhood of the edge (src,dst)
			explore_hop(g, src, dst, num_hops, subg, g_to_subg, subg_to_g);

			bool found_mergeable = false;

			//Call charikar on 'subg'
			auto res_subg = charikar_age(found_mergeable, subg, threshold, sg_weight);

			if (found_mergeable)
			{
				std::vector<id_> res;
				for (int_ i = 0; i < res_subg.size(); i++)
				{
					if (res_subg[i] >= -1e-8)
					{
						//Add vertex to result
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
			return "comp_mig_hop_age";
		}

		std::string config_to_string() const override
		{
			return "\"\"," + std::to_string(threshold) + "," + std::to_string(num_hops);
		}

	private:
		//Maximal hop distance
		int_ num_hops;
		
		//Threshold on the CREP-density for a component to be considered mergeable
		real_ threshold;
	};
}

// Copyright (c) 2019 Martin Schonger