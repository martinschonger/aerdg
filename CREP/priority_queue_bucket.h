#pragma once

#include <utility>
#include <unordered_set>
#include <map>

namespace crep
{
	class priority_queue_bucket
	{
	public:
		priority_queue_bucket();
		~priority_queue_bucket() = default;

		void emplace(int32_t key, int32_t id)
		{
			auto bucket = hashmap.find(key);
			if (bucket == hashmap.end())
			{
				hashmap.emplace(key, std::unordered_set<int32_t>{id});
			}
			else
			{
				bucket->second.emplace(id);
			}
			++num_elements;
		}

		int32_t delete_min()
		{
			auto umap_iter = hashmap.begin();
			if (umap_iter == hashmap.end())
			{
				return -1;
			}

			auto bucket_iter = umap_iter->second.begin();
			if (bucket_iter == umap_iter->second.end())
			{
				return -1;
			}

			int32_t res = *bucket_iter;
			umap_iter->second.erase(res);
			if (umap_iter->second.empty())
			{
				hashmap.erase(umap_iter);
			}
			--num_elements;
			return res;
		}

		void decrease_key_or_emplace(int32_t id, int32_t old_key, int32_t new_key)
		{
			auto umap_iter = hashmap.begin();
			if (umap_iter != hashmap.end())
			{
				auto umap_iter = hashmap.find(old_key);

				if (umap_iter != hashmap.end())
				{
					std::unordered_set<int32_t>& bucket = umap_iter->second;
					num_elements -= bucket.erase(id);
					if (bucket.empty())
					{
						hashmap.erase(umap_iter);
					}
				}
			}

			emplace(new_key, id);
		}

		bool empty()
		{
			return num_elements == 0L;
		}

	private:
		std::map<uint64_t, std::unordered_set<int32_t>> hashmap;
		uint64_t num_elements;
	};

	priority_queue_bucket::priority_queue_bucket()
	{
		num_elements = 0L;
	}
}

// Copyright (c) 2019 Martin Schonger