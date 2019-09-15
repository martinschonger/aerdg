/*
Framework for any data source, i.e. for representing/interfacing a request sequence.
*/

#pragma once

#include <utility>
#include <vector>

#include "typedefs.h"

namespace crep
{
	//Abstract class for any data source
	class data_source
	{
	public:
		virtual ~data_source() = default;

		//Returns true if there are more requests available
		virtual bool has_next() = 0;

		//Get the next request, undefined behavior if none exists
		virtual const std::pair<id_, id_>& next() = 0;

		//Reset the request sequence to its initial state
		virtual void reset() = 0;

		//Get config of data source
		virtual std::string config_to_string() const = 0;

		int_ get_num_batches() const
		{
			return num_batches;
		}

		int_ get_batch_size() const
		{
			return batch_size;
		}

	protected:
		data_source(int_ num_batches = 0, int_ batch_size = 0) : num_batches(num_batches), batch_size(batch_size) {}
		int_ num_batches, batch_size;
	};


	//Data source for hard-coded request sequences (e.g. at dev time)
	class data_dummy : public data_source
	{
	public:
		data_dummy(std::vector<std::pair<id_, id_>>&& input = { {0,1} });
		bool has_next() override;
		const std::pair<id_, id_>& next() override;
		void reset() override;
		std::string config_to_string() const override;

	private:
		int_ cur_request;
		std::vector<std::pair<id_, id_>> input;
	};

	data_dummy::data_dummy(std::vector<std::pair<id_, id_>>&& input) : data_source(1, input.size()), input(input)
	{
		cur_request = 0;
	}

	inline bool data_dummy::has_next()
	{
		return cur_request < input.size();
	}

	inline const std::pair<id_, id_>& data_dummy::next()
	{
		return input[cur_request++];
	}

	inline void data_dummy::reset()
	{
		cur_request = 0;
	}

	inline std::string data_dummy::config_to_string() const
	{
		return "debug,,";
	}
}

// Copyright (c) 2019 Martin Schonger