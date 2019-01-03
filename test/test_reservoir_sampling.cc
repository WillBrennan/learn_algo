#include "learn/reservoir_sampling.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

TEST(ReservoirSampling, Construction) {
    using Sampler = learn::ReservoirSampling<int>;

    ASSERT_NO_THROW({ Sampler sampler(3); });
}

TEST(ReservoirSampling, numToSample) {
    using Sampler = learn::ReservoirSampling<int>;

    Sampler sampler(3);
    EXPECT_EQ(sampler.numToSample(), 3);
}

TEST(ReservoirSampling, numProcessed) {
    using Sampler = learn::ReservoirSampling<int>;

    Sampler sampler(3);

    const auto num_to_process = 10;

    for (int idx = 0; idx < num_to_process; ++idx) {
        EXPECT_EQ(sampler.numProcessed(), idx);
        sampler.process(idx);
    }

    EXPECT_EQ(sampler.numProcessed(), num_to_process);
}

TEST(ReservoirSampling, samplesInitialization) {
    using Sampler = learn::ReservoirSampling<int>;

    Sampler sampler(3);
    EXPECT_EQ(sampler.numProcessed(), 0);

    sampler.process(4);
    EXPECT_THAT(sampler.samples(), testing::ElementsAre(4));
    EXPECT_EQ(sampler.numProcessed(), 1);

    sampler.process(5);
    EXPECT_THAT(sampler.samples(), testing::ElementsAre(4, 5));
    EXPECT_EQ(sampler.numProcessed(), 2);

    sampler.process(12);
    EXPECT_THAT(sampler.samples(), testing::ElementsAre(4, 5, 12));
    EXPECT_EQ(sampler.numProcessed(), 3);
}

TEST(ReservoirSampling, samplesSize) {
    using Sampler = learn::ReservoirSampling<int>;

    const auto num_samples = 4;

    Sampler sampler(num_samples);
    EXPECT_EQ(sampler.numProcessed(), 0);

    sampler.process(3);
    sampler.process(4);
    sampler.process(5);
    sampler.process(8);
    EXPECT_EQ(sampler.numProcessed(), 4);

    const auto num_values = 100;

    for (auto idx = 0; idx < num_values; ++idx) {
        sampler.process(idx);
        EXPECT_THAT(sampler.samples(), testing::SizeIs(num_samples));
        EXPECT_EQ(sampler.numToSample(), num_samples);
    }
}

TEST(ReservoirSampling, StatisticalProperties) {
    using Sampler = learn::ReservoirSampling<int>;

    const auto num_samples = 4;
    const auto max_values = 10;
    const auto num_iter = 30000;

    auto value_counts = std::vector<int>(max_values, 0);

    for (auto iter_idx = 0; iter_idx < num_iter; ++iter_idx) {
        Sampler sampler(num_samples, std::mt19937(iter_idx));

        for (auto value_idx = 0; value_idx < max_values; ++value_idx) {
            sampler.process(value_idx);
        }

        for (const auto& sample : sampler.samples()) {
            value_counts[sample] += 1;
        }
    }

    const auto sample_fraction = double(num_samples) / double(max_values);
    const auto expected_value = num_iter * sample_fraction;

    const auto variance = std::sqrt(expected_value);

    const auto min_range = expected_value - 2 * variance;
    const auto max_range = expected_value + 2 * variance;

    EXPECT_THAT(value_counts,
                testing::Each(testing::AllOf(testing::Ge(min_range), testing::Le(max_range))));
}