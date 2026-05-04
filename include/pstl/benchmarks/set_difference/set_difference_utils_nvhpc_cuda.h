#pragma once

// Per-backend benchmark wrapper for NVHPC_CUDA (nvc++ -stdpar=gpu).
//
// Runs the per-iteration output-buffer reset on the GPU via
// std::fill(std::execution::par_unseq, ...) — nvc++'s stdpar offload picks
// it up. Replaces the serial host std::fill(...) used by the shared
// set_difference_utils.h. See docs/notes/cpu_time_vs_real_time.md.

#include <execution>
#include <numeric>
#include <optional>

#include <benchmark/benchmark.h>

#include "pstl/utils/bench_input.h"
#include "pstl/utils/utils.h"
#include "pstl/utils/verification.h"

namespace benchmark_set_difference
{
	template<class Policy, class Function>
	static void benchmark_wrapper(benchmark::State & state, Function && F)
	{
		constexpr auto execution_policy = Policy{};

		const auto & size = state.range(0);

		auto data1 = pstl::generate_increment(execution_policy, size);
		auto data2 = pstl::generate_increment(execution_policy, size / 2, static_cast<pstl::elem_t>(size / 4),
		                                      static_cast<pstl::elem_t>(1));

		const auto result_size = size - size / 2;
		auto output = pstl::make_buffer(static_cast<std::size_t>(size),
		                                std::numeric_limits<pstl::elem_t>::quiet_NaN());

		{
			pstl::bench_input bench1{ data1 };
			pstl::bench_input bench2{ data2 };
			pstl::bench_input bench_out{ output };
			for (auto _ : state)
			{
				pstl::wrap_timing(state, std::forward<Function>(F), execution_policy, bench1, bench2, bench_out);

				std::fill(std::execution::par_unseq, bench_out.begin(), bench_out.end(),
				          std::numeric_limits<pstl::elem_t>::quiet_NaN());
			}
		}

		auto verification_result = pstl::verify([&]() {
			std::fill(output.begin(), output.end(), std::numeric_limits<pstl::elem_t>::quiet_NaN());
			F(execution_policy, data1, data2, output);

			auto solution = std::vector<pstl::elem_t>(result_size, std::numeric_limits<pstl::elem_t>::quiet_NaN());
			std::set_difference(data1.begin(), data1.end(), data2.begin(), data2.end(), solution.begin());

			auto output_prefix =
			    std::vector<pstl::elem_t>(output.begin(), output.begin() + result_size);
			return pstl::are_equivalent(output_prefix, solution);
		});

		state.SetBytesProcessed(pstl::computed_bytes(state, data1, data2));
		pstl::set_verification_counter(state, verification_result);
	}
} // namespace benchmark_set_difference
