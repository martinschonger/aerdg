#pragma once

#include <vector>
#include <random>
#include <numeric>

#include "data_source.h"
#include "utility.h"

namespace crep
{
	class data_generated : public data_source 
	{
	public:
		data_generated(
			const int_ epoch_length, 
			const int_ num_v, 
			const int_ p_size, 
			const int_ max_num_requests = -1) : 
			data_source(max_num_requests, 1), epoch_length(epoch_length), num_v(num_v), p_size(p_size), max_num_requests(max_num_requests)
		{
			cur_request = 0;

			srcs.resize(p_size - 1, -1);
			dsts.resize(p_size - 1, -1);

			gen = std::mt19937(rd());
			choose_pair = std::uniform_int_distribution<std::mt19937::result_type>(0, p_size - 2);
		}
		~data_generated() = default;

		bool has_next() override
		{
			return (max_num_requests == -1) || (cur_request < max_num_requests);
		}

		const std::pair<id_, id_>& next() override
		{
			if (cur_request % epoch_length == 0)
			{
				reshuffle();
			}

			auto idx = choose_pair(gen);

			helper.first = srcs[idx];
			helper.second = dsts[idx];

			++cur_request;

			return helper;
		}

		void reset() override
		{
			cur_request = 0;
		}

		std::string config_to_string() const override
		{
			return "generated,,\"epoch_length=" + std::to_string(epoch_length) + ",p_size=" + std::to_string(p_size) + "\"";
		}

		std::pair<id_, id_> helper;

	private:
		int_ cur_request;
		const int_ epoch_length;
		const int_ num_v;
		const int_ p_size;
		const int_ max_num_requests;

		std::vector<id_> srcs;
		std::vector<id_> dsts;

		std::random_device rd;
		std::mt19937 gen;
		std::uniform_int_distribution<std::mt19937::result_type> choose_pair;

		void reshuffle()
		{
			std::vector<id_> out(num_v, -1);
			std::iota(out.begin(), out.end(), 0);
			std::shuffle(out.begin(), out.end(), gen);
			std::vector<id_> in{ out[0] };
			out.erase(out.begin());

			for (int_ i = 0; i < p_size - 1; i++)
			{
				std::uniform_int_distribution<std::mt19937::result_type> choose_out(0, out.size() - 1);
				std::uniform_int_distribution<std::mt19937::result_type> choose_in(0, in.size() - 1);

				auto out_idx = choose_out(gen);
				auto in_idx = choose_in(gen);

				srcs[i] = out[out_idx];
				dsts[i] = in[in_idx];

				in.emplace_back(out[out_idx]);
				out.erase(out.begin() + out_idx);
			}
		}

	};
}

// Copyright (c) 2019 Martin Schonger