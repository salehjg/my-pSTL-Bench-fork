#pragma once

#include <algorithm>

#include <benchmark/benchmark.h>

#include "pstl/utils/bench_input.h"
#include "pstl/utils/utils.h"
#include "pstl/utils/verification.h"

namespace benchmark_count
{
	template<class Policy, class Function>
	static void benchmark_wrapper(benchmark::State & state, Function && F)
	{
		constexpr auto execution_policy = Policy{};

		const auto & size = state.range(0);

		auto input = pstl::generate_increment(execution_policy, size);

		static auto rd = std::random_device{};

		static std::minstd_rand               engine{ rd() };
		std::uniform_int_distribution<size_t> gen(0, input.size() - 1);

		using count_t = decltype(std::count(input.begin(), input.end(), pstl::elem_t{}));
		count_t      last_output{};
		pstl::elem_t last_value{};
		{
			pstl::bench_input bench{ input };
			for (auto _ : state)
			{
				const auto value = static_cast<pstl::elem_t>(gen(engine));
				last_value       = value;
				last_output =
				    pstl::wrap_timing(state, std::forward<Function>(F), execution_policy, bench, value);
			}
		}

		auto verification_result = pstl::verify([&]() {
			const auto solution = std::count(input.begin(), input.end(), last_value);
			return pstl::are_equivalent(last_output, solution);
		});

		state.SetBytesProcessed(pstl::computed_bytes(state, input));

		pstl::set_verification_counter(state, verification_result);
	}
} // namespace benchmark_count
