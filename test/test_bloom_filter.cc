#include "learn/bloom_filter.h"

#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace helpers {
std::vector<int> values() { return {0, 3, 2, 5, 7, 6, 54, 383, 392, 49, 39, 590, 30, 4}; }
}  // namespace helpers

TEST(BloomFilter, Construction) {
    using BloomFilter = learn::BloomFilter<int>;

    ASSERT_NO_THROW({ BloomFilter filter(20, 2); });
}

TEST(BloomFilter, numBuckets) {
    using BloomFilter = learn::BloomFilter<int>;

    BloomFilter filter(32, 2);

    ASSERT_EQ(filter.numBuckets(), 32);
}

TEST(BloomFilter, numHashes) {
    using BloomFilter = learn::BloomFilter<int>;

    BloomFilter filter(32, 2);

    ASSERT_EQ(filter.numHashes(), 2);
}

TEST(BloomFilter, make) {
    using BloomFilter = learn::BloomFilter<int>;

    const auto filter = BloomFilter::make(300, 0.01);
    ASSERT_EQ(filter.numHashes(), 7);
    ASSERT_EQ(filter.numBuckets(), 3030);
}

TEST(BloomFilter, addAndPossiblyContains) {
    using BloomFilter = learn::BloomFilter<int>;

    BloomFilter filter(20, 2);

    for (const auto& value : helpers::values()) {
        filter.record(value);

        EXPECT_TRUE(filter.contains(value));
    }
}

TEST(BloomFilter, numBucketsPopulated) {
    using BloomFilter = learn::BloomFilter<int>;

    BloomFilter filter(20, 2);

    filter.record(230);
    ASSERT_EQ(filter.numBucketsPopulated(), 2);

    filter.record(230);
    ASSERT_EQ(filter.numBucketsPopulated(), 2);

    filter.record(233);
    ASSERT_GE(filter.numBucketsPopulated(), 2);
    ASSERT_LE(filter.numBucketsPopulated(), 4);
}

TEST(BloomFilter, EqualsToDifferentNumHashes) {
    using BloomFilter = learn::BloomFilter<int>;

    BloomFilter filterA(20, 2);
    BloomFilter filterB(20, 3);

    ASSERT_FALSE(filterA == filterB);
    ASSERT_FALSE(filterB == filterA);

    ASSERT_TRUE(filterA == filterA);
    ASSERT_TRUE(filterB == filterB);
}

TEST(BloomFilter, EqualsToDifferentNumBuckets) {
    using BloomFilter = learn::BloomFilter<int>;

    BloomFilter filterA(20, 2);
    BloomFilter filterB(21, 2);

    ASSERT_FALSE(filterA == filterB);
    ASSERT_FALSE(filterB == filterA);

    ASSERT_TRUE(filterA == filterA);
    ASSERT_TRUE(filterB == filterB);
}

TEST(BloomFilter, EqualsToBucketContents) {
    using BloomFilter = learn::BloomFilter<int>;

    BloomFilter filterA(20, 2);
    BloomFilter filterB(20, 2);

    ASSERT_TRUE(filterA == filterB);

    filterA.record(1);
    filterB.record(0);

    ASSERT_FALSE(filterA == filterB);

    filterA.record(0);
    filterB.record(1);

    ASSERT_TRUE(filterA == filterB);

    filterA.record(232);

    ASSERT_FALSE(filterA == filterB);
}

TEST(BloomFilter, filterUnion) {
    using BloomFilter = learn::BloomFilter<int>;

    BloomFilter filterA(100, 3);
    filterA.record(0);
    filterA.record(1);

    BloomFilter filterB(100, 3);
    filterB.record(2);

    const auto unionFilter = BloomFilter::filterUnion(filterA, filterB);
    ASSERT_EQ(unionFilter, BloomFilter::filterUnion(filterB, filterA));

    ASSERT_TRUE(unionFilter.contains(0));
    ASSERT_TRUE(unionFilter.contains(1));
    ASSERT_TRUE(unionFilter.contains(2));

    ASSERT_FALSE(unionFilter.contains(4));
    ASSERT_FALSE(unionFilter.contains(5));
    ASSERT_FALSE(unionFilter.contains(6));
    ASSERT_FALSE(unionFilter.contains(-1));
}

TEST(BloomFilter, filterIntersection) {
    using BloomFilter = learn::BloomFilter<int>;

    BloomFilter filterA(100, 3);
    filterA.record(0);
    filterA.record(1);

    BloomFilter filterB(100, 3);
    filterB.record(1);
    filterB.record(2);

    const auto unionFilter = BloomFilter::filterIntersection(filterA, filterB);
    ASSERT_EQ(unionFilter, BloomFilter::filterIntersection(filterB, filterA));

    ASSERT_TRUE(unionFilter.contains(1));

    ASSERT_FALSE(unionFilter.contains(0));
    ASSERT_FALSE(unionFilter.contains(2));

    ASSERT_FALSE(unionFilter.contains(4));
    ASSERT_FALSE(unionFilter.contains(5));
    ASSERT_FALSE(unionFilter.contains(6));
    ASSERT_FALSE(unionFilter.contains(-1));
}