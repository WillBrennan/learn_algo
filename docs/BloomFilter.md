# `BloomFilter`
`BloomFilter` is a probabilistic space-efficient data-structure to say whether an element is in a set. If an element is in the set, `BloomFilter` will always return true. If an element is not in the set it will almost always return false. It's a hash-based algorithm where the number of buckets and hashes used is specified to give a maximum false-positive rate up to a certain number of elements. This is assuming that the hashes are i.i.d. 

## Interesting Properties
- hash based - size is invariant of the object being recorded
- very small in comparison to other datastructures
- constant complexity for lookup and recording operations
- well-defined false positive rate
- false positive rate is a function of the number of buckets per an element and maximum number of elements
- space / false positive trade-offs can be explicitly made
- can perform union and intersection operations on filters directly

## Sample
```cpp
const auto max_num_values = 300;
const auto false_positive = 1e-2;

auto filter = BloomFilter<std::string>::make(max_num_values, false_positive);
filter.record("hello");
filter.record("world");

std::cout << std::boolalpha;
std::cout << "contains bonjour: " << filter.contains("bonjour") << '\n';
// contains bonjour - false
std::cout << "contains world: " << filter.contains("world") << '\n';
// contains world - true
```
## How it works
A `BloomFilter` consits of a bit-array with `num_buckets` elements, and a way of hashing a value `num_hashes` times. When we record a value with a `BloomFilter` we hash the value which each of the hashing functions, which map to `num_hashes` elements in the bit-array. We set these elements to true. 

To see if a `BloomFilter` contains a value, we hash it with the hashing functions. If any of the mapped values in the bit-array are false, then the filter does not contain the value. As the filter becomes more populated the chances of a false-positive increase as the bit-array elements have a chance of being set to true by other contained values. 

