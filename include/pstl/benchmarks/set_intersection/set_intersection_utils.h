#pragma once

#include <numeric>
#include <optional>

#include <benchmark/benchmark.h>

#include "pstl/utils/bench_input.h"
#include "pstl/utils/utils.h"
#include "pstl/utils/verification.h"

namespace benchmark_set_intersection
{
	template<class Policy, class Function>
	static void benchmark_wrapper(benchmark::State & state, Function && F)
	{
		constexpr auto execution_policy = Policy{};

		const auto & size = state.range(0);

		auto data1 = pstl::generate_increment(execution_policy, size);
		auto data2 = pstl::generate_increment(execution_policy, size / 2, static_cast<pstl::elem_t>(size / 4),
		                                      static_cast<pstl::elem_t>(1));

		// `set_intersection` writes at most `min(|data1|, |data2|)` elements, but
		// oneDPL's GPU backend may create a SYCL buffer as large as the first input
		// range when wrapping the output iterator. A smaller allocation causes heap
		// corruption on CUDA, so we oversize `output` to `size` and only treat the
		// first `max_size` entries as meaningful.
		const auto max_size = std::min(data1.size(), data2.size());
		auto output = pstl::make_buffer(static_cast<std::size_t>(size),
		                                std::numeric_limits<pstl::elem_t>::quiet_NaN());

		{
			pstl::bench_input bench1{ data1 };
			pstl::bench_input bench2{ data2 };
			pstl::bench_input bench_out{ output };
			for (auto _ : state)
			{
				pstl::wrap_timing(state, std::forward<Function>(F), execution_policy, bench1, bench2, bench_out);

				{
					auto && h = bench_out.host_view();
					std::fill(std::begin(h), std::end(h), std::numeric_limits<pstl::elem_t>::quiet_NaN());
				}
			}
		}

		auto verification_result = pstl::verify([&]() {
			// The per-iter reset filled `output` with NaN, so re-run F once outside
			// the timed loop on the host-fresh inputs to populate output.
			std::fill(output.begin(), output.end(), std::numeric_limits<pstl::elem_t>::quiet_NaN());
			F(execution_policy, data1, data2, output);

			auto solution = std::vector<pstl::elem_t>(max_size, std::numeric_limits<pstl::elem_t>::quiet_NaN());
			std::set_intersection(data1.begin(), data1.end(), data2.begin(), data2.end(), solution.begin());

			// Compare only the meaningful prefix of `output`; the tail is NaN
			// padding required by the oversized allocation.
			auto output_prefix = std::vector<pstl::elem_t>(output.begin(), output.begin() + max_size);
			return pstl::are_equivalent(output_prefix, solution);
		});

		state.SetBytesProcessed(pstl::computed_bytes(state, data1, data2));
		pstl::set_verification_counter(state, verification_result);
	}
} // namespace benchmark_set_intersection