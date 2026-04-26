#pragma once

#include <cassert>
#include <random>

#include <benchmark/benchmark.h>

#include "pstl/utils/bench_input.h"
#include "pstl/utils/utils.h"
#include "pstl/utils/verification.h"

namespace benchmark_any_of
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
		std::uniform_int_distribution<size_t> gen(0, input.size() - 1);

		bool last_output = false;
		{
			pstl::bench_input bench{ input };
			for (auto _ : state)
			{
				const auto index = gen(engine);
				pstl::elem_t value;
				{
					auto && h = bench.host_view();
					value     = h[index];
				} // host_view dtor (NO_USM only) releases buffer back to device
				last_output = pstl::wrap_timing(state, std::forward<Function>(F), execution_policy, bench,
				                                [=](const auto & val) { return val == value; });
			}
		}

		auto verification_result = pstl::verify([&]() {
			const auto solution = true;
			return pstl::are_equivalent(last_output, solution);
		});

		state.SetBytesProcessed(pstl::computed_bytes(state, input));

		pstl::set_verification_counter(state, verification_result);
	}

} // namespace benchmark_any_of
