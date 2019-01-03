# `ReservoirSampling`
`ReservoirSampling` is a space-efficient uniform-sampling method. It's an online algorithm for choosing *k* samples from a stream of *N* elements where *N* is unknown. Its also often used when *N* values cannot fit into memory. The samples produced by `ReservoirSampling` are valid after each element from *N* is processed. 

In contrast, a common method for selecting `k` random elements was to shuffle them at random using `std::shuffle` and selecting the first `k` elements. This method requires the number of elements to be known and in memory.

In C++17, `std::sample` was introduced which may be implemented with reservoir sampling. It uses an iterator-pair which hides the fact that its an online algorithm,

```cpp
int main() {
    std::string in = "abcdefgh", out;
    std::sample(in.begin(), in.end(), std::back_inserter(out), 5, std::mt19937{});
    std::cout << "five random letters out of " << in << " : " << out << '\n';
}
```

## Interesting Properties

- dont need to know the total number of elements
- small memory footprint - only needs to store the `k` samples and two integers
- 


## Sample
```cpp
int main() {
    using Sampler = learn::ReservoirSampling<int>;
    
    const auto num_samples = 4;
    auto sampler = Sampler(num_samples);

    for (int idx = 0; idx < 10; ++idx) {
        sampler.process(idx);
    }

    for (const auto& sample: sampler.samples()) {
        std::cout << sample << ' ';
    }
}
```

## How it works
`ReservoirSampling` only requires knowledge of the number of samples, `num_to_sample`, its to capture while processing a stream. We construct `ReservoirSampling` with `num_to_sample` and a random number generator, `rand_gen`. The sampler stores these values as well as reserving `samples_` to store the sampled values. This leads to the following decleration,

```cpp
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
```
. `ReservoirSampling` also stores `num_processed_` to count the number of times `process` has been called. `process` works by generating a uniform-random number, `random_index`, in the range of *0* to `numProcessed()`. We sample the value if `random_index` is less than `numToSample()`. This means, 

- `numProcessed() < numToSample()` - we will always keep the `value` until we have `numToSample()` samples
- `numProcessed() >= numToSample()` - we accept a `value` with a probability of `numToSample() / numProcessed()` and replace the sampled element at index `random_index`.

. When we replace an existing item, there's a `1 / numToSample()` probability that we will replace any given element. As such, the probability that we replace any sample element is `1 / numProcessed()`. This means that we sample `numToSample()` elements with a probability of `1 / numProcessed()`. 

```cpp
template <class ValueT>
bool ReservoirSampling<ValueT>::process(const Value& value) {
    const auto random_index = std::uniform_int_distribution<>(0, numProcessed())(rand_gen_);
    num_processed_ += 1;

    const auto sample_value = random_index < numToSample();

    if (sample_value) {
        if (samples_.size() >= numToSample()) {
            samples_.at(random_index) = value;
        } else {
            samples_.emplace_back(value);
        }
    }

    return sample_value;
}
```
