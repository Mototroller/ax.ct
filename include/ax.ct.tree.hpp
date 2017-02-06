#pragma once

#include <string>
#include <sstream>
#include <type_traits>
#include <utility>

#include <ax.ct.hpp>

#define LOG_HEAD "[ct][tree]: "

namespace ax { namespace ct { namespace tree {

/// Alias for nullptr analogue
struct NIL{};

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

/// Contains "less" (lt) type comparator (+ "equal" due to CRTP inheritance) based on sizeof's
struct sizeof_comp : public eq_traits<sizeof_comp> {
    template <typename U, typename V>
    struct lt : std::integral_constant<bool, (sizeof(U) < sizeof(V))> {};
};


/// --- node, interface --- ///

/// Contains height of tree, O(n)
template <typename Node>
struct height;

/// Constructs tuple of ordered tree elements (inorder tree traversal), O(n)
template <typename Node>
struct walk;

/// Constructs tree string representation (for debuging), O(n)
template <typename Node, typename Printer, size_t Depth = 0, size_t From = 0>
struct print;

/**
 * Finds node containing type equal to T in terms of comparators from Comp, O(h)
 * @arg Node    - root of tree
 * @arg T       - type to be found inside some node
 * @arg Comp    - struct containing "lt" and "eq" type comparators (templates)
 * 
 * Contains:
 *      type    - found subtree
 *      tree    - back reference to Node
 */
template <typename Node, typename T, typename Comp = typename Node::comp>
struct search;

/// Pushes type to Tree, O(h)
template <typename Tree, typename T, typename Comp = typename Tree::comp>
struct insert;

/// Removes node with given T from tree, O(h)
template <typename Tree, typename T>
struct remove;

/**
 * Main binary tree entity.
 * Due to recursive binary tree definition, tree <=> node:
 *      node := NIL | [node,T,node]
 *              or
 *      f: [t[1], ..., t[k]]
 *      t: v f
 * Comparing operations (incapsulated to Comp) define exact binary search tree.
 */
template<typename T, typename Left, typename Right, typename Comp = sizeof_comp>
struct node {
    using type  = T;        // node value
    using LT    = Left;     // left subtree
    using RT    = Right;    // right subtree
    using comp  = Comp;     // types comparator (see "sizeof_comp" example above)
    
    //using parent = Parent;  // back reference to parent node TODO: difficult as shit!
    
    template <typename Tree = node>
    using height = std::integral_constant<size_t, height<Tree>::value>;
    
    template <typename Tree = node>
    using inorder_traversal = typename walk<Tree>::type;
    
    template <typename Printer>
    using stringify = print<node, Printer>;
    
    template <typename U>
    using find = typename search<node, U, comp>::type;
    
    template <typename U>
    using insert = typename insert<node, U, comp>::type;
    
    template <typename U>
    using erase = typename remove<node, U>::type;
};

template <typename T, typename C = sizeof_comp>
using leaf = node<T,NIL,NIL,C>;


/// --- equality and size, O(n) --- ///

/// Compares two trees by element
template <typename U, typename V>
struct tree_eq;

template <>
struct tree_eq<NIL,NIL> : std::true_type {};

template <typename U>
struct tree_eq<U,NIL> : std::false_type {};

template <typename V>
struct tree_eq<NIL,V> : std::false_type {};

template <typename U, typename... Rest1, typename... Rest2>
struct tree_eq<node<U,Rest1...>, node<U,Rest2...>> : std::integral_constant<bool,
    tree_eq<typename node<U,Rest1...>::LT, typename node<U,Rest2...>::LT>::value &&
    tree_eq<typename node<U,Rest1...>::RT, typename node<U,Rest2...>::RT>::value
> {};


template <>
struct height<NIL> : std::integral_constant<size_t, 0> {};

template <typename Tree>
struct height {
    static constexpr size_t value = 1 + max(
        height<typename Tree::LT>::value,
        height<typename Tree::RT>::value
    );
};


/// --- walk, O(n) --- ///

template <>
struct walk<NIL> { using type = std::tuple<>; };

template <typename Node>
struct walk {
private:
    using accL = typename walk<typename Node::LT>::type;
    using accX = tuple_push_t<accL, typename Node::type>;
    using accR = tuple_concat_t<accX, typename walk<typename Node::RT>::type>;
public:
    using type = accR;
};

template <typename Node>
using walk_t = typename walk<Node>::type;


/// Breadth-first traversal: constructs tuple of tree elements level-by-level
template <typename Node>
struct level_walk;

namespace level_walk_impl {
    
    template <typename Node, size_t level>
    struct collect_level;
    
    template <>
    struct collect_level<NIL,0> { using type = std::tuple<>; };
    
    template <typename Node>
    struct collect_level<Node,0> { using type = std::tuple<typename Node::type>; };
    
