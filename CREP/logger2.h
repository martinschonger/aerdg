/*
Lightweight logger that supports multiple co-existing logged quantities.
Data is buffered in string streams and periodically written to files.
*/

#pragma once

#include <string>
#include <map>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace crep
{
	namespace log
	{
		namespace fs = std::filesystem;

		class logger2
		{
		public:
			logger2(
				const fs::path& root_dir)
				: root_dir(root_dir)
			{
				counter = 0;
			}

			~logger2() = default;

			//Add a new quantity to log, subsequently identified by 'identifier'
			void add_identifier(const std::string& identifier, const fs::path& filename, const std::string& header)
			{
				auto [it, success] = mapper.emplace(identifier, counter++);
				counts.emplace_back(0);
				ofstreams.emplace_back(std::ofstream(root_dir / filename));
				sstreams.emplace_back(std::stringstream());
				sstreams[it->second] << header << std::endl;
			}

			//Append 'content' to the buffer specified by 'identifier'
			void log(const std::string& identifier, const std::string& content)
			{
				auto id = mapper[identifier];
				auto& outs = sstreams[id];
				outs << content << std::endl;

				if ((++counts[id]) >= 100000)
				{
					ofstreams[id] << outs.rdbuf();
					outs.str(std::string());
					counts[id] = 0;
				}
			}

			//Flush all string streams to file and clear them
			void all_to_file()
			{
				for (int_ i = 0; i < counter; i++)
				{
					ofstreams[i] << sstreams[i].rdbuf() << std::flush;
					sstreams[i].str(std::string());
				}
			}

		private:
			fs::path root_dir;
			int_ counter;

			//Relates identifiers to id's used for indexing the vectors of streams
			std::map<std::string, int_> mapper;

			//File streams for permanent storage
			std::vector<std::ofstream> ofstreams;

			//String streams for buffering
			std::vector<std::stringstream> sstreams;
			
			//Counters used for periodical flushing
			std::vector<int_> counts;
		};
	}
}

// Copyright (c) 2019 Martin Schonger