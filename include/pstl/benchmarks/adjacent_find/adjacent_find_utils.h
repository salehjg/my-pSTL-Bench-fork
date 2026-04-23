#pragma once

#include <numeric>

#include <benchmark/benchmark.h>

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
		std::optional<bool> verification_result = std::nullopt;

        for (auto _ : state)
        {
            const auto index = gen(engine);
            // Make two adjacent elements equal to ensure that there is at least one match
            input[index] = input[index + 1];
            const auto output = pstl::wrap_timing(state, std::forward<Function>(F), execution_policy, input);
            if (not verification_result.has_value())
            {
                verification_result = pstl::verify([&]() {
                    // adjacent_find may return any valid adjacent-equal pair, not
                    // necessarily the one we planted — float precision loss at large
                    // sizes (>2^24) creates naturally occurring duplicates in
                    // generate_increment output.
                    return output != input.end()
                        && pstl::are_equivalent(*output, *(output + 1));
                });
            }
            // Reset the input for the next iteration
            input[index] = input[index + 1] - 1;
        }
        state.SetBytesProcessed(pstl::computed_bytes(state, input));
        pstl::set_verification_counter(state, verification_result);
	}
} // namespace benchmark_adjacent_find
