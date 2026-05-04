#pragma once

#include <numeric>

#include <benchmark/benchmark.h>

#include "pstl/utils/bench_input.h"
#include "pstl/utils/utils.h"
#include "pstl/utils/verification.h"

namespace benchmark_min_element
{
	template<class Policy, class Function>
	static void benchmark_wrapper(benchmark::State & state, Function && F)
	{
		constexpr auto execution_policy = Policy{};

		const auto & size = state.range(0);

		auto input = pstl::generate_increment(execution_policy, size);
		std::shuffle(input.begin(), input.end(), std::mt19937{ std::random_device{}() });

		// bench_input is constructed AFTER the one-time host shuffle so the
		// buffer is built over the already-shuffled host vector.
		std::ptrdiff_t last_offset = -1;
		{
			pstl::bench_input bench{ input };
			for (auto _ : state)
			{
				const auto out_iter =
				    pstl::wrap_timing(state, std::forward<Function>(F), execution_policy, bench);
				last_offset = (out_iter == bench.end())
				                  ? std::ptrdiff_t{ -1 }
				                  : std::distance(bench.begin(), out_iter);
			}
		}

		auto verification_result = pstl::verify([&]() {
			if (last_offset < 0) { return false; }
			const auto solution = std::min_element(input.begin(), input.end());
			return pstl::are_equivalent(*solution, input[last_offset]);
		});

		state.SetBytesProcessed(pstl::computed_bytes(state, input));

		pstl::set_verification_counter(state, verification_result);
	}
} // namespace benchmark_min_element
