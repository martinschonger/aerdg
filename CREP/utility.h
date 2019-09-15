/*
Provides convenience functions for various tasks.
*/

#pragma once

#include <vector>
#include <algorithm>
#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <map>
#include <chrono>
#include <cmath>
#include <string_view>

#include "typedefs.h"

namespace crep
{
	//Essential for accurate time measurement
	typedef std::chrono::high_resolution_clock hrc;

	typedef std::chrono::duration<double> dur;

	struct log_cluster_assignment_record
	{
		log_cluster_assignment_record() : log_cluster_assignment_record(-1, -1, -1, -1, 0) {}
		log_cluster_assignment_record(
			const int32_t& onset, const int32_t& cur_cluster, const int32_t& cur_component_root, const int32_t& cur_component_size, const int32_t& is_component_root = 0)
			: onset(onset), cur_cluster(cur_cluster), cur_component_root(cur_component_root), cur_component_size(cur_component_size), is_component_root(is_component_root)
		{}

		int32_t onset;
		int32_t cur_cluster;
		int32_t cur_component_root;
		int32_t cur_component_size;
		int32_t is_component_root;
	};

	class util
	{
	public:
		util() = delete;

		//Global time and age-time clocks
		static tstamp_ CUR_TIME;
		static tstamp_ CUR_TIME_AGE;

		//Begin: legacy code
		typedef bool (*comp_custom)(const std::pair<int32_t, int32_t>&, const std::pair<int32_t, int32_t>&);
		static bool compare_custom(const std::pair<int32_t, int32_t>& a, const std::pair<int32_t, int32_t>& b)
		{
			return a.first > b.first;
		};

		static dur charikar_partial_time_detailed;
		static dur charikar_partial_time_detailed2;
		static dur charikar_partial_time_detailed3;

		static std::vector<std::vector<std::pair<int32_t, int32_t>>> edge_ontime; //pair: (onset, weight)
		static std::vector<log_cluster_assignment_record> cluster_assignment;
		static std::vector<std::pair<int32_t, int32_t>> cluster;
		//End: legacy code
	};

	//Print vector 'vec' to std::cerr
	template<typename T>
	void print_vector(const std::vector<T>& vec, const std::string& prefix = "", bool include_size = false)
	{
		if (!prefix.empty())
		{
			std::cerr << prefix << ": ";
		}
		if (include_size)
		{
			std::cerr << "(" << vec.size() << "): ";
		}
		std::for_each(vec.begin(), vec.end(),
			[](const auto& e) { std::cerr << e << " "; });
		std::cerr << std::endl;
	}

	//Append 'new_content' to the text/csv file specified by 'filename'
	extern void append_to_file(std::string filename, std::string new_content);

	extern std::string get_timestamp();

	//Get current timestamp in seconds
	inline int_ uid_base()
	{
		auto now = hrc::now();
		return now.time_since_epoch().count();
	}

	//Trim a string_view from front and back.
	//Taken and modified from https://stackoverflow.com/a/54364173/2868795 (Author: Phidelux).
	//Note that for this snippet a license different from the one specified by the LICENSE file in the root directory might apply.
	//Begin: snippet
	inline std::string_view trim(std::string_view s)
	{
		s.remove_prefix(std::min(s.find_first_not_of(" \t\r\v\n"), s.size()));
		s.remove_suffix((s.size() - 1) - std::min(s.find_last_not_of(" \t\r\v\n"), s.size() - 1));

		return s;
	}
	//End: snippet

	//Trim a string from front and back.
	//Note: may be replacable by the above routine.
	//Taken and modified from https://stackoverflow.com/a/25385766/2868795 (Author: Galik).
	//Note that for this snippet a license different from the one specified by the LICENSE file in the root directory might apply.
	//Begin: snippet
	inline const char* ws = " \t\n\r\f\v";

	//Trim from end of string (right)
	inline std::string& rtrim(std::string& s, const char* t = ws)
	{
		s.erase(s.find_last_not_of(t) + 1);
		return s;
	}

	//Trim from beginning of string (left)
	inline std::string& ltrim(std::string& s, const char* t = ws)
	{
		s.erase(0, s.find_first_not_of(t));
		return s;
	}

