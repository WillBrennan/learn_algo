#pragma once

#include <random>
#include <vector>

namespace learn {

template <class ValueT>
class ReservoirSampling {
  public:
    using Index = std::size_t;
    using Value = ValueT;

    explicit ReservoirSampling(Index num_to_sample, std::mt19937&& rand_gen = std::mt19937())
        : num_to_sample_(num_to_sample), rand_gen_(rand_gen) {
        samples_.reserve(num_to_sample);
    }

    Index numToSample() const { return num_to_sample_; }

    Index numProcessed() const { return num_processed_; }

    const std::vector<Value>& samples() const { return samples_; }

    bool process(const Value& value);

  private:
    std::mt19937 rand_gen_;

    Index num_to_sample_ = 0;
    Index num_processed_ = 0;
    std::vector<Value> samples_;
};

template <class ValueT>
bool ReservoirSampling<ValueT>::process(const Value& value) {
    const auto random_index = std::uniform_int_distribution<>(0, numProcessed())(rand_gen_);

    const auto sample_value = random_index < numToSample();

    if (sample_value) {
        if (numProcessed() >= numToSample()) {
            samples_.at(random_index) = value;
        } else {
            samples_.emplace_back(value);
        }
    }

    num_processed_ += 1;

    return sample_value;
}

}  // namespace learn