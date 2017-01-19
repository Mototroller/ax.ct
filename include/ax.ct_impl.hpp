#include <ax.ct.hpp>

namespace ax { namespace ct {

/// --- tuple_concat --- ///

template <typename... L1, typename... L2>
struct tuple_concat<std::tuple<L1...>, std::tuple<L2...>> {
    using type = std::tuple<L1..., L2...>; };


/// --- tuple_push --- ///

template <typename T, typename... U>
struct tuple_push<std::tuple<U...>,T> {
    using type = std::tuple<U..., T>; };

/// --- tuple_transform --- ///

/// End of recursion
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


/// --- tuple_reduce --- ///

/// End of recursion
template <template <class,class> class Fun, class Acc>
struct tuple_reduce<std::tuple<>,Fun,Acc> { using type = Acc; };

template <template <class,class> class Fun, class Acc, typename Head, typename... Tail>
struct tuple_reduce<std::tuple<Head,Tail...>,Fun,Acc> {
private:
    using Acc_ = typename Fun<Acc,Head>::type;
public:
    using type = typename tuple_reduce<std::tuple<Tail...>,Fun,Acc_>::type;
};

} // ct
} // ax
