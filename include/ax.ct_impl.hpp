#include <ax.ct.hpp>

namespace ax { namespace ct {

template <typename... L1, typename... L2>
struct tuple_concat<std::tuple<L1...>, std::tuple<L2...>> {
    using type = std::tuple<L1..., L2...>; };

template <typename Source, size_t I, typename Acc>
struct tuple_reduce<Source,I,I,Acc> { using type = Acc; };

template <typename Source, size_t A, size_t B, typename Acc>
struct tuple_reduce {
    using type = tuple_concat_t<
        tuple_concat_t<
            Acc,
            std::tuple<typename std::tuple_element<A,Source>::type>
        >,
        typename tuple_reduce<Source,A+1,B,Acc>::type
    >;
};


namespace ctstr {
    
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
        using char_list = std::tuple<CT...>;
        using CharT = typename std::tuple_element<0, char_list>::type::char_type;
        static constexpr const CharT literal[sizeof...(CT)] = { CT::value... };
        
        DEFINE_LITERAL(wrapper, literal);
        using type = string<wrapper>;
    };
    
    template <typename... CT>
    constexpr const typename tuple_to_string<std::tuple<CT...>>::CharT tuple_to_string<std::tuple<CT...>>::literal[];
}
    
} // ct
} // ax
