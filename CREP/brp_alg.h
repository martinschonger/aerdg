/*
Base module framework.
*/

#pragma once

#include <type_traits>
#include <map>
#include <chrono>
#include <filesystem>

#include "typedefs.h"
#include "comp_mig_alg.h"
#include "part_alg.h"
#include "graph.h"
#include "utility.h"
#include "logger2.h"

namespace crep
{
	//Abstract class for base module
	class brp_alg
	{
	public:
		virtual ~brp_alg() = default;

		//Init the partition module with the 'get_target_part' strategy for computing a target partition for migration
		template<typename T>
		void init_p_alg(get_target_part_ get_target_part, const std::string& get_target_part_name, std::vector<int_> ps = {}, std::vector<id_> vtp = {})
		{
			static_assert(std::is_base_of<part_alg, T>::value, "template type not derived from part_alg");
			p_alg = std::make_unique<T>(num_v, num_p, p_size, augm, alpha, get_target_part, get_target_part_name);
			p_alg->init(ps, vtp);

			if (logging_enabled)
			{
				loggy->log("time", get_uid());
				loggy->log("time_get_mig_vtxs", get_uid());
				loggy->log("time_migrate", get_uid());
				loggy->log("com_cost", get_uid());
				loggy->log("mig_cost", get_uid());
				loggy->log("num_migs", get_uid());
				loggy->log("migs", get_uid());
				//loggy->log("tmp", get_uid());
			}
		}

		//Init the base module
		virtual void init() = 0;

		//Wrapper for 'inc_edge_helper' to support logging
		void inc_edge(const id_& src, const id_& dst)
		{
			if (logging_enabled)
			{
				auto time_start = hrc::now();
				inc_edge_helper(src, dst);
				auto time_finish = hrc::now();
				auto time_duration = std::chrono::duration_cast<dur>(time_finish - time_start);
				time_total += time_duration.count();
				loggy->log("time", std::to_string(util::CUR_TIME) + "," + std::to_string(time_duration.count()));
			}
			else
			{
				inc_edge_helper(src, dst);
			}
		}

		//Get the partitioning at the present time, as required by the BRP
		virtual const std::vector<id_>& get_current_partitioning() = 0;

		//Get type information of base module
		virtual std::string type_to_string() const = 0;

		real_ get_com_cost() const
		{
			return com_cost;
		}

		real_ get_mig_cost() const
		{
			return mig_cost;
		}

		real_ get_total_cost() const
		{
			return com_cost + mig_cost;
		}

		real_ get_time_total() const
		{
			return time_total;
		}

		//Get config of base module
		virtual std::string config_to_string() const
		{
			return get_uid()
				+ "," + type_to_string()
				+ "," + cm_alg->type_to_string()
				+ "," + cm_alg->config_to_string()
				+ "," + p_alg->type_to_string()
				+ "," + p_alg->config_to_string()
				+ "," + std::to_string(alpha)
				+ "," + std::to_string(num_v)
				+ "," + std::to_string(num_p)
				+ "," + std::to_string(p_size)
				+ "," + std::to_string(augm)
				+ "," + std::to_string(p_alg->get_actual_p_size());
		}

		//Get partition module
		const part_alg& get_p_alg() const
		{
			return *p_alg;
		}

		const std::string get_uid() const
		{
			return uid;
		}

		void enable_logging(const std::filesystem::path& log_dir)
		{
			logging_enabled = true;

			time_total = 0;

			std::filesystem::path filename_time(uid + "__time.csv");
			std::filesystem::path filename_time_get_mig_vtxs(uid + "__time_get_mig_vtxs.csv");
			std::filesystem::path filename_time_migrate(uid + "__time_migrate.csv");
			std::filesystem::path filename_com_cost(uid + "__com_cost.csv");
			std::filesystem::path filename_mig_cost(uid + "__mig_cost.csv");
			std::filesystem::path filename_num_migs(uid + "__num_migs.csv");
			std::filesystem::path filename_migs(uid + "__migs.csv");
			//std::filesystem::path filename_tmp(uid + "__tmp.csv");
			loggy = std::make_unique<log::logger2>(log_dir);

			//Init separately logged quantities
			loggy->add_identifier("time", filename_time, "request,time");
			loggy->add_identifier("time_get_mig_vtxs", filename_time_get_mig_vtxs, "request,time");
			loggy->add_identifier("time_migrate", filename_time_migrate, "request,time");
			loggy->add_identifier("com_cost", filename_com_cost, "request,com_cost");
			loggy->add_identifier("mig_cost", filename_mig_cost, "request,mig_cost,delta_mig_cost");
			loggy->add_identifier("num_migs", filename_num_migs, "vtx_id,num_migs");
			loggy->add_identifier("migs", filename_migs, "request,num_colloc,num_mig");
			//loggy->add_identifier("tmp", filename_tmp, "edge_weights");

			log_num_migs.resize(num_v, 0);
		}

