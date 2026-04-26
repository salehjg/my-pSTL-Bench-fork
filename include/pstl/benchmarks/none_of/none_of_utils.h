#pragma once

#include <cassert>
#include <random>

#include <benchmark/benchmark.h>

#include "pstl/utils/bench_input.h"
#include "pstl/utils/utils.h"
#include "pstl/utils/verification.h"

namespace benchmark_none_of
{
	template<class Policy, class Function>
	static void benchmark_wrapper(benchmark::State & state, Function && F)
	{
		constexpr auto execution_policy = Policy{};

		const auto & size = state.range(0);

		auto input = pstl::generate_increment(execution_policy, size);

		const auto condition = [=](const auto & val) {
			return val > size;
		};

		bool last_output = false;
		{
			pstl::bench_input bench{ input };
			for (auto _ : state)
			{
				last_output =
				    pstl::wrap_timing(state, std::forward<Function>(F), execution_policy, bench, condition);
			}
		}

		auto verification_result = pstl::verify([&]() {
			const auto result = true;
			return pstl::are_equivalent(result, last_output);
		});

		state.SetBytesProcessed(pstl::computed_bytes(state, input));

		pstl::set_verification_counter(state, verification_result);
	}

} // namespace benchmark_none_of