	//Trim from both ends of string (right then left)
	inline std::string& trim_alt(std::string& s, const char* t = ws)
	{
		return ltrim(rtrim(s, t), t);
	}
	//End: snippet

	extern std::string get_project_root();

	extern std::string get_log_dir();

	//Read a standard config file with lines of the form 'key=value'
	extern std::map<std::string, std::string> read_config(const std::string& filename);

	//Write vector to file, extremely fast: memory-mapped
	template<typename T>
	inline void write_vector(const std::string& filename, std::vector<T>& vec)
	{
		size_ total_size = vec.size() * sizeof(T);

		std::ofstream file(filename, std::ios::binary);
		if (!file.is_open())
		{
			std::cerr << "[ERROR] Could not open file '" << filename << "' for writing." << std::endl;
		}
		file.write(reinterpret_cast<const char*>(&vec[0]), total_size);
	}

	//Read vector from file, extremely fast: memory-mapped
	template<typename T>
	inline void read_vector(const std::string& filename, const size_& num_elements, std::vector<T>& vec)
	{
		size_ total_size = num_elements * sizeof(T);

		std::ifstream file(filename, std::ios::binary);
		if (!file.is_open())
		{
			std::cerr << "[ERROR] Could not open file '" << filename << "' for reading." << std::endl;
		}

		auto arr = new T[num_elements];

		file.read(reinterpret_cast<char*>(arr), total_size);
		vec.resize(num_elements);
		std::copy(arr, arr + num_elements, vec.begin());

		delete[] arr;
	}

	//Global modifiable aging parameters
	//The aging clock is incremented every 'time_warp' requests, corresponds to lambda
	inline tstamp_ time_warp = 1;

	//In the time of the aging clock, decaying values are multiplied by 'gamma' every time step
	inline real_ gamma = 1.0;

	//Experimental: 'theta' could be used to set a threshold for delete actions
	inline real_ theta = 0.0;

	inline std::string aging_config_to_string_header()
	{
		return "time_warp,gamma,theta";
	}

	inline std::string aging_config_to_string()
	{
		return std::to_string(time_warp) + "," + std::to_string(gamma) + "," + std::to_string(theta);
	}

	inline std::string aging_config_to_string_placeholder()
	{
		return ",,";
	}

	//Compute the aged value of 'obj'
	inline real_ get_aged_value(const areal_& obj)
	{
		return obj.val * std::pow(gamma, (double)(util::CUR_TIME_AGE - obj.modified));
	}

	//Compute the aged value of (val,modified)
	inline real_ get_aged_value(const real_& val, const tstamp_& modified)
	{
		return val * std::pow(gamma, (double)(util::CUR_TIME_AGE - modified));
	}

	//Modify an aged value 'obj' by 'delta'
	inline void modify_aged_value(areal_& obj, const real_& delta)
	{
		obj.val = get_aged_value(obj) + delta;
		obj.modified = util::CUR_TIME_AGE;
	}

	//Modify an aged value (val,modified) by 'delta'
	inline void modify_aged_value(real_& val, tstamp_& modified, const real_& delta)
	{
		val = get_aged_value(val, modified) + delta;
		modified = util::CUR_TIME_AGE;
	}

	//Read all lines of a text file into a vector of strings
	inline bool read_all_lines(const std::string& file_name, std::vector<std::string>& res)
	{
		std::ifstream in(file_name);

		if (!in.is_open())
		{
			std::cerr << "[ERROR] Could not open file '" << file_name << "' for reading." << std::endl;
			return false;
		}

		std::string str;
		while (std::getline(in, str))
		{
			if (str.size() > 0)
			{
				res.emplace_back(str);
			}
		}

		in.close();
		return true;
	}

	//Experimental: contains probability functions for use in randomized setting.
	//Note: barely tested, consider using boosts functions instead.
	namespace prob
	{
		extern double exp(const double lambda, const double& x);

		extern double exp_log(const double p, const double beta, const double& x);

		extern double beta(const double alpha, const double beta, const double& x);
	}
}

// Copyright (c) 2019 Martin Schonger