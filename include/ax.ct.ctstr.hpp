#pragma once

#include <string>
#include <sstream>
#include <type_traits>
#include <utility>

#define LOG_HEAD "[ct][ctstr]: "

namespace ax { namespace ct {

/// Contains some compile-time string algorithms
namespace ctstr {
        
    using npos_type = decltype(std::string::npos);

    /// Explicitly defined std::string::npos
    enum : npos_type { npos = std::string::npos };

    /// Explicitly defined constexpr strlen()
    template <typename Char>
    constexpr size_t strlen(Char const* str) {
        return *str == Char{'\0'} ? 0 : 1 + strlen(str + 1); }

    template <typename Char>
    constexpr size_t find_substr_impl(
        Char const* c, Char const* s,
        size_t result, size_t offset, size_t pos
    ) {
        return
        (s[offset] == Char{'\0'}) ? ( // matching completed
            result
        ) : (
            (c[pos] == Char{'\0'}) ? ( // end of string
                npos
            ) : (
                (c[pos] == s[offset]) ? ( // symbol, continue (or start if offset == 0)
                    find_substr_impl(c, s, (offset == 0 ? pos : result), offset + 1, pos + 1)
                ) : ( // fail, reset
                    find_substr_impl(c, s, 0, 0, (offset == 0 ? pos + 1 : result + 1))
                )
            )
        );
    }

    /// @returns position of the first occurrence, std::string::npos otherwise
    template <typename Char>
    constexpr size_t find_substr(Char const* c, Char const* s, size_t from = 0) {
        return find_substr_impl(c, s, 0, 0, from); }

    /// @returns the number of occurrences of substring
    template <typename Char>
    constexpr size_t count_substr(Char const* c, Char const* s) {
        return find_substr(c, s) == npos ? 0 : 1 + count_substr(&c[find_substr(c, s) + strlen(s)], s); }

    
    /// --- Literals --- ///

    /// Character <=> type
    template <typename CharT, CharT C>
    struct basic_char_t {
        using char_type = CharT;
        enum : char_type { value = C };
    };

    template <char C>
    using char_t = basic_char_t<char, C>;

    /// Example of prepared to use string literal
    struct literal_example_ {
        /// Symbols type
        using CharT = typename std::decay<decltype("example"[0])>::type;
        
        /// Literal
        static constexpr const CharT* const str() { return "example"; }
    };

    /// Wraps given string literal to structure
    #define DEFINE_LITERAL(name, string) struct name { \
        using CharT = typename std::decay<decltype(string[0])>::type; \
        static constexpr const CharT* const str() { return string; } }

    /// Represents characters subset of given string [from,to)
    template <typename, size_t, size_t>
    struct subset;

    /// Unpacks subset of characters to tuple (adds null-terminator)
    template <typename>
    struct subset_to_tuple;

    /// Unpacks full string to tuple (with null-terminator)
    template <typename>
    struct string_to_tuple;

    template <typename S>
    using string_to_tuple_t = typename string_to_tuple<S>::type;

    /// Packs tuple of character classes to string
    template <typename>
    struct tuple_to_string;

    template <typename T>
    using tuple_to_string_t = typename tuple_to_string<T>::type;

    /// Constructs string with character types values ("char" supported)
    template <typename>
    struct tuple_printer;

    /// Wrapper, allows compile-time "string" (literal) manipulation and analysis
    template <typename T>
    struct string {
        
        using CharT = typename T::CharT;
        using std_string = std::basic_string<CharT>;
        
        template <CharT C>
        using xchar_t = basic_char_t<CharT, C>;
        
        enum : size_t { length = strlen(T::str()) };
        
        /// Equivalent of full string
        using main_subset = subset<string, 0, length>;
        
        /// @returns pointer to string literal
        constexpr static CharT const* const str() {
            return T::str(); }
        
        /// @returns character by index
        constexpr static CharT const at(size_t idx) {
            return main_subset::at(idx); }
        
        static std_string to_string() {
            return main_subset::to_string(); }
        
        /// Definnes substring as new type
        template <size_t from, size_t to>
        struct substr {
            static_assert(from <= to && to <= length, LOG_HEAD "invalid substring bounds");
            using type = typename tuple_to_string<
                typename subset_to_tuple<subset<string, from, to>>::type
            >::type;
        };
        
        template <size_t from, size_t to>
        using substr_t = typename substr<from, to>::type;
    };

    /// Provides access to subset of characters [Alpha, Omega)
    template <typename S, size_t Alpha, size_t Omega>
    struct subset {
        static_assert(Alpha <= Omega, LOG_HEAD "invalid subset bounds");

        /// Provides base type wrapping full literal
        using base = S;
        using CharT = typename base::CharT;
        using std_string = typename base::std_string;
        
        enum : size_t { alpha = Alpha }; // first character index
        enum : size_t { omega = Omega }; // last character index (not inclusive)
        enum : size_t { length = omega - alpha };
        
        /// Same as substr(), but without null-termination
        template <size_t begin, size_t end>
        using slice_t = subset<base, alpha + begin, alpha + end>;
        
        /// @returns character by index
        constexpr static CharT const at(size_t idx) {
            return base::str()[alpha + idx]; }
        
        static std_string to_string() {
            return std_string(&(base::str()[alpha]), length); }
    };

    /// Concatenates two strings into new type
    template <typename S1, typename S2>
    struct concat {
        using type = tuple_to_string_t<tuple_concat_t<
            tuple_transform_t<string_to_tuple_t<S1>,0,S1::length>,
            string_to_tuple_t<S2>
        >>;
    };

    template <typename S1, typename S2>
    using concat_t = typename concat<S1,S2>::type;

    /// Checks two strings equality
    template <typename S1, typename S2>
    struct eq : std::integral_constant<bool,
        std::is_same<string_to_tuple_t<S1>,string_to_tuple_t<S2>>::value> {};

    template <typename S, size_t I>
    struct subset_to_tuple< subset<S,I,I> > {
        using CharT = typename S::CharT;
        
        template <CharT C>
        using xchar_t = basic_char_t<CharT, C>;
        
        using type = std::tuple< xchar_t<CharT{'\0'}> >;
    };

    template <typename S, size_t A, size_t O>
    struct subset_to_tuple< subset<S,A,O> > {
        using CharT = typename S::CharT;
        
        template <CharT C>
        using xchar_t = basic_char_t<CharT, C>;
        
        using sub = subset<S,A,O>;
        
        using type = tuple_concat_t<
            std::tuple< xchar_t<sub::at(0)> >,
            typename subset_to_tuple< typename sub::template slice_t<1, sub::length> >::type
        >;
    };

    /// Creates tuple of symbol classes from given string
    template <typename S>
    struct string_to_tuple {
        using type = typename subset_to_tuple<typename S::main_subset>::type; };

    template <>
    struct tuple_printer<std::tuple<>> { static std::string const print() { return ""; } };    

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

    template <typename... CT>
    struct tuple_to_string<std::tuple<CT...>> {
    private:
        using char_list = std::tuple<CT...>;
        using CharT = typename std::tuple_element<0, char_list>::type::char_type;
        static constexpr const CharT literal[sizeof...(CT)] = { CT::value... };
        DEFINE_LITERAL(wrapper, literal);
    public:
        using type = string<wrapper>;
    };

    template <typename... CT>
    constexpr const typename tuple_to_string<std::tuple<CT...>>::CharT tuple_to_string<std::tuple<CT...>>::literal[];

} // ctstr

/// Experimental: compile time regex-to-DFA
namespace cregex {
    
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

#undef LOG_HEAD
