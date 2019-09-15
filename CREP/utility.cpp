/*
Implementation of some functions declared in utility.h.
*/

#include "utility.h"

namespace crep
{
	tstamp_ util::CUR_TIME = 0;
	tstamp_ util::CUR_TIME_AGE = 0;
	
	//Begin: legacy code
	dur util::charikar_partial_time_detailed = dur(0);
	dur util::charikar_partial_time_detailed2 = dur(0);
	dur util::charikar_partial_time_detailed3 = dur(0);
	std::vector<std::vector<std::pair<int32_t, int32_t>>> util::edge_ontime = std::vector<std::vector<std::pair<int32_t, int32_t>>>();

	std::vector<log_cluster_assignment_record> util::cluster_assignment = std::vector<log_cluster_assignment_record>();
	std::vector<std::pair<int32_t, int32_t>> util::cluster = std::vector<std::pair<int32_t, int32_t>>();
	//End: legacy code

	void append_to_file(std::string filename, std::string new_content)
	{
		std::ofstream ost{ filename, std::ios_base::app };
		ost << new_content << std::endl;
		ost.close();
	}

	std::string get_timestamp()
	{
		auto t = std::time(nullptr);
		auto tm = *std::localtime(&t);

		std::ostringstream oss;
		oss << std::put_time(&tm, "%Y%m%d-%H%M%S");
		auto str = oss.str();

		return str;
	}

	std::string get_project_root()
	{
#if defined(__GNUC__)
		return "/mnt/c/Users/Martin Schonger/source/repos/CREP/";
#elif defined(_MSC_VER)
		return "C:/Users/Martin Schonger/source/repos/CREP/";
#endif
	}

	std::string get_log_dir()
	{
#if defined(__GNUC__)
		return "/mnt/c/Users/Martin Schonger/source/repos/crep_eval/log/";
#elif defined(_MSC_VER)
		return "C:/Users/Martin Schonger/source/repos/crep_eval/log/";
#endif
	}

	std::map<std::string, std::string> read_config(const std::string& filename)
	{
		std::map<std::string, std::string> res;

		std::ifstream is_file(filename);

		if (!is_file.is_open())
		{
			std::cerr << "[ERROR] Failed to open config file '" << filename << "'." << std::endl;
		}
		else
		{
			//Inspired by https://stackoverflow.com/a/6892829/2868795 (Author: sbi).
			//Note that for this snippet a license different from the one specified by the LICENSE file in the root directory might apply.
			//Begin: snippet
			std::string line;
			while (std::getline(is_file, line))
			{
				std::istringstream is_line(line);
				std::string key;
				if (std::getline(is_line, key, '='))
				{
					std::string value;
					if (std::getline(is_line, value))
					{
						res.emplace(key, trim(value));
					}
				}
			}
			//End: snippet
		}

		return res;
	}

	//Begin: legacy code
	namespace prob
	{
		double exp(const double lambda, const double& x)
		{
			return lambda * std::exp((-lambda) * x);
		}

		double exp_log(const double p, const double beta, const double& x)
		{
			return (1 / (-std::log(p)))
				* ((beta * (1 - p) * std::exp((-beta) * x))
					/ (1 - (1 - p) * std::exp((-beta) * x)));
		}

		double beta(const double alpha, const double beta, const double& x)
		{
			return (std::pow(x, alpha - 1.0) * std::pow(1.0 - x, beta - 1.0)) / std::beta(alpha, beta);
		}
	}
	//End: legacy code
}

// Copyright (c) 2019 Martin Schonger