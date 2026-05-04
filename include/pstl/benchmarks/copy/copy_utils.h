#pragma once

#include <numeric>

#include <benchmark/benchmark.h>

#include "pstl/utils/bench_input.h"
#include "pstl/utils/utils.h"
#include "pstl/utils/verification.h"

namespace benchmark_copy
{
	template<class Policy, class Function>
	static void benchmark_wrapper(benchmark::State & state, Function && F)
	{
		constexpr auto execution_policy = Policy{};

		const auto & size = state.range(0);

		auto input = pstl::generate_increment(execution_policy, size);

		auto output = pstl::get_vector<Policy>(size);

		{
			pstl::bench_input bench_in{ input };
			pstl::bench_input bench_out{ output };
			for (auto _ : state)
			{
				pstl::wrap_timing(state, std::forward<Function>(F), execution_policy, bench_in, bench_out);
			}
		}

		auto verification_result =
		    pstl::verify([&]() { return std::equal(input.begin(), input.end(), output.begin()); });

		state.SetBytesProcessed(pstl::computed_bytes(state, input, output));

		pstl::set_verification_counter(state, verification_result);
	}
} // namespace benchmark_copy
