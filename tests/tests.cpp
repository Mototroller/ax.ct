#include <ax.hpp>

#include <ax.ct.hpp>

using namespace ax;

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
        
        static_assert(height<tree>::value == 3, "");
    }
    
    {
        using namespace ctstr;
        using ctstr::strlen;
        
        constexpr auto* str1 = "aababa";
        
        constexpr auto* pat1 = "aba";
        constexpr auto* pat2 = "abaa";
        constexpr auto* pat3 = "ba";
        constexpr auto* pat4 = "a";
        
        static_assert(find_substr(str1, str1) == 0, "");
        static_assert(find_substr(str1, pat1) == 1, "");
        static_assert(find_substr(str1, pat2) == npos, "");
        static_assert(find_substr(str1, pat3) == 2, "");
        static_assert(find_substr(str1, pat4) == 0, "");
        
        static_assert(count_substr(str1, str1) == 1, "");
        static_assert(count_substr(str1, pat1) == 1, "");
        static_assert(count_substr(str1, pat2) == 0, "");
        static_assert(count_substr(str1, pat3) == 2, "");
        static_assert(count_substr(str1, pat4) == 4, "");
        
        //constexpr auto str2 = "abc*|d"; // clang error, TODO: why
        #define str2 "abc*|d"
        
        DEFINE_LITERAL(literal, str2);
        
        using str = string<literal>;
        
        static_assert(str::length == strlen(str2), "");
        static_assert(str::at(0) == 'a', "");
        static_assert(str::at(5) == 'd', "");
        static_assert(eq<str,str>::value, "");
        
        LIGHT_TEST(str::to_string() == str2);
        
        using sub = str::substr_t<1,str::length>;
        
        static_assert(sub::length + 1 == str::length, "");
        static_assert(sub::at(0) == 'b', "");
        static_assert(sub::at(4) == 'd', "");
        static_assert(!eq<sub,str>::value, "");
        
        using conc = concat_t<str,sub>;
        
        LIGHT_TEST(conc::to_string() == str::to_string() + sub::to_string());
        
        using packed = string_to_tuple_t<str>;
        using unpacked = tuple_to_string_t<packed>;
        
        static_assert(eq<str,unpacked>::value, "");
        
        #undef str2
    }
    
    {
        using namespace cregex;
        
        #define TEST_REGEX "aba*|c"
        
        #undef TEST_REGEX
    }
}

int main() {
    ct_test();
}