#pragma once

#include <limits>
#include <type_traits>
#include <utility>

#include <ax.ct.hpp>

#define LOG_HEAD "[ct][math]: "

namespace ax { namespace ct { namespace math {

/// @returns array size
template <typename T, size_t N>
constexpr size_t array_size(T const (&)[N]) { return N; }

/// Number <=> type
template<typename NumT, NumT number>
struct basic_num_t {
    using number_type = NumT;
    enum : number_type { value = number };
};

/// Alias for size_t numbers
template <size_t I>
using num_t = basic_num_t<size_t, I>;


/// --- tuple_to_array --- ///

/// Constructs new type containing array of numbers from tuple of number classes
template <typename T>
struct tuple_to_array;

template <typename... Num>
struct tuple_to_array<std::tuple<Num...>> {
private:
    using num_list = std::tuple<Num...>;
    using NumT = typename std::tuple_element<0, std::tuple<Num...>>::type::number_type;
public:
    struct type { static constexpr const NumT values[] = { Num::value... }; };
};

template <typename... Num>
constexpr const typename tuple_to_array<std::tuple<Num...>>::NumT tuple_to_array<std::tuple<Num...>>::type::values[];

template <typename T>
using tuple_to_array_t = typename tuple_to_array<T>::type;


/// --- array_to_tuple --- ///

/// Constructs tuple of number classes from array (T must contain constexpr "values" array)
template <typename T>
struct array_to_tuple;

template <typename T, size_t Left>
struct array_to_tuple_impl;

template <typename T>
struct array_to_tuple_impl<T,0> {
    using type = std::tuple<>; };

template <typename T, size_t Left>
struct array_to_tuple_impl {
private:
    using NumT = typename std::decay<decltype(T::values[0])>::type;
public:
    using type = tuple_push_t<
        typename array_to_tuple_impl<T,Left-1>::type,
        basic_num_t<NumT, T::values[Left-1]>
    >;
};

template <typename T>
struct array_to_tuple {
    using type = typename array_to_tuple_impl<T,array_size(T::values)>::type; };

template <typename T>
using array_to_tuple_t = typename array_to_tuple<T>::type;


/// --- Math --- ///

template <typename T>
constexpr bool array_eq_impl(T const* p1, T const* p2, size_t N) {
    return  (N == 0) || ((*p1 == *p2) && array_eq_impl(p1 + 1, p2 + 1, N - 1)); }

/// Compares two arrays by element. Requires same type T for both arrays elements.
template <typename T, size_t N1, size_t N2>
constexpr bool array_eq(T const (&arr1)[N1], T const (&arr2)[N2]) {
    return (N1 == N2) && array_eq_impl(&arr1[0], &arr2[0], N1); }

constexpr size_t isqrt_impl(size_t n, size_t x0) {
    return  (n == 0) ? 0 :
            ((x0 + n/x0)/2 == x0 + 1 || (x0 + n/x0)/2 == x0) ? x0 :
            isqrt_impl(n, (x0 + n/x0)/2);
}

/// @returns compile-time integer square root (Newton algorithm), O(sqrt(n))
constexpr size_t isqrt(size_t n) {
    return isqrt_impl(n, n); }

/// @returns smallest divisor (value >= from), O(n). Assuming num >= 2.
constexpr size_t smallest_divisor(size_t num, size_t from = 2) {
    return  num % from == 0 ? from :
            from > num/2    ? num :
            num > 2         ? smallest_divisor(num, from + (from % 2 + 1)) :
                              throw;
}

/// @returns greatest divisor (value <= from), O(n)
constexpr size_t greatest_divisor(size_t num, size_t from = std::numeric_limits<size_t>::max()) {
    return  (from > num/2) ? greatest_divisor(num, num/2) :
            (num % from == 0) ? from : greatest_divisor(num, from - 1);
}

/// Greatest common divisor binary algorithm, ~O(log(digits))
constexpr size_t gcd(size_t u, size_t v) {
    return  (u == v) ? u :
            (u == 0) ? v :
            (v == 0) ? u :
            (~u & 0x1) ? (
                (v & 0x1) ? gcd(u >> 1, v) :
                            gcd(u >> 1, v >> 1) << 1
            ) :
            (~v & 0x1) ? gcd(u, v >> 1) :
            (u > v) ? gcd((u - v) >> 1, v) :
                        gcd((v - u) >> 1, u);
}

/// Pollard's rho-factorization, for now ~O(sqrt(n)), TODO: improve recursion
namespace rho {
    
