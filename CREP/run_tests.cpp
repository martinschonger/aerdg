/*
Facilitate automated tests with differnt algorithms and configurations.
*/

#include "run_tests.h"

int main(int argc, char* argv[])
{
#if defined(__GNUC__)
	MPI_Init(&argc, &argv);
#endif

	//Setup logging
	std::string main_timestamp = crep::get_timestamp();
	std::filesystem::path log_dir = crep::get_log_dir() + main_timestamp;
	std::filesystem::create_directory(log_dir);
	crep::log::logger2 loggy(log_dir);
	loggy.add_identifier("summary", main_timestamp + "__summary.csv",
		"run_number,uid,base_alg,expl_strat,expl_strat_specifics,threshold,add_param,part_strat,part_strat_specifics,alpha,num_v,num_p,p_size,augm,actual_p_size,db,trace,db_trace_specifics," + crep::aging_config_to_string_header() + ",time_total,com_cost,mig_cost");

	std::string master_config_filename = crep::get_project_root();
	master_config_filename += "config/";
	master_config_filename += argv[1];
	master_config_filename += ".txt";

	std::vector<std::string> config_filenames;
	crep::read_all_lines(master_config_filename, config_filenames);


	auto uid_base = main_timestamp;
	crep::int_ alg_counter = 0;


	//Iterate tests for each config file
	for (auto cfile : config_filenames)
	{
		//Config
		std::string config_file = crep::trim_alt(cfile);
		config_file += ".txt";
		std::string config_filename(crep::get_project_root() + "config/" + config_file);
		std::cerr << "[INFO] Reading config from '" << config_filename << "'." << std::endl;
		auto config = crep::read_config(config_filename);

		crep::int_ num_requests = std::stoi(config.at("num_requests"));
		double augmentation = std::stod(config.at("augmentation"));

		std::stringstream iss(config.at("alpha"));
		std::vector<double> alpha_set;
		{
			double cur_alpha;
			while (iss >> cur_alpha)
			{
				alpha_set.emplace_back(cur_alpha);
			}
		}

		iss = std::stringstream(config.at("partition_size"));
		std::vector<int32_t> partition_size_set;
		{
			int32_t cur_partition_size;
			while (iss >> cur_partition_size)
			{
				partition_size_set.emplace_back(cur_partition_size);
			}
		}

		iss = std::stringstream(config.at("num_partitions"));
		std::vector<int32_t> num_partitions_set;
		{
			int32_t cur_num_partitions;
			while (iss >> cur_num_partitions)
			{
				num_partitions_set.emplace_back(cur_num_partitions);
			}
		}

		iss = std::stringstream(config.at("gamma"));
		std::vector<crep::real_> gamma_set;
		{
			crep::real_ cur_gamma;
			while (iss >> cur_gamma)
			{
				gamma_set.emplace_back(cur_gamma);
			}
		}

		iss = std::stringstream(config.at("time_warp"));
		std::vector<crep::tstamp_> time_warp_set;
		{
			crep::tstamp_ cur_time_warp;
			while (iss >> cur_time_warp)
			{
				time_warp_set.emplace_back(cur_time_warp);
			}
		}

		iss = std::stringstream(config.at("theta"));
		std::vector<crep::real_> theta_set;
		{
			crep::real_ cur_theta;
			while (iss >> cur_theta)
			{
				theta_set.emplace_back(cur_theta);
			}
		}

		crep::int_ runs_per_config = std::stoi(config.at("num_runs"));

		iss = std::stringstream(config.at("algs"));
		std::vector<std::string> alg_set;
		{
			std::string cur_alg;
			while (iss >> cur_alg)
			{
				alg_set.emplace_back(cur_alg);
			}
		}

		assert(partition_size_set.size() == num_partitions_set.size());

		int32_t num_vertices = std::stoi(config.at("num_vertices")); //upper bound -> for dynamic partitioning graph will be initialized with this amount of vertices

		std::string database_name, collection_name;
		auto db_iter = config.find("database_name");
		if (db_iter != config.end())
		{
			database_name = config.at("database_name");
			collection_name = config.at("collection_name");
		}
		else //Fallback for older config files
		{
			database_name = "p_fab";
			collection_name = "trace_0_5";
		}

		std::filesystem::copy(config_filename, log_dir / config_file);


		std::unique_ptr<crep::data_source> ds;

		//Number of requests to be skipped, in addition to config-specific skips
		crep::int_ base_skip = 1500000;

		//Random engine for initial partitioning
		auto g = std::default_random_engine{};

		//Stores the different BRP algorithms for each parameter combination
		std::vector<std::unique_ptr<crep::brp_alg>> algs;


		for (crep::int_ lk = 0; lk < num_partitions_set.size(); lk++)
		{
			auto num_partitions = num_partitions_set[lk];
			auto partition_size = partition_size_set[lk];

			std::vector<crep::int_> ps_random(num_partitions, 0);
			std::vector<crep::id_> vtp_random(num_vertices, -1);

			//Reset random engine
			g = std::default_random_engine{};

			for (crep::int_ run_number = 0; run_number < runs_per_config; ++run_number)
			{
				//Compute starting point in trace
				crep::int_ cur_skip = base_skip + (run_number * num_requests);

				//Init request sequence
				ds = std::make_unique<crep::data_mongo_offline>(database_name, collection_name, num_requests, cur_skip);

				//Compute initial partitioning
				crep::assign_initial_random(num_vertices, num_partitions, partition_size, ps_random, vtp_random, g);

				for (crep::tstamp_ warpy : time_warp_set)
				{
					for (crep::real_ gammy : gamma_set)
					{
						for (crep::real_ thety : theta_set)
						{
							crep::time_warp = warpy;
							crep::gamma = gammy;
							crep::theta = thety;

							for (auto alpha : alpha_set)
							{
								std::cerr << " [INFO] Current config: " << "alpha=" << alpha << ", K=" << partition_size << ", L=" << num_partitions << std::endl;

								ds->reset();
								algs.clear();


								//Static offline algorithm
								bool enable_stat = false;
								if (enable_stat)
								{
									int64_t com_cost_stat = 0;
									double total_time_stat = 0.0;
									//If augmentation != 1.0, imbalance may be configured in static_partitioning.h
#if defined(__GNUC__)
									auto node_to_partition = run_static_partition(ds, num_vertices, num_partitions, partition_size, com_cost_stat, total_time_stat);
#endif
									std::cerr << "[static]" << std::endl << " time=" + std::to_string(total_time_stat) << std::endl << " cost=" << std::to_string(com_cost_stat) << std::endl;

									loggy.log("summary", std::to_string(run_number) + "," + uid_base + "_" + std::to_string(alg_counter++) + ",static,,,,,,," + std::to_string(alpha) + "," + std::to_string(num_vertices) + "," + std::to_string(num_partitions) + "," + std::to_string(partition_size) + ",0," + std::to_string(partition_size) + "," + ds->config_to_string() + "," + crep::aging_config_to_string_placeholder() + "," + std::to_string(total_time_stat) + "," + std::to_string(com_cost_stat) + ",0");
									loggy.all_to_file();

									ds->reset();
								}


								//Dynamic online algorithms
								std::vector<crep::real_> thresholds{ alpha };
								if (gammy != 1.0) //Optionally use lower thresholds based on current config
								{
									//thresholds.emplace_back(gammy * alpha);
								}

								for (auto tholdy : thresholds)
								{
									if (std::find(alg_set.begin(), alg_set.end(), std::string("NAIVE")) != alg_set.end())
									{
										{
											auto cm_alg = std::make_unique<crep::comp_mig_age>(tholdy);
											auto alg = std::make_unique<crep::brp_crep_age>(uid_base + "_" + std::to_string(alg_counter++), alpha, num_vertices, num_partitions, partition_size, augmentation, std::move(cm_alg));
											alg->enable_logging(log_dir);
											alg->init();
											alg->init_p_alg<crep::part_simple>(crep::get_target_part_simple, "simple", ps_random, vtp_random);
											algs.push_back(std::move(alg));
										}
									}
									if (std::find(alg_set.begin(), alg_set.end(), std::string("CC")) != alg_set.end())
									{
										{
											auto cm_alg = std::make_unique<crep::comp_mig_cc_age>(tholdy);
											auto alg = std::make_unique<crep::brp_crep_age>(uid_base + "_" + std::to_string(alg_counter++), alpha, num_vertices, num_partitions, partition_size, augmentation, std::move(cm_alg));
											alg->enable_logging(log_dir);
											alg->init();
											alg->init_p_alg<crep::part_simple>(crep::get_target_part_simple, "simple", ps_random, vtp_random);
											algs.push_back(std::move(alg));
										}
									}
									if (std::find(alg_set.begin(), alg_set.end(), std::string("HOP")) != alg_set.end())
									{
										{
											//Init update module
											auto cm_alg = std::make_unique<crep::comp_mig_hop_age>(2, tholdy);

											//Init base module
											auto alg = std::make_unique<crep::brp_crep_age>(uid_base + "_" + std::to_string(alg_counter++), alpha, num_vertices, num_partitions, partition_size, augmentation, std::move(cm_alg));
											alg->enable_logging(log_dir);
											alg->init();

											//Init partition module
											alg->init_p_alg<crep::part_simple>(crep::get_target_part_simple, "simple", ps_random, vtp_random);

											algs.push_back(std::move(alg));
										}
									}
								}

								//Iterate dynamic algorithms
								for (auto&& alg : algs)
								{
									//Reset global clocks
									crep::util::CUR_TIME = 0;
									crep::util::CUR_TIME_AGE = 0;

									auto begin_alg = crep::hrc::now();
									//Process entire request sequence
									while (ds->has_next())
									{
										auto [src, dst] = ds->next();
										alg->inc_edge(src, dst);
									}

									auto time_alg = std::chrono::duration_cast<crep::dur>(crep::hrc::now() - begin_alg);
									alg->finish_logging();

									std::string log_entry = std::to_string(run_number)
										+ "," + alg->config_to_string()
										+ "," + ds->config_to_string()
										+ "," + crep::aging_config_to_string()
										+ "," + std::to_string(alg->get_time_total())
										+ "," + std::to_string(alg->get_com_cost())
										+ "," + std::to_string(alg->get_mig_cost());
									loggy.log("summary", log_entry);
									loggy.all_to_file();

									std::cerr << "[" << log_entry << "]" << std::endl
										<< " time=" << time_alg.count() << std::endl
										<< " com_cost=" << alg->get_com_cost() << std::endl
										<< " mig_cost=" << alg->get_mig_cost() << std::endl;

									ds->reset();
								}
							}
						}
					}
				}
			}
		}
	}

#if defined(__GNUC__)
	MPI_Finalize();
#endif
	return 0;
}

// Copyright (c) 2019 Martin Schonger