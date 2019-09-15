/*
Partition module framework.
*/

#pragma once

#include <vector>
#include <queue>
#include <functional>
#include <string>

#include "typedefs.h"
#include "graph_extended.h"
#include "assign_initial.h"

namespace crep
{
	class part_alg;

	//Interface for functions to compute the target partition, given a set of vertices to collocate
	typedef std::function<id_(part_alg&, const graph_extended&, const std::vector<id_>&)> get_target_part_;

	//Abstract class for partition module
	class part_alg
	{
	public:
		virtual ~part_alg() = default;

		//Get the partition having the most free capacity
		virtual id_ get_least_full_part() = 0;

		//Get type information of partition module
		virtual std::string type_to_string() const = 0;

		//Init the partition module
		void init(std::vector<int_> ps, std::vector<id_> vtp)
		{
			assign_initial(ps, vtp);

			for (size_ i = 0; i < num_p; i++)
			{
				pq.emplace(part_size[i], i);
			}
		}

		//Wrapper for 'migrate_helper' to support logging
		int_ migrate(graph_extended& g, const std::vector<id_>& to_mig)
		{
			int_ num_migrated = migrate_helper(g, to_mig);

			if (logging_enabled)
			{

			}

			return num_migrated;
		}

		//Wrapper for computing/assigning the initial partitioning to support logging
		void assign_initial(std::vector<int_> ps, std::vector<id_> vtp)
		{
			if (vtp.empty() || ps.empty())
			{
				assign_initial_default(num_v, num_p, p_size, part_size, vtx_to_part);
			}
			else
			{
				std::copy(ps.begin(), ps.end(), part_size.begin());
				std::copy(vtp.begin(), vtp.end(), vtx_to_part.begin());
			}

			if (logging_enabled)
			{

			}
		}

		size_ get_num_v() const
		{
			return num_v;
		}

		size_ get_num_p() const
		{
			return num_p;
		}

		size_ get_p_size() const
		{
			return p_size;
		}

		//Get augmented partition size
		size_ get_actual_p_size() const
		{
			return std::ceil(augm * p_size);
		}

		real_ get_alpha() const
		{
			return alpha;
		}

		std::vector<int_>& get_part_size()
		{
			return part_size;
		}

		std::vector<id_>& get_vtx_to_part()
		{
			return vtx_to_part;
		}

		//Get partition of vertex
		id_ get_cur_partition(const id_& vertex_id) const
		{
			return vtx_to_part[vertex_id];
		}

		//Convenience wrapper for 'get_target_part'
		id_ get_target_partition(const graph_extended& g, const std::vector<id_>& to_mig)
		{
			return get_target_part(*this, g, to_mig);
		}

		//Get config of partition module
		virtual std::string config_to_string() const
		{
			return "\"get_target_part=" + get_target_part_name + "\"";
		}

	protected:
		part_alg(const size_& num_vertices,
			const size_& num_partitions,
			const size_& partition_size,
			const real_& augmentation,
			const real_& alpha,
			get_target_part_ get_target_part,
			const std::string& get_target_part_name,
			bool logging_enabled = false)
			: num_v(num_vertices), num_p(num_partitions), p_size(partition_size), augm(augmentation), alpha(alpha), get_target_part(get_target_part), get_target_part_name(get_target_part_name), logging_enabled(logging_enabled)
		{
			part_size = std::vector<int_>(num_p, 0);
			vtx_to_part = std::vector<id_>(num_v, -1);
		}

		size_ num_v;
		size_ num_p;
		int_ p_size;
		real_ augm;
		real_ alpha;

		//Partition sizes/utilization
		std::vector<int_> part_size;

		//Partition assignment of vertices
		std::vector<id_> vtx_to_part;

		pq_pair pq{ compare_pair_greater };

		get_target_part_ get_target_part;
		std::string get_target_part_name;

		bool logging_enabled;

		//Perform the actual migrations
		virtual int_ migrate_helper(graph_extended& g, const std::vector<id_>& to_mig) = 0;

		//Wrapper for 'move_vtx_helper' to support logging
		void move_vtx(const id_& vertex_id, const id_& target_partition)
		{
			move_vtx_helper(vertex_id, target_partition);

			if (logging_enabled)
			{

			}
		}

		//Reassign a single vertex to another partition
		void move_vtx_helper(const id_& vertex_id, const id_& target_partition)
		{
			--part_size[vtx_to_part[vertex_id]];
			vtx_to_part[vertex_id] = target_partition;
			++part_size[target_partition];
		}
	};
}

// Copyright (c) 2019 Martin Schonger