		//Write log buffers to file
		void finish_logging()
		{
			for (int_ i = 0; i < num_v; i++)
			{
				loggy->log("num_migs", std::to_string(i) + "," + std::to_string(log_num_migs[i]));
			}

			loggy->all_to_file();
		}

	protected:
		brp_alg(
			const std::string& uid,
			const real_& alpha,
			const size_& num_vertices,
			const size_& num_partitions,
			const size_& partition_size,
			const real_& augmentation,
			std::unique_ptr<comp_mig_alg> cm_alg)
			: uid(uid), alpha(alpha), num_v(num_vertices), num_p(num_partitions), p_size(partition_size), augm(augmentation), cm_alg(std::move(cm_alg)), logging_enabled(false)
		{
			com_cost = 0;
			mig_cost = 0;
		}

		//Globally unique identifier, mainly for logging purposes
		const std::string uid;

		real_ alpha;
		size_ num_v;
		size_ num_p;
		size_ p_size;
		real_ augm;

		real_ com_cost;
		real_ mig_cost;

		std::unique_ptr<comp_mig_alg> cm_alg;

		bool logging_enabled;
		std::unique_ptr<log::logger2> loggy;
		real_ time_total;
		std::vector<int_> log_num_migs;

		std::unique_ptr<part_alg> p_alg;

		//Process a request from src to dst, main point of user interaction
		virtual void inc_edge_helper(const id_& src, const id_& dst) = 0;

		//Wrapper for cm_alg->get_mig_vtxs to support logging
		std::vector<id_> get_mig_vtxs(graph_extended& gr, const id_& src, const id_& dst, real_& sg_weight, const std::vector<int32_t>& parent = {})
		{
			if (logging_enabled)
			{
				auto time_start = hrc::now();
				auto tmp_mig_vtxs = cm_alg->get_mig_vtxs(gr, src, dst, sg_weight, parent);
				auto time_finish = hrc::now();
				auto time_duration = std::chrono::duration_cast<dur>(time_finish - time_start);
				loggy->log("time_get_mig_vtxs", std::to_string(util::CUR_TIME) + "," + std::to_string(time_duration.count()));
				return tmp_mig_vtxs;
			}
			else
			{
				return cm_alg->get_mig_vtxs(gr, src, dst, sg_weight, parent);
			}
		}

		//Wrapper for 'p_alg->migrate' to support logging
		void migrate(graph_extended& g, const std::vector<id_>& to_mig)
		{
			int_ num_migrated = 0;

			if (logging_enabled)
			{
				auto time_start = hrc::now();
				num_migrated = p_alg->migrate(g, to_mig);
				auto time_finish = hrc::now();
				auto time_duration = std::chrono::duration_cast<dur>(time_finish - time_start);
				loggy->log("time_migrate", std::to_string(util::CUR_TIME) + "," + std::to_string(time_duration.count()));
				loggy->log("migs", std::to_string(util::CUR_TIME) + "," + std::to_string(to_mig.size()) + "," + std::to_string(num_migrated));
			}
			else
			{
				num_migrated = p_alg->migrate(g, to_mig);
			}

			real_ delta_mig_cost = alpha * num_migrated;
			mig_cost += delta_mig_cost;

			if (logging_enabled)
			{
				loggy->log("mig_cost", std::to_string(util::CUR_TIME) + "," + std::to_string(mig_cost) + "," + std::to_string(delta_mig_cost));
				for (auto vtx_id : to_mig)
				{
					++log_num_migs[vtx_id];
				}
			}
		}

		//Log com costs for serving a request from 'src' to 'dst'
		void serve_request(const id_& src, const id_& dst)
		{
			if (p_alg->get_cur_partition(src) != p_alg->get_cur_partition(dst))
			{
				++com_cost;

				if (logging_enabled)
				{
					loggy->log("com_cost", std::to_string(util::CUR_TIME) + ",1");
				}
			}
		}
	};
}

// Copyright (c) 2019 Martin Schonger