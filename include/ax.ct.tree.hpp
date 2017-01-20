#pragma once

#include <type_traits>
#include <utility>

#include <ax.ct.hpp>

#define LOG_HEAD "[ct][tree]: "

namespace ax { namespace ct { namespace tree {

/// Alias for nullptr analogue
struct NIL{};

template<typename T, typename Left, typename Right, typename Parent = NIL>
struct node {
    using type   = T;       // node value
    using LT     = Left;    // left subtree
    using RT     = Right;   // right subtree
    using parent = Parent;  // back reference to parent node
};

template <typename T, typename P = NIL>
using leaf = node<T,NIL,NIL,P>;


/// --- walk --- ///

/// Constructs tuple of ordered tree elements ("inorder tree walk")
template <typename Node, typename Acc = std::tuple<>>
struct walk;

template <typename Acc>
struct walk<NIL,Acc> { using type = Acc; };

template <typename Node, typename Acc>
struct walk {
private:
    using accL = typename walk<typename Node::LT, Acc>::type;
    using accX = tuple_push_t<accL, typename Node::type>;
    using accR = typename walk<typename Node::RT, accX>::type;
public:
    using type = accR;
};

template <typename Node, typename Acc = std::tuple<>>
using walk_t = typename walk<Node, Acc>::type;


/// --- search --- ///

/**
 * Constructs "equal" comparator based on "less" one (lt_traits::lt):
 *      U == V   <=>   !(U < V) && !(V < U)
 * Can be used as base class (including CRTP case)
 */
template <typename lt_traits>
struct eq_traits {
    template <typename U, typename V>
    struct lt : std::integral_constant<bool, lt_traits::template lt<U,V>::value> {};
    
    template <typename U, typename V>
    struct eq : std::integral_constant<bool, (!lt<U,V>::value && !lt<V,U>::value)> {};
};

/// Contains "less" (lt) type comparator (+ "equal" due to CRTP inheritance)
struct sizeof_comp : public eq_traits<sizeof_comp> {
    template <typename U, typename V>
    struct lt : std::integral_constant<bool, (sizeof(U) < sizeof(V))> {};
};

/**
 * Finds node containing type equal to T in terms of comparators from CompTraits.
 * @arg Node    - root of tree
 * @arg T       - type to be found inside some node
 * @arg CompTraits - struct containing "lt" and "eq" type comparators (templates)
 */
template <typename Node, typename T, typename CompTraits>
struct search;

//template <typename Node, typename CompTraits> // must not find NIL
//struct search<Node,NIL,CompTraits> { using type = NIL; };

template <typename T, typename CompTraits>
struct search<NIL,T,CompTraits> { using type = NIL; };

template <typename Node, typename T, typename CompTraits>
struct search {
    using type = typename std::conditional<
        CompTraits::template eq<T, typename Node::type>::value,
        Node,
        typename std::conditional<
            CompTraits::template lt<T, typename Node::type>::value,
            typename search<typename Node::LT, T, CompTraits>::type,
            typename search<typename Node::RT, T, CompTraits>::type
        >::type
    >::type;
};

template <typename Node, typename T, typename CompTraits>
using search_t = typename search<Node, T, CompTraits>::type;


/// --- insert --- ///

template <typename Node, typename T, typename Comp>
struct insert;

template <typename T, typename Comp>
struct insert<NIL,T,Comp> { using type = leaf<T,NIL>; };

template <typename U, typename T, typename Comp>
struct insert<leaf<U>,T,Comp> {
    struct type;
    using child = leaf<T,type>;
    struct type : std::conditional<
        Comp::template lt<T, U>::value,
        node<T,child,NIL,NIL>,
        node<T,NIL,child,NIL>
    >::type {};
};

} // tree
} // ct
} // ax

#undef LOG_HEAD