    template <typename Node, size_t level>
    struct collect_level {
        using accL = typename collect_level<typename Node::LT, level - 1>::type;
        using accR = typename collect_level<typename Node::RT, level - 1>::type;
        using type = tuple_concat_t<accL, accR>;
    };
    
    template <typename Node, size_t level>
    struct level_walk_impl;
    
    //template <size_t level> // seems like unnecessary
    //struct level_walk_impl<NIL,level> { using type = std::tuple<>; };
    
    template <typename Node>
    struct level_walk_impl<Node,0> { using type = std::tuple<>; };
    
    template <typename Node, size_t level>
    struct level_walk_impl {
        using prev = typename level_walk_impl<Node, level - 1>::type;
        using type = tuple_concat_t<prev, typename collect_level<Node, level-1>::type>;
    };
}

template <typename Node>
struct level_walk {
    using type = typename level_walk_impl::level_walk_impl<
        Node,
        height<Node>::value
    >::type;
};

template <typename Node>
using level_walk_t = typename level_walk<Node>::type;

/// Constructs tuple of types by level, O(nh)
template <typename Node, size_t level>
using collect_level_t = typename level_walk_impl::collect_level<Node,level>::type;


/// --- print, O(n) --- ///

struct value_printer {
    template <typename T>
    static std::string str() {
        std::ostringstream stream;
        stream << T::value;
        return stream.str();
    }
};

struct sizeof_printer {
    template <typename T>
    static std::string str() { return std::to_string(sizeof(T)); }
};

template <typename Printer, size_t Depth, size_t From>
struct print<NIL,Printer,Depth,From> { static std::string str() { return ""; } };

template <typename Node, typename Printer, size_t Depth, size_t From>
struct print {
private:
    enum : size_t { floor_length = 8 };
    
    static auto has_left(...) ->
    decltype(std::false_type());
    
    template <typename Bush>
    static auto has_left(Bush&&) ->
    decltype(std::declval<typename Bush::LT::type>(), std::true_type());
    
    static auto has_right(...) ->
    decltype(std::false_type());
    
    template <typename Bush>
    static auto has_right(Bush&&) ->
    decltype(std::declval<typename Bush::RT::type>(), std::true_type());
    
public:
    static std::string str() {
        auto lp = print<typename Node::LT, Printer, Depth + 1, 1>::str();
        auto rp = print<typename Node::RT, Printer, Depth + 1, 2>::str();
        bool end = (lp.length() + rp.length() == 0);
        
        char m[2][2] = {{' ','\\'},{'/','<'}};
        
        auto next = std::string(1, m
            [decltype(has_left (std::declval<Node>()))::value]
            [decltype(has_right(std::declval<Node>()))::value]
        );
        //auto next = path<typename Node::LT, typename Node::RT>();
        auto prev = std::string(From == 1 ? "/" : From == 2 ? "\\" : "");
        
        auto val = Printer::template str<typename Node::type>();
        (val = prev + "--{" + val + "}").resize(floor_length, end ? ' ' : '-');
        val += next;
        val = std::string((floor_length + 1) * Depth, ' ') + val;
        
        return lp + (lp.length() > 0 ? "\n" : "") + val + (rp.length() > 0 ? "\n" : "") + rp;
    }
};


/// --- search, O(h) --- ///

//template <typename Node, typename Comp> // must not find NIL
//struct search<Node,NIL,Comp> { using type = NIL; };

template <typename T, typename Comp>
struct search<NIL,T,Comp> { using type = NIL; using tree = NIL; };

template <typename Node, typename T, typename Comp>
struct search {
    using type = typename std::conditional<
        Comp::template eq<T, typename Node::type>::value,
        Node,
        typename search<typename std::conditional<
            Comp::template lt<T, typename Node::type>::value,
            typename Node::LT,
            typename Node::RT
        >::type, T, Comp>::type
    >::type;
    
    using tree = Node;
};

template <typename Node, typename T, typename Comp = typename Node::comp>
using search_t = typename search<Node, T, Comp>::type;


/// --- insert, O(h) --- ///

template <typename T, typename Comp>
struct insert<NIL,T,Comp> { using type = leaf<T,Comp>; };

template <typename Node, typename T, typename Comp>
struct insert {
private:
    enum : bool { is_left = Node::comp::template lt<T, typename Node::type>::value };
    
