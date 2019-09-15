/*
Primary implementation of the partition module.
*/

#pragma once

#include <set>

#include "part_alg.h"
#include "debug.h"

namespace crep
{
	class part_simple : public part_alg
	{
	public:
		part_simple(const size_& num_vertices,
			const size_& num_partitions,
			const size_& partition_size,
			const real_& augmentation,
			const real_& alpha,
			get_target_part_ get_target_part,
			const std::string& get_target_part_name,
			bool logging_enabled = false) : part_alg(num_vertices, num_partitions, partition_size, augmentation, alpha, get_target_part, get_target_part_name, logging_enabled)
		{}
		~part_simple() = default;

		id_ get_least_full_part() override
		{
			std::pair<int_, id_> pq_elem{ -1, -1 };

			while (!pq.empty() && part_size[(pq_elem = pq.top()).second] != pq_elem.first)
			{
				pq.pop();
			}

			if (pq.empty())
			{
				std::cerr << "[CHECKME] no valid partition found" << std::endl;
				return -1;
			}

			return pq_elem.second;
		}

		std::string type_to_string() const override
		{
			return "part_simple";
		}

	protected:
		int_ migrate_helper(graph_extended& g, const std::vector<id_>& to_mig) override
		{
			int_ num_migrated = 0;
			int_ num_vtx_mig = to_mig.size();

			std::vector<int_> utilized(num_p, 0);
			for (size_ i = 0; i < num_vtx_mig; i++)
			{
				++utilized[vtx_to_part[to_mig[i]]];
			}

			id_ target_partition = get_target_partition(g, to_mig);

			for (size_ i = 0; i < num_vtx_mig; i++)
			{
				id_ cur = to_mig[i];
				if (vtx_to_part[cur] != target_partition)
				{
					//Actually move vertex cur
					move_vtx(cur, target_partition);
					g.migrated[cur] = util::CUR_TIME;
				}
			}

			if (part_size[target_partition] > get_actual_p_size())
			{
				std::cerr << "[DEBUG] part_size[target_partition] (=" << part_size[target_partition] << ") > get_actual_p_size() (=" << get_actual_p_size() << ")." << std::endl;
			}

			//Update the priority queue with the new partition sizes
			pq.emplace(part_size[target_partition], target_partition);

			for (size_ i = 0; i < this->num_p; i++)
			{
				if (i != target_partition && utilized[i] > 0)
				{
					pq.emplace(part_size[i], i);
					num_migrated += utilized[i];
				}
			}

			return num_migrated;
		}

	private:

	};

	/*
	Default strategy for computing a target partition:
	 Let S be the set of partitions containing at least one vertex of to_mig.
	 If S is not empty, select the partition containing the most vertices of to_mig.
	 Else, select any partition with maximum free capacity.
	*/
	inline id_ get_target_part_simple(part_alg& party, const graph_extended& g, const std::vector<id_>& to_mig)
	{
		int_ num_vtx_mig = to_mig.size();

		//Note: this vector could also be passed from the outside to avoid unnecessary computation.
		std::vector<int_> utilized(party.get_num_p(), 0);
		for (size_ i = 0; i < num_vtx_mig; i++)
		{
			++utilized[party.get_vtx_to_part()[to_mig[i]]];
		}

		int_ min_mig_effort = num_vtx_mig;
		id_ target_partition = -1;
		for (size_ i = 0; i < num_vtx_mig; i++)
		{
			id_ cur = to_mig[i];
			id_ cur_partition = party.get_vtx_to_part()[cur];
			if (party.get_part_size()[cur_partition] + (num_vtx_mig - utilized[cur_partition]) <= party.get_actual_p_size()
				&& (num_vtx_mig - utilized[cur_partition]) < min_mig_effort)
			{
				min_mig_effort = (num_vtx_mig - utilized[cur_partition]);
				target_partition = cur_partition;
			}
		}

		if (target_partition == -1)
		{
			target_partition = party.get_least_full_part();
		}

		return target_partition;
	}

	/*
	More sophisticated strategy for computing a target partition:
	 The base logic is the same as for get_target_part_simple.
	 Additionally, here we incorporate factors such as number of newly cut edges.
	*/
	inline id_ get_target_part_smarter(part_alg& party, const graph_extended& g, const std::vector<id_>& to_mig)
	{
		int_ num_vtx_mig = to_mig.size();

		std::set<id_> to_mig_set(to_mig.begin(), to_mig.end());
		std::vector<int_> utilized(party.get_num_p(), 0);
		std::vector<int_> mig_effort(party.get_num_p(), num_vtx_mig);

		//cut_score[i]: these many edges will be cut if partition i is 'not' the target partition
		std::vector<int_> cut_score(party.get_num_p(), 0);
		int_ total_cut_score = 0;

		for (const auto& cur : to_mig_set)
		{
			id_ cur_partition = party.get_vtx_to_part()[cur];
			++utilized[cur_partition];
			--mig_effort[cur_partition];

			for (const auto& e : g.get_adj_naged(cur))
			{
				if (cur_partition == party.get_vtx_to_part()[e.target]
					&& to_mig_set.find(e.target) == to_mig_set.end())
				{
					real_ aged_weight = get_aged_value(e.weight, e.modified);
					cut_score[cur_partition] += aged_weight;
					total_cut_score += aged_weight;
				}
			}
		}

		/*
		Factors for partition i:
		 - number of edges that will be cut if i is target_partition:	total_cut_score - cut_score[i]
		 - number of nodes that will be migrated:						mig_effort[i]
		 - number of nodes (if any) that would have to be evicted
		   and migrated to other partitions:							max(0, party.get_part_size()[i] + mig_effort[i] - party.get_actual_p_size())
		*/

		//Consider all feasible partitions that contain at least one vertex of to_mig
		bool first_score = true;
		real_ best_score = 0;
		id_ target_partition = -1;
		for (size_ i = 0; i < party.get_num_p(); i++)
		{
			if (utilized[i] == 0 || party.get_part_size()[i] + mig_effort[i] - party.get_actual_p_size() > 0)
			{
				continue;
			}

			real_ score_cut = (real_)cut_score[i] / (real_)std::max(1LL, (long long int)total_cut_score);
			real_ score_mig = (real_)mig_effort[i] / (real_)num_vtx_mig;
			real_ score = (1.0 / party.get_alpha()) * (+score_cut) + (-score_mig);

			if (first_score || score > best_score)
			{
				first_score = false;
				best_score = score;
				target_partition = i;
			}
		}

		//Consider a totally different partition (the least full one)
		id_ tmp_target_partition = party.get_least_full_part();
		real_ score_cut = (real_)cut_score[tmp_target_partition] / (real_)std::max(1LL, (long long int)total_cut_score);
		real_ score_mig = (real_)mig_effort[tmp_target_partition] / (real_)num_vtx_mig;
		real_ score = (+score_cut) + party.get_alpha() * (-score_mig);
		if (first_score || score > best_score)
		{
			first_score = false;
			best_score = score;
			target_partition = tmp_target_partition;
		}

		if (party.get_part_size()[target_partition] + mig_effort[target_partition] - party.get_actual_p_size() > 0)
		{
			std::cerr << "[DEBUG] party.get_part_size()[target_partition] (=" << party.get_part_size()[target_partition] << ") + mig_effort[target_partition] (=" << mig_effort[target_partition] << ") - party.get_actual_p_size() (=" << party.get_actual_p_size() << ") > 0" << std::endl;
		}

		return target_partition;
	}
}

// Copyright (c) 2019 Martin Schonger