    /// Linear congruential generator
    struct LCG {
        enum : size_t { m = 0xFFFFFFFFFFFFFFFF };
        enum : size_t { a = 6364136223846793005 };
        enum : size_t { c = 1442695040888963407 };
        
        constexpr static size_t random(size_t seed, size_t lower, size_t higher) {
            return lower + (seed * a + c) % (higher + 1 - lower); }
    };
    
    constexpr size_t abs_sub(size_t x, size_t y) {
        return (x < y) ? (y - x) : (x - y); }
    
    constexpr size_t gxn(size_t x, size_t n) {
        return (x*x + 1) % n; }
    
    template <size_t n, size_t x, size_t y, size_t d>
    struct calc_d;
    
    /// Specialisation for (possible) primes
    template <size_t n, size_t x, size_t y>
    struct calc_d<n, x, y, n> {
        enum : size_t { value = n }; };
    
    /// Cycle specialisation for iterating over d
    template <size_t n, size_t x, size_t y>
    struct calc_d<n, x, y, 1> {
        enum : size_t { new_x = gxn(x, n) };
        enum : size_t { new_y = gxn(gxn(y, n), n) };
        enum : size_t { value = calc_d<n, new_x, new_y, gcd(abs_sub(new_x, new_y), n)>::value };
    };
    
    /// Main template, result of factorization
    template <size_t n, size_t x, size_t y, size_t d>
    struct calc_d {
        enum : size_t { value = d }; };
    
    /// Finds random divisor of given n
    template <size_t n, size_t rounds = 3>
    struct divisor;
    
    /// Trivial 2
    template <size_t r>
    struct divisor<2, r> {
        enum : size_t { value = 2 }; };
    
    /// End or heuristic tests sequence
    template <size_t n>
    struct divisor<n, 1> {
        enum : size_t { value = calc_d<n, 2, 2, 1>::value }; };
    
    template <size_t n, size_t rounds>
    struct divisor {
    private:
        constexpr static bool   even  = (n % 2 == 0);
        constexpr static size_t prev  = even ? n : divisor<n, rounds-1>::value;
        constexpr static size_t xy    = even ? 2 : LCG::random(rounds, 2, n - 1);
    public:
        constexpr static size_t value = even ? 2 :
                                        (prev != n) ? prev : calc_d<n, xy, xy, 1>::value;
    };
    
    /// Finds random prime divisor of given n
    template <size_t n, size_t div = divisor<n>::value>
    struct prime_divisor;
    
    template <size_t n>
    struct prime_divisor<n,n> : std::integral_constant<size_t,n> {};
    
    template <size_t n, size_t div1>
    struct prime_divisor {
    private:
        constexpr static size_t div2 = n/div1;
        constexpr static size_t minval = div1 < div2 ? div1 : div2;
    public:
        constexpr static size_t value = prime_divisor<minval>::value;
    };
    
    /// Factorization using Pollard rho algorithm, ~O(sqrt(n))
    template <size_t...>
    struct prime_factors;
    
    template <size_t... primes>
    struct prime_factors<0, primes...> {
        static constexpr size_t values[] = { primes... }; };
    
    template <size_t val, size_t... primes>
    struct prime_factors<val, primes...> :
        prime_factors<
            prime_divisor<val>::value == val ? 0 : val/prime_divisor<val>::value,
            prime_divisor<val>::value,
            primes...
        > {};
    
    template <size_t... primes>
    constexpr size_t prime_factors<0, primes...>::values[];
}

/// Naive factorization, ~O(n^2)
template <size_t...>
struct prime_factors;

template <size_t... primes>
struct prime_factors<0, primes...> {
    static constexpr size_t values[] = { primes... }; };

template <size_t val, size_t... primes>
struct prime_factors<val, primes...> :
    prime_factors<
        smallest_divisor(val) == val ? 0 : val/smallest_divisor(val),
        smallest_divisor(val),
        primes...
    > {};

template <size_t... primes>
constexpr size_t prime_factors<0, primes...>::values[];

} // math
} // ct
} // ax

#undef LOG_HEAD
