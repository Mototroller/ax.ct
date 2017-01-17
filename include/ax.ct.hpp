#pragma once

#include <algorithm>
#include <string>
#include <sstream>
#include <type_traits>
#include <utility>

#define LOG_HEAD "[ct]: "

namespace ax { namespace ct {

template <typename T>
constexpr T min(T a, T b) {
    return a > b ? b : a; }

template <typename T>
constexpr T max(T a, T b) {
    return a < b ? b : a; }

template <typename U, typename V>
struct tuple_concat;

template <typename... L1, typename... L2>
struct tuple_concat<std::tuple<L1...>, std::tuple<L2...>> {
    using type = std::tuple<L1..., L2...>; };

template <typename U, typename V>
using tuple_concat_t = typename tuple_concat<U,V>::type;

/// Contains some compile-time string algorithms
namespace ctstr {
    
    /// Explicitly defined constexpr strlen()
    template <typename Char>
    constexpr size_t strlen(Char const* str, size_t acc = 0) {
        return str[acc] == Char{'\0'} ? acc : strlen(str, acc + 1); }
    
    template <typename Char>
    constexpr size_t count_substr_impl(Char const* c, Char const* s, size_t number, size_t offset) {
        return 
        (*c == Char{'\0'}) ? ( // end of string
            number + (s[offset] == Char{'\0'}) // last match
        ) : (
            (s[offset] == Char{'\0'}) ? ( // match completed
                count_substr_impl(c, s, number + 1, 0) // reset, continue
            ) : (
                (*c == s[offset]) ? (
                    count_substr_impl(c + 1, s, number, offset + 1) // symbol, continue
                ) : (
                    count_substr_impl(c + (offset == 0), s, number, 0) // fail, continue/reset
                )
            )
        );
    }
    
    /// @returns the number of occurrences of substring
    template <typename Char>
    constexpr size_t count_substr(Char const* c, Char const* s) {
        return count_substr_impl(c, s, 0, 0); }
    
    
    /// --- Literals --- ///
    
    /// Character <=> type
    template <typename CharT, CharT C>
    struct basic_char_t { enum : CharT { value = C }; };
    
    template <char C>
    using char_t = basic_char_t<char, C>;
    
    /// Example of prepared to use string literal
    struct literal_example_ {
        /// Symbols type
        using CharT = std::decay<decltype("example"[0])>::type;
        
        /// Literal
        static constexpr const CharT* const str() { return "example"; }
    };
    
    /// Wraps given string literal to structure
    #define DEFINE_LITERAL(name, string) \
        struct name { \
            using CharT = std::decay<decltype(string[0])>::type; \
            static constexpr const CharT* const str() { return string; } \
        }
    
    /// Wrapper, allows compile-time "string" (literal) manipulation and analysis
    template <typename T, size_t Alpha = 0, size_t Omega = strlen(T::str())>
    struct string {
        static_assert(Alpha <= Omega, LOG_HEAD "invalid bounds");
        
        /// Provides base type wrapping full literal
        using base = string<T, 0, strlen(T::str())>;
        
        enum : size_t { alpha = Alpha }; // first character index
        enum : size_t { omega = Omega }; // last character index (not inclusive)
        
        enum : size_t { length = omega - alpha };
        enum : bool   { is_base = std::is_same<base, string>::value };
        
        /// Provides "substring" type
        template <size_t begin, size_t end>
        using substr_t = string<T, alpha + begin, alpha + end>;
        
        /// @returns basic string literal (pointer)
        constexpr static char const* const source() {
            return T::str(); }
        
        /// @returns character by index
        constexpr static char const at(size_t idx) {
            return source()[alpha + idx]; }
        
        template <size_t idx>
        struct char_at : std::integral_constant<char, at(idx)> {
            static_assert(idx <= length, LOG_HEAD "at() index beyond the bounds"); };
        
        static std::string to_string() {
            return std::string(&source()[alpha], length); }
    };
    
    
    
    
    
    /// Creates tuple of symbol classes from given string
    template <typename>
    struct literal_to_tuple;
    
    template <typename T, size_t I>
    struct literal_to_tuple<string<T,I,I>> {
        using type = std::tuple<char_t<'\0'>>; };
    
    template <typename T, size_t A, size_t O>
    struct literal_to_tuple<string<T,A,O>> {
        using LW = string<T,A,O>;
        using type = tuple_concat_t<
            std::tuple< char_t<LW::at(0)> >,
            typename literal_to_tuple< typename LW::template substr_t<1, LW::length> >::type
        >;
    };
    
    template <typename LW>
    using literal_to_tuple_t = typename literal_to_tuple<LW>::type;
    
    template <typename>
    struct tuple_printer;

    template <typename T>
    struct tuple_printer<std::tuple<T>> {
        static std::string const print() {
            std::ostringstream stream;
            stream << T::value;
            return stream.str();
        }
    };

