#pragma once

#include "data_source.h"
#include <string>
#include <map>
#include "include_mongodb.h"

namespace crep
{
	class data_mongodb : public data_source
	{
	public:
		data_mongodb(const std::string& database_name, const std::string& collection_name, const std::string& sort_by, const std::string& src_name, const std::string& dst_name, const int_& num_batches = 1, const int_ & batch_size = 1);
		~data_mongodb() = default;

		bool has_next() override;
		const std::pair<id_, id_>& next() override;
		void reset() override;
		std::string config_to_string() const override;
		std::pair<id_, id_> helper;
	private:
		std::string database_name;
		std::string collection_name;
		int_ cur_request, cur_batch;
		std::map<std::string, id_> mappy;
		std::string src_name, dst_name;
#if defined(__GNUC__)
		mongocxx::instance inst;
		mongocxx::client conn;
		std::unique_ptr<mongocxx::client_session> session;
		mongocxx::collection coll;
		mongocxx::options::find opts;
		std::unique_ptr<mongocxx::cursor> cursor;
#elif defined(_MSC_VER)
		
#endif
	};

	data_mongodb::data_mongodb(
		const std::string& database_name, 
		const std::string& collection_name, 
		const std::string& sort_by, 
		const std::string& src_name, 
		const std::string& dst_name, 
		const int_& num_batches, 
		const int_& batch_size) : data_source(num_batches, batch_size), database_name(database_name), collection_name(collection_name), src_name(src_name), dst_name(dst_name)
	{
		cur_request = 0;
		cur_batch = -1;
		helper = std::make_pair(0, 0);

#if defined(__GNUC__)
		conn = mongocxx::client(mongocxx::uri{"mongodb://[SENSITIVE INFORMATION HAS BEEN REMOVED]/" + database_name});
		session = std::make_unique< mongocxx::client_session>(conn.start_session());
		coll = conn[database_name][collection_name];

		opts = mongocxx::options::find();
		opts.sort(bsoncxx::builder::basic::make_document(bsoncxx::builder::basic::kvp(sort_by, 1)));
		opts.limit(batch_size);
#elif defined(_MSC_VER)
		std::cerr << "Currently not supported" << std::endl;
#endif
	}

	inline bool data_mongodb::has_next()
	{
#if defined(__GNUC__)
		return cur_request < num_batches * batch_size;
#elif defined(_MSC_VER)
		return false;
#endif
	}

	inline const std::pair<id_, id_>& data_mongodb::next()
	{
#if defined(__GNUC__)
		if (cur_request % batch_size == 0)
		{
			cur_batch++;
			opts.skip(cur_batch * batch_size);
			cursor = std::make_unique<mongocxx::cursor>(coll.find({}, opts));
		}
		else
		{
			++cursor->begin();
		}

		bsoncxx::document::view doc = *cursor->begin(); //when begin is called more than once, it returns the next result

		bsoncxx::document::element srcrack{ doc[src_name] };
		bsoncxx::document::element dstrack{ doc[dst_name] };

		/*std::string srcrack_str = (std::string)bsoncxx::stdx::string_view(srcrack.get_value().get_utf8());
		std::string dstrack_str = (std::string)bsoncxx::stdx::string_view(dstrack.get_value().get_utf8());

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
		}*/

		cur_request++;

		helper.first = srcrack.get_value().get_int32();
		helper.second = dstrack.get_value().get_int32();
#endif
		return helper;
	}

	inline void data_mongodb::reset()
	{
		cur_request = 0;
		cur_batch = -1;
		mappy.clear();
	}

	inline std::string data_mongodb::config_to_string() const
	{
		return database_name + "," + collection_name + ",";
	}
}

// Copyright (c) 2019 Martin Schonger