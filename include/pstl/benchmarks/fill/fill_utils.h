#pragma once

#include <algorithm>

#include <benchmark/benchmark.h>

#include "pstl/utils/bench_input.h"
#include "pstl/utils/utils.h"
#include "pstl/utils/verification.h"

namespace benchmark_fill
{
	template<class Policy, class Function>
	static void benchmark_wrapper(benchmark::State & state, Function && F)
	{
		constexpr auto execution_policy = Policy{};

		const auto & size = state.range(0);

		auto input = pstl::generate_increment(execution_policy, size);

		{
			pstl::bench_input bench{ input };
			for (auto _ : state)
			{
				pstl::wrap_timing(state, std::forward<Function>(F), execution_policy, bench, pstl::elem_t{});
			}
		}

		auto verification_passed = pstl::verify([&]() {
			return std::all_of(input.begin(), input.end(),
			                   [](const auto & val) { return pstl::are_equivalent(val, pstl::elem_t{}); });
		});

		state.SetBytesProcessed(pstl::computed_bytes(state, input));

		pstl::set_verification_counter(state, verification_passed);
	}
} // namespace benchmark_fill
