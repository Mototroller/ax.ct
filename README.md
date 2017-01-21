# ax.ct [![Build Status](https://travis-ci.org/Mototroller/ax.ct.svg?branch=master)](https://travis-ci.org/Mototroller/ax.ct)

Some compile-time powahful shit. Contains:

Namespace/Set | POWAAAH!
--- | ---
`ct` | Main ulility functions, tuple operations: `concat`, `transform`, `reduce`
`ct::ctstr` | Compile-time string algorithms: `strlen`, `find_substr`, `count_substr`; literal-like `string` class with interface: `at`, `to_string`, `substr`, `concat`, `eq`, `string_to_tuple`, `tuple_to_string`
`ct::math` | Compile-time arithmetics and algorithms: `array_eq`, `tuple_to_array`, `array_to_tuple`, `isqrt`, `smallest_divisor`, `greatest_divisor`, `gcd`; 2 implementations of prime factorization: naive (exponential complexity) and Pollard Rho algorithm (~sqrt complexity)
`ct::tree` | compile-time binary search tree: `tree_eq`, `height`, `walk` (inorder traversal), `level_walk` (breadth-first traversal), `print`, `search`, `insert`, `insert_tuple`, `parent_of`
`ct::cregex` | _Under construction_: compile-time regex-to-DFA engine

## Examples

`test.cpp` contains usage examples of mentioned functions. More complex applications will be introduced later.

## Building

Header-only library, has no dependencies: just use your favourite C++11 compiler!

> NOTE: clang-3.7 and below has a bug which prevents deducing brace-enclosed list (such as `{1UL,2UL,3UL}`) to array type (`T const(&)[N]` -> `unsigned long const(&)[3]`), so tests wont compile and should be rewritten with explicit test arrays definitions. But I'm too lazy to fix dis right now =)

## To inifinity and beyond!

Updates to be continued...
