#include "create_offline_copy.h"

int main(int argc, char* argv[])
{
	crep::int_ num_batches2 = 170;
	crep::int_ batch_size2 = 100000;

	std::string database_name = "hpc";
	std::string collection_name = "exact_boxlib_multigrid_c_large";

	auto ds2 = std::make_unique<crep::data_mongodb>(database_name, collection_name, "number", "srcip", "dstip", num_batches2, batch_size2);
	///auto ds2 = std::make_unique<crep::data_generated>(1000, 128, 8, num_batches2 * batch_size2);

	for (crep::int_ i = 0; i < num_batches2; i++)
	{
		std::vector<std::pair<crep::id_, crep::id_>> vec(batch_size2);
		for (crep::int_ j = 0; j < batch_size2; j++)
		{
			vec[j] = ds2->next();
		}

		std::string config_filename = crep::get_project_root() + "mdb_offline/" + database_name + "/" + collection_name + "/";
		///std::string config_filename = crep::get_project_root() + "mdb_offline/gen/1000_128_8/";
		config_filename += std::to_string(i) + ".txt";
		std::cerr << "[INFO] writing '" << config_filename << "'." << std::endl;
		crep::write_vector(config_filename, vec);
	}

	std::cerr << "[INFO] done" << std::endl;

	return 0;
}

// Copyright (c) 2019 Martin Schonger