Using `num_hashes` different hashes on each operation would be very slow. Instead; we use a technique called [double-hashing](https://en.wikipedia.org/wiki/Double_hashing). This technique was suggested in [*Less Hashing, Same Performance: Building a Better Bloom Filter* by Adam Kirsch & Michael Mitzenmacher](http://www.eecs.harvard.edu/~michaelm/postscripts/rsa2008.pdf). 

### Construction
We can construct a `BloomFilter` by specifying the number of buckets and hashes directly,

```cpp
const auto num_buckets = 3030;
const auto num_hashes = 7;
auto filterA = BloomFilter<std::string>(num_buckets, num_hashes);
```
. We can see that this `BloomFilter` has a number of buckets and hashes. Within the filter we use `std::vector<bool>` to represent the bit-array, 

```cpp
template <class ValueT, class HashT = std::hash<ValueT>>
class BloomFilter {
  public:
    using Index = std::size_t;
    using Scalar = double;
    using Value = ValueT;
    using Hash = HashT;

    // static factory methods.....

    BloomFilter(Index num_buckets, Index num_hashes)
        : buckets_(num_buckets, false), num_hashes_(num_hashes) {}


    // operations on the filter...
    // binary operations on the filter... 

  private:
    Index num_hashes_;
    std::vector<bool> buckets_;

    // some private hashing functions....
};
```
. From this we can see that its size is invariant of the size of `ValueT` and the number being recorded. We provide an alternative constructor, `BloomFilter<T>::make`, so we can construct the filter based on the maximum number of values it may hold, and a desired false positive rate, 

```cpp
const auto max_num_values = 300;
const auto false_positive = 1e-2;

auto filterB = BloomFilter<std::string>::make(max_num_values, false_positive);
```
. The formulas defining this can be found on [Wikipedia](https://en.wikipedia.org/wiki/Bloom_filter#Probability_of_false_positives),

```cpp
static BloomFilter make(Index max_num, Scalar false_positive_error) {
    const auto ideal_num_hashes = std::log(1.0 / false_positive_error) / std::log(2);
    const auto num_hashes = Index(std::ceil(ideal_num_hashes));

    const auto ideal_num_buckets = max_num * (num_hashes / std::log(2));
    const auto num_buckets = Index(std::ceil(ideal_num_buckets));

    return BloomFilter(num_buckets, num_hashes);
}
```
.

### Recording Values
To record a value with the `BloomFilter`, we use the `record` method, 

```cpp
filterA.record("hello world");
filterB.record("bonjour");
```
. Internally, this is done using the double-hashing technique. Our hashing function, `hash(value)`, produces two hashes, `hashA` and `hashB`. Instead of computing `num_hashes` hashes, we create the `nth_hash` by using `hash_a` and `hash_b` as part of a [linear congruential generator (LCG)](https://en.wikipedia.org/wiki/Linear_congruential_generator). We use `nth_hash` to set bucket elements to true,

```cpp
template <class ValueT, class HashT>
void BloomFilter<ValueT, HashT>::record(const Value& value) {
    const auto[hash_a, hash_b] = hash(value);

    for (Index n = 0; n < num_hashes_; ++n) {
        const auto nth_hash = (hash_a + n * hash_b) % buckets_.size();
        buckets_[nth_hash] = true;
    }
}
```
. We compute the two hashes from a single hashing function `Hash`. As shown below, we hash `value` with `Hash` and then apply bitwise-shifts to produce two psuedo-independent hashes, 

```cpp
template <class ValueT, class HashT>
auto BloomFilter<ValueT, HashT>::hash(const Value& value) const {
    const auto hash = Hash{}(value);
    const auto shift_by = sizeof(Index) / 2;

    Index hash_a = hash << shift_by;
    Index hash_b = hash >> shift_by;

    return std::make_tuple(hash_a, hash_b);
}
```
. This method of producing two hashes from one is suitable for most applications. A similar technique is found within `std::linear_congruential_engine`, which uses bitwise-shifts to produce two psuedo-random numbers for its linear combination.

To check if a `value` is contained by the filter, we again apply double-hashing, but this time we check that all of buckets that `nth_hash` fall into are true. 

```cpp
template <class ValueT, class HashT>
bool BloomFilter<ValueT, HashT>::contains(const Value& value) const {
    const auto[hash_a, hash_b] = hash(value);

    for (Index n = 0; n < num_hashes_; ++n) {
        const auto nth_hash = (hash_a + n * hash_b) % buckets_.size();

        if (!buckets_[nth_hash]) {
            return false;
        }
    }

    return true;
}
```

### Binary Bloom Filter Operations
#### Comparison

As a `BloomFilter` is a type of hash-table, they can be compared to each other, 

```cpp
std::cout << "filterA == filterB: " << (filterA == filterB) << '\n';
// filterA == filterB: false

filterA.record("bonjour");
filterB.record("hello world");

std::cout << "filterA == filterB: " << (filterA == filterB) << '\n';
// filterA == filterB: true
```
. If they contain the same number of hashes and buckets, and the buckets are equal, then they almost always contain the same values.

```cpp
template <class ValueT, class HashT>
bool BloomFilter<ValueT, HashT>::operator==(const BloomFilter& other) const {
    return num_hashes_ == other.num_hashes_ && buckets_ == other.buckets_;
}
```

#### Union and Intersection
One interesting property of `BloomFilter`s is that you can perform union and intersection operations on them, 

```cpp
int main() {
    using BloomFilter = learn::BloomFilter<int>;

    BloomFilter filterA(100, 3);
    filterA.record(0);
    filterA.record(1);

    BloomFilter filterB(100, 3);
    filterB.record(2);

    const auto filterUnion = BloomFilter::filterUnion(filterA, filterB);
    const auto filterIntersection = BloomFilter::filterIntersection(filterA, filterB);
}
```
. These operations require that both filters have the same number of buckets and filters. The union operation is the simplest, if an element in either of `filterA` or `filterB` is true, then the union is true, 

```cpp
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
```
. `BloomFilter`s have this property as its invariant of the order that values are recorded, 

```cpp
int main() {
    using BloomFilter = learn::BloomFilter<std::string>;

    auto filterA = BloomFilter::make(10, 3);
    auto filterB = BloomFilter::make(10, 3);

    std::cout << std::boolalpha;

    std::cout << "filterA == filterB: " << (filterA == filterB) << '\n';
    // filterA == filterB: true

    filterA.record("hello world");
    filterB.record("bonjour");
    
    std::cout << "filterA == filterB: " << (filterA == filterB) << '\n';
    // filterA == filterB: false

    filterA.record("bonjour");
    filterB.record("hello world");

    std::cout << "filterA == filterB: " << (filterA == filterB) << '\n';
    // filterA == filterB: true
}

```
. As we can define a filter-union, we can also define a filter-intersection. We do this by replacing a or-comparison with a and-comparison when we create our new buckets, (`lhs || rhs` vs `lhs && rhs`), 

```cpp
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
```