    using modified_subtree = typename insert<
        typename std::conditional<
            is_left,
            typename Node::LT,
            typename Node::RT
        >::type,
        T,
        Comp
    >::type;
    
public:
    using type = node<
        typename Node::type,
        typename std::conditional<
            is_left,
            modified_subtree,
            typename Node::LT
        >::type,
        typename std::conditional<
            is_left,
            typename Node::RT,
            modified_subtree
        >::type,
        Comp
    >;
};

template <typename Tree, typename T, typename Comp = typename Tree::comp>
using insert_t = typename insert<Tree,T,Comp>::type;

/// Inserts batch (tuple) of types to Tree
template <typename Tree, typename Tuple, typename Comp = typename Tree::comp>
struct insert_tuple {
private:
    template <typename Acc, typename T>
    struct inserter { using type = insert_t<Acc, T, Comp>; };
public:
    using type = tuple_reduce_t<Tuple, inserter, Tree>;
};

template <typename Tree, typename Tuple, typename Comp = typename Tree::comp>
using insert_tuple_t = typename insert_tuple<Tree,Tuple,Comp>::type;


/// --- delete --- ///

/// Contains parent node of given Subtree inside Tree, O(h)
template <typename Tree, typename Subtree, typename Prev = NIL>
struct parent_of;

template <typename Subtree, typename Prev>
struct parent_of<NIL,Subtree,Prev> { using type = NIL; };

template <typename Tree, typename Prev>
struct parent_of<Tree,Tree,Prev> { using type = Prev; };

template <typename Tree, typename Subtree, typename Prev>
struct parent_of {
    using type = typename parent_of<
        typename std::conditional<
            Tree::comp::template lt<typename Subtree::type, typename Tree::type>::value,
            typename Tree::LT,
            typename Tree::RT
        >::type,
        Subtree,
        Tree
    >::type;
};

template <typename Tree, typename Subtree, typename Prev = NIL>
using parent_of_t = typename parent_of<Tree,Subtree,Prev>::type;


/// Contains number of child nodes for given Tree, O(1)
template <typename Tree>
struct children_amount;

template <>
struct children_amount<NIL> : std::integral_constant<size_t, 0> {};

template <typename T, typename Comp>
struct children_amount<node<T,NIL,NIL,Comp>> : std::integral_constant<size_t, 0> {};

template <typename T, typename U, typename Comp>
struct children_amount<node<T,U,NIL,Comp>> : std::integral_constant<size_t, 1> {};

template <typename T, typename U, typename Comp>
struct children_amount<node<T,NIL,U,Comp>> : std::integral_constant<size_t, 1> {};

template <typename T, typename U, typename V, typename Comp>
struct children_amount<node<T,U,V,Comp>> : std::integral_constant<size_t, 2> {};


/// Contains node with minimal key, O(h)
template <typename Tree, typename Prev = NIL>
struct min_node;

template <typename Prev>
struct min_node<NIL,Prev> { using type = Prev; };

template <typename Tree, typename>
struct min_node { using type = typename min_node<typename Tree::LT, Tree>::type; };

template <typename Tree, typename Prev = NIL>
using min_node_t = typename min_node<Tree, Prev>::type;


template <typename T>
struct remove<NIL,T> { using type = NIL; };

template <typename Tree, typename T>
struct remove {
private:
    enum : bool { is_less   = Tree::comp::template lt<T, typename Tree::type>::value };
    enum : bool { is_equal  = Tree::comp::template eq<T, typename Tree::type>::value };
    
    enum : size_t { children = children_amount<Tree>::value };
    
    using key = typename min_node_t<
        typename std::conditional<
            is_equal && children == 2,
            typename Tree::RT,
            leaf<typename Tree::type, typename Tree::comp>
        >::type
    >::type;
    
    using recursive_call = typename remove<
        typename std::conditional<
            is_less,
            typename Tree::LT,
            typename std::conditional<
                !is_equal || children == 2,
                typename Tree::RT,
                NIL
            >::type
        >::type,
        typename std::conditional<
            is_equal,
            key,
            T
        >::type
    >::type;
    
    // TODO: unacceptable if c-tors are deleted, but more simple
    static auto root_dispatcher(...) ->
    typename Tree::RT;
    
    template <typename Bush>
    static auto root_dispatcher(Bush&&) ->
    decltype(typename Bush::LT::type(), typename Bush::LT());
    //*/
    
    /*/! Common solution, pretty overkill, but general
    static typename Tree::RT
    root_dispatcher(...);
    
    template <typename Bush>
    static typename std::enable_if<sizeof(typename Bush::LT::type), typename Bush::LT>::type
    root_dispatcher(Bush&&);
    //*/
    
public:
    using type = typename std::conditional<
        is_equal && (children < 2),
        decltype(root_dispatcher(std::declval<Tree>())),
        node<
            key,
            typename std::conditional<
                is_less,
                recursive_call,
                typename Tree::LT
            >::type,
            typename std::conditional<
                !is_less,
                recursive_call,
                typename Tree::RT
            >::type,
            typename Tree::comp
        >
    >::type;
};

template <typename Tree, typename T>
using remove_t = typename remove<Tree, T>::type;

/// Removes batch (tuple) of types from Tree
template <typename Tree, typename Tuple>
struct remove_tuple {
private:
    template <typename Acc, typename T>
    struct remover { using type = remove_t<Acc, T>; };
public:
    using type = tuple_reduce_t<Tuple, remover, Tree>;
};

template <typename Tree, typename Tuple>
using remove_tuple_t = typename remove_tuple<Tree,Tuple>::type;

} // tree
} // ct
} // ax

#undef LOG_HEAD
