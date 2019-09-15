#pragma once

#include <string>
#include <utility>
#include <map>

#include "utility.h"

#define CREP_DEBUG_LOG_TIME 1
//#define CREP_DEBUG_LOG_VERTEX_EDGE_SPELLS 1
//#define CREP_DEBUG_LOG_CLUSTER_DYNAMICS 1
//#define CREP_DEBUG_LOG_EVENTS 1

namespace crep
{
	namespace log
	{
		class logger
		{
		public:
			logger()
			{
				tstamp = get_timestamp();
			}

			void add_identifier_existing_file(const std::string& identifier, const std::string& filename)
			{
				identifier_to_filename.emplace(identifier, filename);
			}

			void add_identifier(const std::string& identifier, const std::string& filename, const std::string& header)
			{
				set_or_update_identifier(identifier, filename);
				append_to_file(filename, header);
			}

			template<typename... Args>
			void log(const std::string& identifier, const Args& ... args) const
			{
				std::ostringstream stream;
				using List= int[];
				(void)List {
					0, ((void)(stream << args << ","), 0) ...
				};

				std::string result = stream.str();
				result.pop_back();
				append_to_file(identifier_to_filename.at(identifier), result);
			}

			std::string tstamp;

			void reset_timestamp() //probably not thread safe
			{
				tstamp = get_timestamp();
			}

			void set_or_update_identifier(const std::string& identifier, const std::string& filename)
			{
				identifier_to_filename.erase(identifier);
				identifier_to_filename.emplace(identifier, filename);
			}

		private:
			const std::string filename;
			std::map<std::string, std::string> identifier_to_filename;
		};

		extern logger logger_instance;

		template<typename... Args>
		void log(const std::string& identifier, const Args& ... args)
		{
			logger_instance.log(identifier, args...);
		}
	}
}

// Copyright (c) 2019 Martin Schonger