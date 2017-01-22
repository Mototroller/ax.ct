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


/// Concatenates two tuple-like classes
template <class U, class V>
struct tuple_concat;

template <
    template <class...> class T,
    class... Alist,
    class... Blist
> struct tuple_concat<T<Alist...>, T<Blist...>> { using type = T<Alist..., Blist...>; };

template <typename U, typename V>
using tuple_concat_t = typename tuple_concat<U,V>::type;


/// Pushes type to tuple-like class
template <class Tuple, class T>
struct tuple_push;

template <
    template <class...> class Tuple,
    class... Args,
    class T
> struct tuple_push<Tuple<Args...>,T> { using type = Tuple<Args..., T>; };

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

template <typename Source, size_t I, template <class> class Mod, typename Acc>
struct tuple_transform<Source,I,I,Mod,Acc> { using type = Acc; };

template <typename Source, size_t A, size_t B, template <class> class Mod, typename Acc>
struct tuple_transform {
    using type = typename tuple_transform<
        Source, A + 1, B, Mod,
        tuple_push_t<
            Acc,
            typename Mod<typename std::tuple_element<A,Source>::type>::type
        >
    >::type;
};

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

template <template <class,class> class Fun, class Acc>
struct tuple_reduce<std::tuple<>,Fun,Acc> { using type = Acc; };

template <template <class,class> class Fun, class Acc, typename Head, typename... Tail>
struct tuple_reduce<std::tuple<Head,Tail...>,Fun,Acc> {
private:
    using Acc_ = typename Fun<Acc,Head>::type;
public:
    using type = typename tuple_reduce<std::tuple<Tail...>,Fun,Acc_>::type;
};

template <
    class Source,
    template <class,class> class Fun,
    class Acc = std::tuple<>
>
using tuple_reduce_t = typename tuple_reduce<Source,Fun,Acc>::type;


} // ct
} // ax

#undef LOG_HEAD
