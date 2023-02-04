# flatlib

Flatten any C++ collection to only not flattenable values. That's it.

Library provides `flat_view` which is both a range and an iterator in flattened manner over any type of range provided by the user.

```cpp
std::list<std::vector<int>> numbers{ {1,2,3}, {4,5,6,7} };
for (int n : flatlib::flat(numbers)) {
	std::cout << n << '\n';
}
```

Requires C++20.

## Rules

Each rule is marked if it's implemented

- [x] `flat_view` is only a non-owning view: it only accepts lvalue references
- [ ] Flattened range with only one layer like `flat_view<std::vector<int>>` preserves iterator category
- [ ] Flattened at least bidirectional range is bidirectional
- [x] Flattened at least forward range is forward
- [x] Flattened at least input range is input
- [ ] Flattened collection of `std::array` is contigious
- [ ] Supports C++20 ranges semantics
- [ ] Supports C++17 iterator semantics

for definitions of input, forward, bidirectional and contigious from [C++ standard libary](https://en.cppreference.com/w/cpp/header/ranges#Range_concepts).