    template <typename H, typename... T>
    struct tuple_printer<std::tuple<H, T...>> {
        static std::string const print() {
            std::ostringstream stream;
            stream << H::value << ",";
            return stream.str() + tuple_printer<std::tuple<T...>>::print();
        }
    };
    
    template <typename>
    struct tuple_to_literal;
    
    template <typename... CT>
    struct tuple_to_literal<std::tuple<CT...>> {
        struct literal {
            static constexpr const char string[sizeof...(CT)] = { CT::value... };
            static constexpr const char * const str() { return string; }
        };
        using type = string<literal>;
    };
    
    template <typename... CT>
    constexpr const char tuple_to_literal<std::tuple<CT...>>::literal::string[];
}

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

    
namespace cregex {
    
    using namespace ctstr;
    
    /// Nodes opearations
    enum OPS : int {
        CONC,   // Concat symbol    (.)
        UNION,  // Union symbol     (|)
        ITER    // Iteration symbol (*)
    };
    
    struct OP_CONC : std::integral_constant<int, OPS::CONC>  {};
    struct OP_UNIN : std::integral_constant<int, OPS::UNION> {};
    struct OP_ITER : std::integral_constant<int, OPS::ITER>  {};
    
    /// Null-terminator (symbol '#')
    enum : char { TERM = '\0' };
    
    template <typename LW, size_t P, char C>
    struct position {
        using literal = LW;
        enum : size_t { pos = P };
        enum : char { symbol = C };
    };
    
    /// Alias (L,R ~ syntax_tree, T ~ operation)
    template <typename L, typename T, typename R>
    using syntax_tree = ctree::bintree<L, T, R>;
    
    /// Leaves contain positions (T ~ position)
    template <typename T>
    using syntax_leaf = syntax_tree<void, T, void>;
    
    template <typename L, typename>
    struct tree_hang_left;
    
    template <typename L, typename T, typename R>
    struct tree_hang_left<L, syntax_tree<void, T, R>> {
        using type = syntax_tree<L, T, R>; };
    
    /// ---- nullable --- ///

    template <typename>
    struct nullable;
    
    template <typename T, size_t P>
    struct nullable<syntax_leaf<position<T,P,TERM>>> : std::true_type {};
    
    template <typename T, size_t P, char C>
    struct nullable<syntax_leaf<position<T,P,C>>> : std::false_type {};
    
    template <typename L, typename R>
    struct nullable<syntax_tree<L,OP_UNIN,R>> : std::integral_constant<bool,
        nullable<L>::value || nullable<R>::value
    > {};
    
    template <typename L, typename R>
    struct nullable<syntax_tree<L,OP_CONC,R>> : std::integral_constant<bool,
        nullable<L>::value && nullable<R>::value
    > {};
    
    template <typename L>
    struct nullable<syntax_tree<L,OP_ITER,void>> : std::true_type {};
    
    /// ---- firstpos --- ///
    
    template <typename>
    struct firstpos;
    
    template <typename T>
    struct firstpos<syntax_leaf<T>> {
        using type = std::tuple<T>; };
    
    template <typename L, typename R>
    struct firstpos<syntax_tree<L,OP_UNIN,R>> {
        using type = typename tuple_concat<
            typename firstpos<L>::type,
            typename firstpos<R>::type
        >::type;
    };
    
    template <typename L, typename R>
    struct firstpos<syntax_tree<L,OP_CONC,R>> {
        using type = typename std::conditional<
            nullable<L>::value,
            typename firstpos<syntax_tree<L,OP_UNIN,R>>::type,
            typename firstpos<L>::type
        >::type;
    };
    
    template <typename L>
    struct firstpos<syntax_tree<L,OP_ITER,void>> {
        using type = typename firstpos<L>::type; };
    
    /// ---- lastpos --- ///
    
    template <typename>
    struct lastpos;
    
    template <typename T>
    struct lastpos<syntax_leaf<T>> {
        using type = std::tuple<T>; };
    
    template <typename L, typename R>
    struct lastpos<syntax_tree<L,OP_UNIN,R>> {
        using type = typename tuple_concat<
            typename lastpos<L>::type,
            typename lastpos<R>::type
        >::type;
    };
    
    template <typename L, typename R>
    struct lastpos<syntax_tree<L,OP_CONC,R>> {
        using type = typename std::conditional<
            nullable<R>::value,
            typename lastpos<syntax_tree<L,OP_UNIN,R>>::type,
            typename lastpos<R>::type
        >::type;
    };
    
    template <typename L>
    struct lastpos<syntax_tree<L,OP_ITER,void>> {
        using type = typename lastpos<L>::type; };
    
    
    template <typename LW, size_t end_idx>
    struct find_group {
        enum : size_t { right_bound = (end_idx == 0) ? 0 : end_idx };
        enum : size_t {
            left_bound = (right_bound == 0) ? 0 : 1
        };
    };
    
    
    
    /**
     * Main expression entity.
     * @arg LW - literal_wrapper
     */
    template <typename LW>
    struct expression {
        
    };
}
    
} // ct
} // ax

#include <ax.ct_impl.hpp>

#undef LOG_HEAD
