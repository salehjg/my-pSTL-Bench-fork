#pragma once

#include <numeric>

#include <benchmark/benchmark.h>

#include "pstl/utils/bench_input.h"
#include "pstl/utils/utils.h"
#include "pstl/utils/verification.h"

namespace benchmark_search
{
	template<class Policy, class Function>
	static void benchmark_wrapper(benchmark::State & state, Function && F)
	{
		constexpr auto execution_policy = Policy{};

		const auto & size = state.range(0);

		auto data1 = pstl::generate_increment(execution_policy, size);
		auto data2 = pstl::generate_increment(execution_policy, size / 2, static_cast<pstl::elem_t>(size / 4),
		                                      static_cast<pstl::elem_t>(1));

		std::ptrdiff_t last_offset = -1;
		{
			pstl::bench_input bench1{ data1 };
			pstl::bench_input bench2{ data2 };
			for (auto _ : state)
			{
				const auto out_iter =
				    pstl::wrap_timing(state, std::forward<Function>(F), execution_policy, bench1, bench2);
				last_offset = (out_iter == bench1.end())
				                  ? std::ptrdiff_t{ -1 }
				                  : std::distance(bench1.begin(), out_iter);
			}
		}

		auto verification_result = pstl::verify([&]() {
			const auto solution = std::search(data1.begin(), data1.end(), data2.begin(), data2.end());
			if (solution == data1.end()) { return last_offset < 0; }
			if (last_offset < 0) { return false; }
			return pstl::are_equivalent(*solution, data1[last_offset]);
		});

		state.SetBytesProcessed(pstl::computed_bytes(state, data1, data2));

		pstl::set_verification_counter(state, verification_result);
	}
} // namespace benchmark_search
