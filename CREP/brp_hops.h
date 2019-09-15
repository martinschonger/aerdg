#pragma once

#include <map>
#include <random>
#include <queue>

#include "brp_alg.h"
#include "graph_extended.h"
#include "cluster_state.h"

#include "debug.h"

#include <boost/math/distributions/exponential.hpp>
#include <boost/math/distributions/skew_normal.hpp>
#include <boost/math/distributions/beta.hpp>

namespace crep
{
	class brp_hops : public brp_alg
	{
	public:
		static dur time_explore;
		brp_hops(
			const std::string& uid,
			const real_& alpha, 
			const size_& num_vertices, 
			const size_& num_partitions, 
			const size_& partition_size,
			const real_& augmentation,
			std::unique_ptr<comp_mig_alg> cm_alg,
			const size_ num_hops)
			: brp_alg(uid, alpha, num_vertices, num_partitions, partition_size, augmentation, std::move(cm_alg)), num_hops(num_hops) {}
		
		~brp_hops() = default;
		
		void init() override
		{
			this->g = graph_extended(this->num_v);

			util::CUR_TIME = 0;

			std::random_device rd;
			this->gen = std::mt19937(rd());
			this->dis = std::uniform_real_distribution<>(0.0, 1.0);
		}

		const std::vector<id_>& get_current_partitioning()
		{
			return this->p_alg->get_vtx_to_part();
		}

		std::string type_to_string() const override
		{
			return "brp_hops";
		}

		std::string config_to_string() const override
		{
			return brp_alg::config_to_string() + ",'" + "num_hops=" + std::to_string(num_hops) + "'";
		}

	protected:
		void inc_edge_helper(const id_& src, const id_& dst) override
		{
			++util::CUR_TIME;

			g.inc_edge(src, dst);

			//explore num_hops-hop neighbors

			g.included[src] = util::CUR_TIME;
			g.included[dst] = util::CUR_TIME;

			std::vector<id_> incl_vtx{ src, dst };
			int_ num_vtxs = 2;
			int_ total_weight = 0;
			auto begin_explore = hrc::now();
			explore(src, dst, num_vtxs, total_weight, incl_vtx);
			time_explore += std::chrono::duration_cast<dur>(hrc::now() - begin_explore);

			int_ num_vtx_mig = incl_vtx.size();

			if (num_vtx_mig <= 2)
			{
				return;
			}

			real_ den = (real_)total_weight / ((real_)num_vtxs - 1.0);

			boost::math::skew_normal_distribution<> myskewnorm2(this->alpha, 1, 1);
			real_ mig_prob = std::max(0.005, boost::math::cdf(myskewnorm2, den));
			if (mig_prob < dis(gen))
			{
				return;
			}
			//DLOG(mig_prob);
			//DLOG(den);

			//perform migrations
			migrate(g, incl_vtx);

			serve_request(src, dst);
		}

	private:
		int_ num_hops;

		graph_extended g;

		std::mt19937 gen;
		std::uniform_real_distribution<> dis;

		void explore(const id_& src, const id_& dst, int_& num_vtxs, int_& total_weight, std::vector<id_>& incl_vtx)
		{
			std::queue<std::pair<id_, int_>> q;
			q.emplace(src, 0);
			q.emplace(dst, 0);

			while (!q.empty())
			{
				auto [cur, depth] = q.front();
				q.pop();

				if (g.visited[cur] != util::CUR_TIME)
				{
					g.visited[cur] = util::CUR_TIME;
					//++num_vtxs;
					
					for (const auto& e : g.get_adj(cur))
					{
						if (cur == src && e.target == dst)
						{
							total_weight += e.weight;
							continue;
						}

						if (depth < num_hops && g.visited[e.target] != util::CUR_TIME)
						{
							q.emplace(e.target, depth + 1);
							//total_weight += e.weight;
						}
						else if (depth == num_hops && g.visited[e.target] == util::CUR_TIME)
						{
							//total_weight += e.weight;
						}

						if ((depth < num_hops && g.visited[e.target] != util::CUR_TIME) 
							|| (depth == num_hops && g.visited[e.target] == util::CUR_TIME))
						{
							if (g.included[e.target] == util::CUR_TIME)
							{
								continue;
							}

							real_ lambda1 = 0.1,
								lambda2 = 3,
								lambda3 = 0.5,
								lambda4 = lambda2,
								lambda5 = lambda3;
							real_ factor1 = 0.3,
								factor2 = 0.4,
								factor3 = 0.3;

							boost::math::exponential_distribution<> myexp(lambda1);
							int_ max_lookback = 30;
							real_ p1 = std::max(0.01, boost::math::cdf(myexp, std::max(0.0, 20.0 - (util::CUR_TIME - e.updated))));

							//real_ p1 = 2.0 * prob::exp(lambda1, util::CUR_TIME - e.updated);
							//real_ p2 = std::min(1.0, (1.0 / prob::exp_log(lambda3, lambda2, 1.0 / alpha)) * prob::exp_log(lambda3, lambda2, 1.0 / (real_)e.weight));
							//real_ p2 = std::min(1.0, (1.0 / prob::beta(alpha, 10, 1.0 / alpha)) * prob::beta(alpha, 10, 1.0 / (real_)e.weight));
							//boost::math::beta_distribution<> mybeta(this->alpha, 2 * this->alpha);
							//real_ p2 = std::min(1.0,
							//	(1.0 / boost::math::cdf(mybeta, 1.0 - (1.0 / this->alpha))) * boost::math::cdf(mybeta, 1.0 - (1.0 / (real_)e.weight)));

							boost::math::skew_normal_distribution<> myskewnorm(this->alpha, 1, -2);
							real_ p2 = std::max(0.01, boost::math::cdf(myskewnorm, e.weight));
							//std::cerr << boost::math::cdf(myskewnorm, this->alpha) << std::endl;

							//real_ p3 = (1.0 / prob::exp_log(lambda5, lambda4, 0.0)) * prob::exp_log(lambda5, lambda4, 1.0 / (real_)(util::CUR_TIME - g.migrated[e.target]));

							//DLOG(p1);
							//DLOG(p2);
							//DLOG(p3);

							//real_ p_total = factor1 * p1 + factor2 * p2 + factor3 * p3;
							real_ p_total = p1 * p2;// *std::max(0.01, p3);
							if (p2 > 0)
							{
								//DLOG(e.weight);
								//DLOG(p2);
								//DLOG(p_total);
							}
							else
							{
								//DLOG(e.weight);
							}

							if (p_total >= dis(gen))
							{
								if (g.included[cur] != util::CUR_TIME)
								{
									g.included[cur] = util::CUR_TIME;
									incl_vtx.emplace_back(cur);
									++num_vtxs;
								}
								g.included[e.target] = util::CUR_TIME;
								incl_vtx.emplace_back(e.target);
								++num_vtxs;
								total_weight += e.weight;
							}
						}
					}
				}
			}
		}
	};

	dur brp_hops::time_explore;

}

// Copyright (c) 2019 Martin Schonger