#pragma once

#include "comp_mig_alg.h"

namespace crep
{
	class comp_mig_dummy : public comp_mig_alg
	{
	public:
		comp_mig_dummy() = default;
		~comp_mig_dummy() = default;

		std::vector<id_> get_mig_vtxs(graph_extended& g, const id_& src, const id_& dst) override
		{
			return { src, dst };
		}

		std::string type_to_string() const override
		{
			return "comp_mig_dummy";
		}

		std::string config_to_string() const override
		{
			return "\"\",,";
		}

	private:

	};
}

// Copyright (c) 2019 Martin Schonger