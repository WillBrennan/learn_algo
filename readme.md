# Learn Algo
## Learning about interesting algorithms through implementation
This library was inspired by [Fantastic Algorithms and Where to Find Them (CppCon 2017)](https://youtu.be/YA-nB2wjVcI), where a set of interesting algorithms where presented. This library contains data-structures and functions which have interesting properties such as being highly space efficient, 

Each of the components have documentation in [/docs](https://github.com/WillBrennan/learn_algo/tree/master/docs) explaining how they work, what idioms the use, and whats interesting about them.

Hopefully you'll find this an interesting read!

Feel free to submit a PR adding more components or improving library / documentation!

### [`BloomFilter`](https://github.com/WillBrennan/learn_algo/tree/master/docs/BloomFilter.md)
`BloomFilter` is a probabilistic space-efficient data-structure to say whether an element is in a set, where false-positives are possible but false-negatives are not. A `BloomFilter` with an optimal number of hashes only requires 20-bits per an element to achieve a false positive rate of 0.01%. A naive set of tweets would take 160 bytes per a tweet, a `BloomFilter` would take 2.4 bytes, being 66x smaller. Its also has a comparable speed to a hash-map.