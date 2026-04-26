#pragma once

#include <numeric>

#include <benchmark/benchmark.h>

#include "pstl/utils/bench_input.h"
#include "pstl/utils/utils.h"
#include "pstl/utils/verification.h"

namespace benchmark_adjacent_find
{
	template<class Policy, class Function>
	static void benchmark_wrapper(benchmark::State & state, Function && F)
	{
		constexpr auto execution_policy = Policy{};
		const auto & size = state.range(0);
		auto input = pstl::generate_increment(execution_policy, size);
		static auto rd = std::random_device{};

		// Choose a random number between 1 and size
		static std::minstd_rand               engine{ rd() };
		std::uniform_int_distribution<size_t> gen(0, input.size() - 2);

		// Capture the FINAL iteration's offset so verify can run after the
		// loop on host-fresh `input`. Iterators returned by oneDPL on a
		// sycl::buffer are not host-dereferenceable.
		std::ptrdiff_t last_offset = -1;
		std::size_t    last_index  = 0;
		{
			pstl::bench_input bench{ input };
			for (auto _ : state)
			{
				const auto index = gen(engine);
				last_index       = index;
				{
					auto && h = bench.host_view();
					// plant adjacent-equal pair at [index, index+1]
					h[index] = h[index + 1];
				}
				const auto out_iter =
				    pstl::wrap_timing(state, std::forward<Function>(F), execution_policy, bench);
				last_offset = (out_iter == bench.end())
				                  ? std::ptrdiff_t{ -1 }
				                  : std::distance(bench.begin(), out_iter);
				// reset for next iteration
				{
					auto && h = bench.host_view();
					h[index]  = h[index + 1] - pstl::elem_t{ 1 };
				}
			}
		}
		// The per-iter reset un-plants our pair; re-plant once for verify so
		// that input[last_offset] and input[last_offset+1] are equivalent.
		// (If the algo returned a naturally occurring pair at offset !=
		// last_index, re-planting at last_index doesn't disturb that pair.)
		if (last_offset >= 0) { input[last_index] = input[last_index + 1]; }

		auto verification_result = pstl::verify([&]() {
			// adjacent_find may return any valid adjacent-equal pair, not
			// necessarily the one we planted — float precision loss at large
			// sizes (>2^24) creates naturally occurring duplicates in
			// generate_increment output.
			if (last_offset < 0) { return false; }
			return pstl::are_equivalent(input[last_offset], input[last_offset + 1]);
		});
		state.SetBytesProcessed(pstl::computed_bytes(state, input));
		pstl::set_verification_counter(state, verification_result);
	}
} // namespace benchmark_adjacent_find
