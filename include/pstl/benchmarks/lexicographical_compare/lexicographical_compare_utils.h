#pragma once

#include <numeric>

#include <benchmark/benchmark.h>

#include "pstl/utils/bench_input.h"
#include "pstl/utils/utils.h"
#include "pstl/utils/verification.h"

namespace benchmark_lexicographical_compare
{
	template<class Policy, class Function>
	static void benchmark_wrapper(benchmark::State & state, Function && F)
	{
		constexpr auto execution_policy = Policy{};

		const auto & size = state.range(0);

		auto data1 = pstl::generate_increment(execution_policy, size);
		auto data2 = pstl::generate_increment(execution_policy, size);

		bool last_output = false;
		{
			pstl::bench_input bench1{ data1 };
			pstl::bench_input bench2{ data2 };
			for (auto _ : state)
			{
				last_output =
				    pstl::wrap_timing(state, std::forward<Function>(F), execution_policy, bench1, bench2);
			}
		}

		auto verification_result = pstl::verify([&]() {
			const auto result =
			    std::lexicographical_compare(data1.begin(), data1.end(), data2.begin(), data2.end());
			return pstl::are_equivalent(result, last_output);
		});

		state.SetBytesProcessed(pstl::computed_bytes(state, data1, data2));

		pstl::set_verification_counter(state, verification_result);
	}
} // namespace benchmark_lexicographical_compare
