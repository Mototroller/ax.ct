#pragma once

#include <algorithm>
#include <string>
#include <sstream>
#include <type_traits>
#include <utility>

#define LOG_HEAD "[ct]: "

namespace ax { namespace ct {

using std::size_t;

template <typename T>
constexpr T min(T a, T b) { return a > b ? b : a; }

template <typename T>
constexpr T max(T a, T b) { return a < b ? b : a; }

/// Concatenates two tuples
template <typename U, typename V>
struct tuple_concat;

template <typename U, typename V>
using tuple_concat_t = typename tuple_concat<U,V>::type;

/// Pushes type to tuple
template <typename Tuple, typename T>
struct tuple_push;

template <typename Tuple, typename T>
using tuple_push_t = typename tuple_push<Tuple,T>::type;

/// Contains the same type of template parameter
template <typename T>
struct identity { using type = T; };

template <typename T>
using identity_t = typename identity<T>::type;

/**
 * Constructs new tuple by range [From,To) from Source, applies Mod to every element type.
 * @arg Source  - tuple to be transformed
 * @arg From,To - index range [From,To)
 * @arg Mod     - type modifier, identity (T->T) by default
 * @arg Acc     - accumulator tuple, empty (std::tuple<>) by default
 * 
 * Examples:
 *      t.slice(A,B)    == tuple_transform_t<t,A,B>
 *      t.map(Func)     == tuple_transform_t<t, 0, t.size, Func>
 */
template <
    typename Source, size_t From, size_t To,
    template <class> class Mod = identity,
    typename Acc = std::tuple<>
> struct tuple_transform;

template <
    typename Source, size_t From, size_t To,
    template <class> class Mod = identity,
    typename Acc = std::tuple<>
>
using tuple_transform_t = typename tuple_transform<Source,From,To,Mod,Acc>::type;

/**
 * Constructs new tuple from results of applying Fun(Acc,Type) to every Type From Source.
 * @arg Source  - tuple to be reduced
 * @arg Fun     - type modifier, instantiates as Fun<Acc,Type>, must contain "type" definition
 * @arg Acc     - accumulator type, empty tuple (std::tuple<>) by default
 */
template <
    class Source,
    template <class,class> class Fun,
    class Acc = std::tuple<>
> struct tuple_reduce;

template <
    class Source,
    template <class,class> class Fun,
    class Acc = std::tuple<>
>
using tuple_reduce_t = typename tuple_reduce<Source,Fun,Acc>::type;

namespace ctree {
    template <typename L, typename T, typename R>
    struct bintree {
        using node  = T;
        using left  = L;
        using right = R;
    };
    
    template <typename T>
    using leaf = bintree<void, T, void>;
    
    template <typename Tree>
    struct height;
    
    template <typename T>
    struct height<leaf<T>> : std::integral_constant<size_t, 1> {};
    
    template <typename L, typename T, typename R>
    struct height<bintree<L,T,R>> : std::integral_constant<size_t,
        1 + max(height<L>::value, height<R>::value)
    > {};
}

} // ct
} // ax

#include <ax.ct_impl.hpp>

#undef LOG_HEAD
