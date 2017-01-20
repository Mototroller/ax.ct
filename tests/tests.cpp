#include <ax.hpp>

#include <algorithm>
#include <set>

#include <ax.ct.hpp>
#include <ax.ct.ctstr.hpp>
#include <ax.ct.math.hpp>
#include <ax.ct.tree.hpp>

using namespace ax;

template <typename T>
struct unsignifier { using type = typename std::make_unsigned<T>::type; };

template <typename Acc, typename T>
struct sizeofs_summator {
    struct type { enum : size_t { value = sizeof(T) + Acc::value }; };
};

struct array_holder { static constexpr const size_t values[] = {1,2,3}; };
constexpr const size_t array_holder::values[];

void ct_test() {
    using namespace ct;
    
    {
        using t1 = std::tuple<char, int, long>;
        using t2 = std::tuple<float, double>;
        using t3 = std::tuple<char, int, long, float, double>;
        using t4 = std::tuple<char, int, long, float, double, bool>;
        using t5 = std::tuple<long, float>;
        using t6 = std::tuple<unsigned char, unsigned int, unsigned long>;
        
        static_assert(std::is_same<tuple_concat_t<t1,t2>, t3>::value, "");
        static_assert(std::is_same<tuple_push_t<t3,bool>, t4>::value, "");
        static_assert(std::is_same<tuple_transform_t<t4,2,4>, t5>::value, "");
        static_assert(std::is_same<tuple_transform_t<t1,0,std::tuple_size<t1>::value, unsignifier>, t6>::value, "");
        
        struct zeroacc { enum : size_t { value = 0 }; };
        using reduced = tuple_reduce_t<t1, sizeofs_summator, zeroacc>;
        static_assert(reduced::type::value == sizeof(char) + sizeof(int) + sizeof(long), "");
    }
    
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
        using namespace math;
        
        static_assert( array_eq({1,2,3}, {1,2,3}), "");
        static_assert(!array_eq({1,2,3}, {1,2,4}), "");
        static_assert(!array_eq({1,2,3}, {1,2,3,4}), "");
        static_assert(!array_eq({1,2,3}, {1,2}), "");
        
        {
            using nums1 = std::tuple< num_t<1>, num_t<2>, num_t<3> >;
            using nums2 = std::tuple< num_t<4>, num_t<5>, num_t<6> >;
            using conc1 = tuple_concat_t<nums1,nums2>;
            
            static_assert(array_eq(tuple_to_array_t<nums1>::values, {1UL,2UL,3UL}), "");
            static_assert(array_eq(tuple_to_array_t<nums2>::values, {4UL,5UL,6UL}), "");
            static_assert(array_eq(tuple_to_array_t<conc1>::values, {1UL,2UL,3UL,4UL,5UL,6UL}), "");
            
            using conv1 = array_to_tuple_t<array_holder>;
            using conv2 = array_to_tuple_t<tuple_to_array_t<nums2>>;
            
            static_assert(std::is_same<nums1,conv1>::value, "");
            static_assert(std::is_same<nums2,conv2>::value, "");
        }
        
        static_assert(isqrt(0) == 0, "");
        static_assert(isqrt(1) == 1, "");
        static_assert(isqrt(2) == 1, "");
        static_assert(isqrt(3) == 1, "");
        static_assert(isqrt(4) == 2, "");
        static_assert(isqrt(5) == 2, "");
        static_assert(isqrt(24) == 4, "");
        static_assert(isqrt(25) == 5, "");
        static_assert(isqrt(26) == 5, "");
        static_assert(isqrt(100) == 10, "");
        static_assert(isqrt(1023) == 31, "");
        static_assert(isqrt(1024) == 32, "");
        static_assert(isqrt(1025) == 32, "");
        static_assert(isqrt(999999999) == 31622, "");
        
        static_assert(smallest_divisor(2) == 2, "");
        static_assert(smallest_divisor(3) == 3, "");
        static_assert(smallest_divisor(4) == 2, "");
        static_assert(smallest_divisor(5) == 5, "");
        static_assert(smallest_divisor(8) == 2, "");
        static_assert(smallest_divisor(31) == 31, "");
        static_assert(smallest_divisor(503) == 503, "");
        
        static_assert(greatest_divisor(2) == 1, "");
        static_assert(greatest_divisor(3) == 1, "");
        static_assert(greatest_divisor(4) == 2, "");
        static_assert(greatest_divisor(5) == 1, "");
        static_assert(greatest_divisor(8) == 4, "");
        static_assert(greatest_divisor(31) == 1, "");
        static_assert(greatest_divisor(503) == 1, "");
        static_assert(greatest_divisor(1024) == 512, "");
        
        static_assert(gcd(1, 1) == 1, "");
        static_assert(gcd(2, 2) == 2, "");
        static_assert(gcd(2, 4) == 2, "");
        static_assert(gcd(3, 5) == 1, "");
        static_assert(gcd(503, 807) == 1, "");
        static_assert(gcd(1000000, 999999) == 1, "");
        static_assert(gcd(0, 5) == 5, "");
        static_assert(gcd(5, 0) == 5, "");
        static_assert(gcd(1024, 512) == 512, "");
        static_assert(gcd(1000000007, 1000000207) == 1, "");
        static_assert(gcd(1000000007, 10007) == 1, "");
        
        static_assert(array_eq(prime_factors<2>::values, {2UL}), "");
        static_assert(array_eq(prime_factors<4>::values, {2UL,2UL}), "");
        static_assert(array_eq(prime_factors<7>::values, {7UL}), "");
        static_assert(array_eq(prime_factors<9>::values, {3UL,3UL}), "");
        static_assert(array_eq(prime_factors<12>::values, {3UL,2UL,2UL}), "");
        static_assert(array_eq(prime_factors<15015>::values, {13UL,11UL,7UL,5UL,3UL}), "");
        static_assert(array_eq(prime_factors<811>::values, {811UL}), "");
        
        static_assert(std::is_same<
            array_to_tuple_t<prime_factors<811>>,
            std::tuple<num_t<811>>>::value,
        "");
        
        static_assert(
            std::is_same<array_to_tuple_t<prime_factors<125>>,
            std::tuple<num_t<5>,num_t<5>,num_t<5>>>::value,
        "");
        
        {
            /// Pollard's Rho factorization
            using namespace rho;
            using rho::prime_factors;
            
            static_assert(divisor<2>::value == 2, "");
            static_assert(divisor<3>::value == 3, "");
            static_assert(divisor<13>::value == 13, "");
            static_assert(divisor<20011>::value == 20011, "");
            
            static_assert(array_eq(prime_factors<4>::values, {2UL,2UL}), "");
            static_assert(array_eq(prime_factors<7>::values, {7UL}), "");
            static_assert(array_eq(prime_factors<9>::values, {3UL,3UL}), "");
            static_assert(array_eq(prime_factors<811>::values, {811UL}), "");
            static_assert(array_eq(prime_factors<10007>::values, {10007UL}), "");
            static_assert(array_eq(prime_factors<282475249>::values, {7UL,7UL,7UL,7UL,7UL,7UL,7UL,7UL,7UL,7UL}), "");
            
            //static_assert(array_eq(prime_factors<15015>::values, {13UL,11UL,7UL,5UL,3UL}), "");
            
            auto const& pf15015 = prime_factors<15015>::values;
            std::multiset<size_t> fac1(std::begin(pf15015), std::end(pf15015));
            std::multiset<size_t> res1 = {13,11,7,5,3};
            LIGHT_TEST(fac1 == res1);
            
            auto const& pf8060 = prime_factors<8060>::values;
            std::multiset<size_t> fac2(std::begin(pf8060), std::end(pf8060));
            std::multiset<size_t> res2 = {2,2,13,31,5};
            LIGHT_TEST(fac2 == res2);
        }
    }
    
    {
        using namespace tree;
        using namespace math;
        
        using t1 = node<
            num_t<5>,
            node<
                num_t<3>,
                leaf<num_t<2>>,
                leaf<num_t<5>>
            >,
            node<
                num_t<7>,
                NIL,
                leaf<num_t<8>>
            >
        >;
        
        using vals1 = tuple_to_array_t<walk_t<t1>>;
        static_assert(array_eq(vals1::values, {2UL,3UL,5UL,5UL,7UL,8UL}), "");
        
        using found1 = search_t<t1, double, sizeof_comp>;
        using found2 = search_t<t1, num_t<5>, sizeof_comp>;
        
        static_assert(std::is_same<found1, NIL>::value, "");
        static_assert(std::is_same<found2, t1>::value, "");
        
        struct num_comp : eq_traits<num_less> {};
        
        //using found3 = search_t<t1, NIL, num_comp>; // must not fap!
        using found4 = search_t<t1, num_t<5>, num_comp>;
        using found5 = search_t<t1, num_t<8>, num_comp>;
        using found6 = search_t<t1, num_t<7>, num_comp>;
        
        static_assert(std::is_same<found4, t1>::value, "");
        static_assert(std::is_same<found5, leaf<num_t<8>>>::value, "");
        
        static_assert(!std::is_same<found6, leaf<num_t<7>>>::value, "");
        static_assert(std::is_same<found6, node<num_t<7>,NIL,leaf<num_t<8>>>>::value, ""); // subtree
        
        using t2 = leaf<num_t<5>>;
        using t3 = typename insert<t2, num_t<3>, num_comp>::type;
        using t4 = typename insert<t3, num_t<7>, num_comp>::type;
        
        for(auto x : tuple_to_array_t<walk_t<t4>>::values) stdcout(x);
        //static_assert(array_eq(tuple_to_array_t<walk_t<t4>>::values, {3UL,5UL,7UL}), "");
        
        /*/!
        stdcout(typeid(t3).name());
        stdcout(typeid(t3::LT).name());
        stdcout(typeid(t3::LT::parent).name());
        stdcout(typeid(t3::RT).name());
        stdcout(typeid(t3::parent).name());
        //*/
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