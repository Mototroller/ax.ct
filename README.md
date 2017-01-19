# ax.ct [![Build Status](https://travis-ci.org/Mototroller/ax.ct.svg?branch=master)](https://travis-ci.org/Mototroller/ax.ct)

Some compile-time powahful shit. Contains:

Namespace/Set | POWAAAH!
--- | ---
`ct` | Main ulility functions, tuple operations: `concat`, `transform`, `reduce`
`ct::ctstr` | Compile-time string algorithms: `strlen`, `find_substr`, `count_substr`; literal-like `string` class with interface: `at`, `to_string`, `substr`, `concat`, `eq`, `string_to_tuple`, `tuple_to_string`
`ct::math` | Compile-time arithmetics and algorithms: `array_eq`, `isqrt`, `smallest_divisor`, `greatest_divisor`, `gcd`; 2 implementations of prime factorization: naive (exponential complexity) and Pollard Rho algorithm (~sqrt complexity)
`ct::ctree` | _Under construction_: compile-time AVL-tree engine
`ct::cregex` | _Under construction_: compile-time regex-to-DFA engine

## Building

Header-only library, has no dependencies: just use your favourite C++11 compiler =3

## To inifinity and beyond!

Updates to be continued...
