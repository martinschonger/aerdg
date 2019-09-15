/*
Access offline copies of traces through this class.
*/

#pragma once

#include <string>

#include "data_source.h"
#include "utility.h"
#include <filesystem>

namespace crep
{
	class data_mongo_offline : public data_source
	{
	public:
		//Specifies the number of requests to read from the underlying trace at a time
		static constexpr int_ CREP_BUFFER_SIZE = 100000;

		data_mongo_offline(
			const std::string& database_name,
			const std::string& collection_name,
			const int_ max_num_samples,
			const int_ skip = 0) : data_source(max_num_samples, 1), database_name(database_name), collection_name(collection_name), max_num_samples(max_num_samples), skip(skip)
		{
			cur_request = 0;
			cur_buffer = std::floor((double)skip / (double)CREP_BUFFER_SIZE);
			left_in_buffer = 0;
			buffer.reserve(CREP_BUFFER_SIZE);
		}
		~data_mongo_offline() = default;

		bool has_next() override
		{
			return cur_request < max_num_samples;
		}

		const std::pair<id_, id_>& next() override
		{
			if (left_in_buffer == 0)
			{
				//(Re)fill the request buffer
				if (cur_buffer == std::floor((double)skip / (double)CREP_BUFFER_SIZE))
				{
					left_in_buffer = CREP_BUFFER_SIZE - (skip - cur_buffer * CREP_BUFFER_SIZE);
				}
				else
				{
					left_in_buffer = CREP_BUFFER_SIZE;
				}

				std::string config_filename(get_project_root() + "mdb_offline/" + database_name + "/" + collection_name + "/" + std::to_string(cur_buffer++) + ".txt");
				read_vector(config_filename, CREP_BUFFER_SIZE, buffer);
			}

			//Get next request from buffer
			auto& res = buffer[CREP_BUFFER_SIZE - left_in_buffer];
			--left_in_buffer;
			++cur_request;

			return res;
		}

		void reset() override
		{
			cur_request = 0;
			cur_buffer = std::floor((double)skip / (double)CREP_BUFFER_SIZE);
			left_in_buffer = 0;
			buffer.clear();
		}

		std::string config_to_string() const override
		{
			return database_name + "," + collection_name + ",\"" + "skip=" + std::to_string(skip) + "\"";
		}

	private:
		//This many requests have been read from the sequence so far
		int_ cur_request;

		//Length of the view of the request sequence repesented by this instance
		const int_ max_num_samples;

		//Skip this many requests from the beginning of the underlying trace
		const int_ skip;

		//Current part of the underlying trace
		int_ cur_buffer;
		
		//Number of requests left in current buffer
		int_ left_in_buffer;

		//Request buffer
		std::vector<std::pair<id_, id_>> buffer;
		
		std::string database_name, collection_name;
	};
}

// Copyright (c) 2019 Martin Schonger