#pragma once

#include <cmath>

#include <algorithm>
#include <functional>
#include <tuple>
#include <vector>

namespace learn {

template <class ValueT, class HashT = std::hash<ValueT>>
class BloomFilter {
  public:
    using Index = std::size_t;
    using Scalar = double;
    using Value = ValueT;
    using Hash = HashT;

    static BloomFilter make(Index max_num, Scalar false_positive_error) {
        const auto ideal_num_hashes = std::log(1.0 / false_positive_error) / std::log(2);
        const auto num_hashes = Index(std::ceil(ideal_num_hashes));

        const auto ideal_num_buckets = max_num * (num_hashes / std::log(2));
        const auto num_buckets = Index(std::ceil(ideal_num_buckets));

        return BloomFilter(num_buckets, num_hashes);
    }

    BloomFilter(Index num_buckets, Index num_hashes)
        : buckets_(num_buckets, false), num_hashes_(num_hashes) {}

    void record(const Value& value);

    bool contains(const Value& value) const;

    Index numBuckets() const { return buckets_.size(); }
    Index numHashes() const { return num_hashes_; }
    Index numBucketsPopulated() const { return std::count(buckets_.begin(), buckets_.end(), true); }

    bool operator==(const BloomFilter& other) const;

    static BloomFilter filterUnion(const BloomFilter& lhs, const BloomFilter& rhs);
    static BloomFilter filterIntersection(const BloomFilter& lhs, const BloomFilter& rhs);

  private:
    Index num_hashes_;
    std::vector<bool> buckets_;

    auto hash(const Value& value) const;
};

template <class ValueT, class HashT>
void BloomFilter<ValueT, HashT>::record(const Value& value) {
    const auto [hash_a, hash_b] = hash(value);

    for (Index n = 0; n < num_hashes_; ++n) {
        const auto nth_hash = (hash_a + n * hash_b) % buckets_.size();
        buckets_[nth_hash] = true;
    }
}

template <class ValueT, class HashT>
bool BloomFilter<ValueT, HashT>::contains(const Value& value) const {
    const auto [hash_a, hash_b] = hash(value);

    for (Index n = 0; n < num_hashes_; ++n) {
        const auto nth_hash = (hash_a + n * hash_b) % buckets_.size();

        if (!buckets_[nth_hash]) {
            return false;
        }
    }

    return true;
}

template <class ValueT, class HashT>
auto BloomFilter<ValueT, HashT>::hash(const Value& value) const {
    const auto hash = Hash{}(value);
    const auto shift_by = sizeof(Index) / 2;

    Index hash_a = hash << shift_by;
    Index hash_b = hash >> shift_by;

    return std::make_tuple(hash_a, hash_b);
}

template <class ValueT, class HashT>
bool BloomFilter<ValueT, HashT>::operator==(const BloomFilter& other) const {
    return num_hashes_ == other.num_hashes_ && buckets_ == other.buckets_;
}

template <class Value, class Hash>
BloomFilter<Value, Hash> BloomFilter<Value, Hash>::filterUnion(const BloomFilter& lhs,
                                                               const BloomFilter& rhs) {
    if (lhs.numHashes() != rhs.numHashes()) {
        throw std::runtime_error("error! number of hashes dont match for union");
    }

    if (lhs.numBuckets() != rhs.numBuckets()) {
        throw std::runtime_error("error! number of buckets dont match for union");
    }

    BloomFilter result(lhs.numBuckets(), lhs.numHashes());

    std::transform(lhs.buckets_.begin(), lhs.buckets_.end(), rhs.buckets_.begin(),
                   result.buckets_.begin(),
                   [](const bool& lhs, const bool& rhs) -> bool { return lhs || rhs; });

    return result;
}

template <class Value, class Hash>
BloomFilter<Value, Hash> BloomFilter<Value, Hash>::filterIntersection(const BloomFilter& lhs,
                                                                      const BloomFilter& rhs) {
    if (lhs.numHashes() != rhs.numHashes()) {
        throw std::runtime_error("error! number of hashes dont match for union");
    }

    if (lhs.numBuckets() != rhs.numBuckets()) {
        throw std::runtime_error("error! number of buckets dont match for union");
    }

    BloomFilter result(lhs.numBuckets(), lhs.numHashes());

    std::transform(lhs.buckets_.begin(), lhs.buckets_.end(), rhs.buckets_.begin(),
                   result.buckets_.begin(),
                   [](const bool& lhs, const bool& rhs) -> bool { return lhs && rhs; });

    return result;
}

}  // namespace learn