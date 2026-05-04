#pragma once

#include <numeric>

#include <benchmark/benchmark.h>

#include "pstl/utils/bench_input.h"
#include "pstl/utils/utils.h"
#include "pstl/utils/verification.h"

namespace benchmark_mismatch
{
	template<class Policy, class Function>
	static void benchmark_wrapper(benchmark::State & state, Function && F)
	{
		constexpr auto execution_policy = Policy{};

		const auto & size = state.range(0);

		auto data1 = pstl::generate_increment(execution_policy, size);

		static auto rd  = std::random_device{};
		static auto rng = std::mt19937{ rd() };
		auto        gen = std::uniform_int_distribution<int64_t>{ 0, size - 1 };

		auto data2 = data1;

		std::ptrdiff_t last_first_offset = 0;
		int64_t        last_idx          = 0;
		{
			pstl::bench_input bench1{ data1 };
			pstl::bench_input bench2{ data2 };
			for (auto _ : state)
			{
				const auto idx = gen(rng);
				last_idx       = idx;
				{
					auto && h2 = bench2.host_view();
					auto && h1 = bench1.host_view();
					h2[idx]    = h1[idx] + pstl::elem_t{ 1 };
				}

				const auto output =
				    pstl::wrap_timing(state, std::forward<Function>(F), execution_policy, bench1, bench2);
				last_first_offset = std::distance(bench1.begin(), output.first);

				// Undo the mismatch for the next iteration
				{
					auto && h2 = bench2.host_view();
					auto && h1 = bench1.host_view();
					h2[idx]    = h1[idx];
				}
			}
		}

		// Re-introduce the mismatch so verify can compare std::mismatch on
		// the host vectors against the offset captured from the algo.
		data2[last_idx] = data1[last_idx] + pstl::elem_t{ 1 };

		auto verification_result = pstl::verify([&]() {
			const auto solution = std::mismatch(data1.begin(), data1.end(), data2.begin());
			return (solution.first - data1.begin()) == last_first_offset;
		});

		state.SetBytesProcessed(pstl::computed_bytes(state, data1, data2));

		pstl::set_verification_counter(state, verification_result);
	}
} // namespace benchmark_mismatch
