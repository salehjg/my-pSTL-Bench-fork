#pragma once

#include <benchmark/benchmark.h>

#include "pstl/utils/bench_input.h"
#include "pstl/utils/utils.h"
#include "pstl/utils/verification.h"

namespace benchmark_transform_reduce
{
	const auto transform_kernel = [](const auto & input) {
		return input * input;
	};

	template<class Policy, class Function>
	static void benchmark_wrapper(benchmark::State & state, Function && f)
	{
		constexpr auto execution_policy = Policy{};

		const auto & size = state.range(0);

		auto input = pstl::generate_increment(execution_policy, size);

		pstl::elem_t last_output{};
		{
			pstl::bench_input bench{ input };
			for (auto _ : state)
			{
				last_output =
				    pstl::wrap_timing(state, std::forward<Function>(f), execution_policy, bench, transform_kernel);
			}
		}

		auto verification_result = pstl::verify([&]() {
			const auto solution = std::transform_reduce(input.cbegin(), input.cend(), pstl::elem_t{}, std::plus<>(),
			                                            transform_kernel);
			return pstl::are_equivalent(solution, last_output);
		});

		state.SetBytesProcessed(pstl::computed_bytes(state, input));

		pstl::set_verification_counter(state, verification_result);
	}
} // namespace benchmark_transform_reduce
