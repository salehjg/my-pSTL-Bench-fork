
#pragma once

#include <cmath>

#include <benchmark/benchmark.h>

#include "pstl/utils/bench_input.h"
#include "pstl/utils/utils.h"
#include "pstl/utils/verification.h"

namespace benchmark_transform
{
	const auto kernel = [](const auto & input) {
		return std::min(std::sin(input), std::tan(input));
	};

	template<class Policy, class Function>
	static void benchmark_wrapper(benchmark::State & state, Function && f)
	{
		constexpr auto execution_policy = Policy{};

		const auto & size = state.range(0);

		auto input  = pstl::generate_increment(execution_policy, size);
		auto output = input;

		{
			pstl::bench_input bench_in{ input };
			pstl::bench_input bench_out{ output };
			for (auto _ : state)
			{
				pstl::wrap_timing(state, std::forward<Function>(f), execution_policy, bench_in, bench_out, kernel);
			}
		}

		auto verification_result = pstl::verify([&]() {
			auto solution = pstl::generate_increment(execution_policy, size);
			std::transform(input.begin(), input.end(), solution.begin(), kernel);
			return pstl::are_equivalent(output, solution);
		});

		state.SetBytesProcessed(pstl::computed_bytes(state, input));

		pstl::set_verification_counter(state, verification_result);
	}
} // namespace benchmark_transform
