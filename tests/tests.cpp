#include <ax.hpp>

#include <ax.ct.hpp>

using namespace ax;

#define TEST_STRING "aba*|c"

void ct_test() {
    using namespace ct;
    
    {
        using namespace ctree;
        
        //|       c      |
        //|      / \     |
        //|     c   c    |
        //|    / \       |
        //|   c   c      |
        using tree = bintree<
            bintree<
                leaf<char>,
                char,
                leaf<char>
            >,
            char,
            leaf<char>
        >;
        stdcout(height<tree>::value);
    }
    
    {
        using namespace cregex;
        
        //struct Str { static constexpr const char * const str() { return TEST_STRING; } };
        DEFINE_LITERAL(Str, TEST_STRING);
        
        using WStr = string<Str>;
        
        stdcout(WStr::length, " ", WStr::source());
        auto at1 = WStr::char_at<0>::value;
        auto at2 = WStr::at(5);
        stdcout(at1, " ", at2, " ", WStr::to_string(), " ", WStr::is_base);
        
        using subWStr = WStr::substr_t<1,WStr::length>;
        auto at3 = subWStr::at(0);
        auto at4 = subWStr::at(1);
        stdcout(at3, " ", at4, " ", subWStr::to_string(), " ", subWStr::is_base);
        
        using sub2WStr = subWStr::substr_t<1,subWStr::length>;
        auto at5 = sub2WStr ::at(0);
        auto at6 = sub2WStr ::at(1);
        stdcout(at5, " ", at6, " ", sub2WStr ::to_string(), " ", sub2WStr ::is_base);
        
        using tuplified = literal_to_tuple_t<WStr>;
        stdcout(tuple_printer<tuplified>::print());
        
        using unpacked = typename tuple_to_literal<tuplified>::type;
        stdcout(unpacked::to_string());
        
        stdcout(sizeof(std::decay<decltype(U"qwerty"[0])>::type));
    }
}

#undef TEST_STRING

int main() {
    ct_test();
}