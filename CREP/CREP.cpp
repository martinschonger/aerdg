// CREP.cpp : Defines the entry point for the application.

#include "CREP.h"

#define L 128//48
#define K 32//16
#define AUGMENTATION 2.1 //the competitive ratio for CREP holds with (2+epsilon)-augmentation, epsilon >= 1/K
#define ALPHA 2.5

//#define CREP_NOPT //disable optimization

enum RT_CONFIG
{
	MONGODB,
	JSON,
	DUMMY
};

int main(int argc, char* argv[])
{
#if defined(__GNUC__)
	int rank, nprocs;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
#endif

	//printf("Hello, world.  I am %d of %d\n", rank, nprocs); fflush(stdout);


	try
	{

		RT_CONFIG rt_cfg = RT_CONFIG::DUMMY;
		if (argc > 1)
		{
			std::string rt_cfg_str = argv[1];
			if (rt_cfg_str == "MONGODB")
			{
				rt_cfg = RT_CONFIG::MONGODB;
			}
			else if (rt_cfg_str == "JSON")
			{
				rt_cfg = RT_CONFIG::JSON;
			}
			else if (rt_cfg_str == "DUMMY")
			{
				rt_cfg = RT_CONFIG::DUMMY;
			}
			else
			{
				throw std::invalid_argument("Unrecognized runtime config value (argv[1]='" + rt_cfg_str + "')");
			}
		}

		std::unique_ptr<crep::data_source> ds;

		int32_t num_batches = 10;
		int32_t batch_size = 100;

		switch (rt_cfg)
		{
		case MONGODB: //argv[2] := num_batches, argv[3] := batch_size
			if (argc >= 4)
			{
				num_batches = std::stoi(argv[2]);
				batch_size = std::stoi(argv[3]);
			}
			ds = std::make_unique<crep::data_mongodb>("facebook", "clusterA", "timestamp", "srcrack", "dstrack", num_batches, batch_size);
			break;
		case JSON: //argv[2] := num_batches, argv[3] := batch_size, argv[4] := filename
			{
				std::string in_filename = "";
				if (BOOST_COMP_MSVC)
				{
					in_filename = "C:/Users/Martin Schonger/source/repos/CREP/data/base_result.json";
				}
				else if (BOOST_COMP_GNUC)
				{
					in_filename = "/mnt/c/Users/Martin Schonger/source/repos/CREP/data/base_result.json";
				}
				if (argc >= 4)
				{
					num_batches = std::stoi(argv[2]);
					batch_size = std::stoi(argv[3]);
				}
				if (argc >= 5)
				{
					in_filename = argv[4];
				}
				ds = std::make_unique<crep::data_json>(in_filename, num_batches, batch_size);
			}
			break;
		case DUMMY:
		default:
			std::vector<std::pair<crep::id_, crep::id_>> tmp_input
			{
				/*{ 0, 1 },
				{ 1, 2 },
				{ 3, 4 },
				{ 2, 0 },
				{ 0, 1 },
				{ 1, 2 },
				{ 3, 4 },
				{ 2, 0 },
				{ 3, 4 },
				{ 3, 4 },
				{ 3, 4 }*/
			/*{0,1},
			{2,3},
			{0,4},
			{4,1}*/
			{0,1},
			{0,2},
			{1,3},
			{2,3},
			{2,3}
			};
			ds = std::make_unique<crep::data_dummy>(std::move(tmp_input));
			break;
		}


		//static partitioning
		int64_t com_cost_stat = 0;
		double total_time_stat = 0;
#if defined(__GNUC__)
		//auto node_to_partition_stat = run_static_partition(ds, L, K, com_cost_stat, total_time_stat);
#elif defined(_MSC_VER)
		std::cerr << "[WARNING] static partitioning only works on linux so far" << std::endl;
#endif
		std::cout << "[static] communication cost:  " << com_cost_stat << std::endl;
		std::cout << "[static] total time:  " << total_time_stat << std::endl;
		ds->reset();


		//adaptive partitioning
		int64_t com_cost_ada = 0;
		double total_time_ada = 0;
		int32_t max_num_vertices = 5375;
		int32_t num_skip_requests = batch_size;
#if defined(__GNUC__)
		//auto node_to_partition_ada = run_adaptive_partition(ds, L, K, num_skip_requests, ALPHA, max_num_vertices, com_cost_ada, total_time_ada);
#elif defined(_MSC_VER)
		std::cerr << "[WARNING] adaptive partitioning only works on linux so far" << std::endl;
#endif
		std::cout << "[adaptive] communication cost: " << com_cost_ada << std::endl;
		std::cout << "[adaptive] total time:  " << total_time_ada << std::endl;
		ds->reset();


		//dynamic partitioning
		crep::log_options log_opts;
		
#ifdef CREP_DEBUG_LOG_TIME
		log_opts.time = true;
#endif // CREP_DEBUG_LOG_TIME

#ifdef CREP_DEBUG_LOG_VERTEX_EDGE_SPELLS
		log_opts.vertex_edge_spells = true;
#endif // CREP_DEBUG_LOG_VERTEX_EDGE_SPELLS

#ifdef CREP_DEBUG_LOG_CLUSTER_DYNAMICS
		log_opts.cluster_dynamics = true;
#endif // CREP_DEBUG_LOG_CLUSTER_DYNAMICS

#ifdef CREP_DEBUG_LOG_EVENTS
		log_opts.events = true;
#endif // CREP_DEBUG_LOG_EVENTS

		
		int64_t com_cost_dyn = 0;
		double total_time_dyn = 0.0;
		int32_t num_merges_dyn = 0.0;
		int32_t num_deletes_dyn = 0.0;
		
		run_dynamic_partition(ds, ALPHA, L, K, AUGMENTATION, num_batches, batch_size, log_opts, com_cost_dyn, total_time_dyn, num_merges_dyn, num_deletes_dyn, false);

		std::cout << "[dynamic] communication cost: " << com_cost_dyn << std::endl;
		std::cout << "[dynamic] total time: " << total_time_dyn << std::endl;


		//std::string cfg_filename = crep::log::logger_instance.tstamp + "__config" + ".txt";
		
		std::map<std::string, std::string> cfg{
			{"num_clusters",std::to_string(L)},
			{"cluster_size",std::to_string(K)},
			{"batch_size",std::to_string(batch_size)},
			{"num_batches",std::to_string(num_batches)},
			{"augmentation",std::to_string(AUGMENTATION)},
			{"alpha",std::to_string(ALPHA)},
			{"optimization","basic"}
		};

		std::string params = "";
		for (const auto& [key, val] : cfg)
		{
			//crep::append_to_file(crep::get_project_root() + "log/" + cfg_filename, key + "=" + val);
			params += val + ",";
		}
		params.pop_back();

		std::map<std::string, std::string> res{
			{"total_time_dyn",std::to_string(total_time_dyn)},
			{"num_merges_dyn",std::to_string(num_merges_dyn)},
			{"num_deletes_dyn",std::to_string(num_deletes_dyn)},
			{"com_cost_dyn",std::to_string(com_cost_dyn)},
			{"com_cost_stat",std::to_string(com_cost_stat)},
			{"total_time_stat",std::to_string(total_time_stat)}
		};

		std::string res_string = "";
		for (const auto& [key, val] : res)
		{
			res_string += val + ",";
		}
		res_string.pop_back();

		crep::log::logger_instance.add_identifier_existing_file("archive_log", crep::get_log_dir() + "archive_log" + ".csv");
		crep::log::logger_instance.log("archive_log", crep::log::logger_instance.tstamp, params, res_string);

#if defined(__GNUC__)
		MPI_Finalize();
#endif
		return 0;

	}
#if defined(__GNUC__)
	catch (const mongocxx::v_noabi::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
#endif
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
}

// Copyright (c) 2019 Martin Schonger