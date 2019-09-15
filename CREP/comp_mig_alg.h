/*
Update module framework.
*/

#pragma once

#include <string>
#include <vector>

#include "typedefs.h"
#include "graph_extended.h"

namespace crep
{
	//Abstract class for update module
	class comp_mig_alg
	{
	public:
		virtual ~comp_mig_alg() = default;

		//Compute mergeable component set
		virtual std::vector<id_> get_mig_vtxs(graph_extended& g, const id_& src, const id_& dst, real_& sg_weight, const std::vector<int32_t>& parent = {}) = 0;

		//Get type information of update module
		virtual std::string type_to_string() const = 0;

		//Get config of update module
		virtual std::string config_to_string() const = 0;

	protected:
		comp_mig_alg() = default;
	};
}

// Copyright (c) 2019 Martin Schonger