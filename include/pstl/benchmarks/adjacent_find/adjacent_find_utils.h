#pragma once

#include <numeric>

#include <benchmark/benchmark.h>

#include "pstl/utils/utils.h"

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

		for (auto _ : state)
		{
			const auto index = gen(engine);
			input[index]     = input[index + 1];

			const auto output = pstl::wrap_timing(state, std::forward<Function>(F), execution_policy, input);

			// Verify a valid adjacent pair was found.  We don't require it to be
			// the exact pair we planted because float precision loss at large sizes
			// (>2^24) creates naturally occurring duplicates in generate_increment
			// output, and adjacent_find may return any of them.
			assert(output != input.end() && *output == *(output + 1));

			input[index] = input[index + 1] - 1;
		}

		state.SetBytesProcessed(pstl::computed_bytes(state, input));
	}
} // namespace benchmark_adjacent_find
