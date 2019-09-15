#pragma once

#include "data_source.h"

#include <string>
#include <map>
#include <fstream>
#include <sstream>

#include "include_boost.h"
#include "../lib/rapidjson/document.h"

namespace crep
{
	class data_json : public data_source 
	{
	public:
		data_json(const std::string& filename, const int_& num_batches = 1, const int_ & batch_size = 1);
		~data_json();

		virtual bool has_next() override;
		virtual const std::pair<id_, id_>& next() override;
		virtual void reset() override;
		std::string config_to_string() const override;
		std::pair<id_, id_> helper;
	private:
		int_ num_batches;
		std::map<std::string, id_> mappy;
		std::ifstream inFile;
		rapidjson::Document d;
		int_ cur_request;
	};

	data_json::data_json(const std::string& filename, const int_& num_batches, const int_& batch_size) : data_source(num_batches, batch_size)
	{
		cur_request = 0;
		helper = std::make_pair(0, 0);

		inFile.open(filename); //open the input file

		std::stringstream strStream;
		strStream << inFile.rdbuf(); //read the file
		std::string str = strStream.str();

		const char* json = str.c_str();
		d.Parse(json);

		if (d.Size() < num_batches * batch_size) std::cerr << "[WARNING] Number of available requests is less than specified." << std::endl;

		inFile.close();
	}

	data_json::~data_json()
	{
		if (inFile.is_open())
		{
			inFile.close();
		}
	}
	
	inline bool data_json::has_next()
	{
		return (cur_request < d.Size()) && (cur_request < num_batches * batch_size);
	}

	inline const std::pair<id_, id_>& data_json::next()
	{
		std::string srcrack_str = d[cur_request]["srcrack"].GetString();
		std::string dstrack_str = d[cur_request]["dstrack"].GetString();

		auto search_src = mappy.find(srcrack_str);
		id_ src = -1;
		if (search_src != mappy.end()) {
			src = search_src->second;
		}
		else {
			mappy.emplace(srcrack_str, mappy.size());
			src = mappy.size() - 1;
		}

		auto search_dst = mappy.find(dstrack_str);
		id_ dst = -1;
		if (search_dst != mappy.end()) {
			dst = search_dst->second;
		}
		else {
			mappy.emplace(dstrack_str, mappy.size());
			dst = mappy.size() - 1;
		}

		cur_request++;

		helper.first = src;
		helper.second = dst;

		return helper;
	}

	inline void data_json::reset()
	{
		cur_request = 0;
		mappy.clear();
	}

	inline std::string data_json::config_to_string() const
	{
		return "json,,";
	}
}

// Copyright (c) 2019 Martin Schonger