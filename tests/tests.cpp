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

template <typename Acc, typename T>
struct concatenator { using type = ct::tuple_push_t<Acc, T>; };

struct array_holder { static constexpr const size_t values[] = {1,2,3}; };
constexpr const size_t array_holder::values[];

struct str_func { constexpr static char const* str() { return "12345"; } };
struct str_data { constexpr static auto str = str_func::str(); };

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
        
        using concatenated = tuple_reduce_t<t2, concatenator, t1>;
        static_assert(std::is_same<concatenated, t3>::value, "");
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
        
        static_assert(strlen(str_func::str()) == 5, "");
        static_assert(strlen(str_data::str) == 5, "");
        
        DEFINE_LITERAL(literal_from_func, str_func::str());
        DEFINE_LITERAL(literal_from_data, str_data::str);
        
        using string_from_func = string<literal_from_func>;
        using string_from_data = string<literal_from_data>;
        
        static_assert(eq<string_from_func, string_from_data>::value, "");
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
                leaf<num_t<4>>
            >,
            node<
                num_t<7>,
                NIL,
                leaf<num_t<8>>
            >
        >;
        
        static_assert(height<t1>::value == 3, "");
        
        //stdcout("\n"); for(auto x : tuple_to_array_t<walk_t<t1>>::values) stdcout(x);
        
        using vals1 = tuple_to_array_t<walk_t<t1>>;
        static_assert(array_eq(vals1::values, {2UL,3UL,4UL,5UL,7UL,8UL}), "");
        
        using found1 = search_t<t1, double, sizeof_comp>;
        using found2 = search_t<t1, num_t<5>, sizeof_comp>;
        
        static_assert(std::is_same<found1, NIL>::value, "");
        static_assert(std::is_same<found2, t1>::value, "");
        
        struct num_comp : eq_traits<num_less> {};
        
        //using found3 = search_t<t1, NIL, num_comp>; // must not fap!
        using found3 = search_t<NIL, num_t<0>, num_comp>;
        static_assert(std::is_same<found3, NIL>::value, "");
        
        using found4 = search_t<t1, num_t<5>, num_comp>;
        using found5 = search_t<t1, num_t<8>, num_comp>;
        using found6 = search_t<t1, num_t<7>, num_comp>;
        
        static_assert(std::is_same<found4, t1>::value, "");
        static_assert(std::is_same<found5, leaf<num_t<8>>>::value, "");
        
        static_assert(!std::is_same<found6, leaf<num_t<7>>>::value, "");
        static_assert(std::is_same<found6, node<num_t<7>,NIL,leaf<num_t<8>>>>::value, ""); // subtree
        
        static_assert(std::is_same<typename search<t1, num_t<7>, num_comp>::tree, t1>::value, "");
        
        static_assert(std::is_same<parent_of_t<t1, t1    >, NIL   >::value, "");
        static_assert(std::is_same<parent_of_t<t1, found2>, NIL   >::value, "");
        static_assert(std::is_same<parent_of_t<t1, found5>, found6>::value, "");
        static_assert(std::is_same<parent_of_t<t1, found6>, t1    >::value, "");
        
        using t2 = leaf<num_t<5>, num_comp>;
        using t3 = insert_t<t2, num_t<3>>;
        using t4 = insert_t<t3, num_t<7>>;
        
        static_assert(height<t4>::value == 2, "");
        static_assert(array_eq(tuple_to_array_t<walk_t<t4>>::values, {3UL,5UL,7UL}), "");
        
        using t5 = insert_t<insert_t<insert_t<t4, num_t<2>>, num_t<4>>, num_t<8>>;
        static_assert(array_eq(tuple_to_array_t<walk_t<t5>>::values, {2UL,3UL,4UL,5UL,7UL,8UL}), "");
        
        // Debug representation
        stdcout(print<t5, value_printer>::str());
        
        static_assert(!std::is_same<t5, t1>::value, ""); // cause of different initial comparators
        static_assert( tree_eq<t1,t1>::value, ""); // inner type-by-type comparing
        static_assert(!tree_eq<t1,t2>::value, "");
        static_assert( tree_eq<t1,t5>::value, "");
        
        using t6 = insert_tuple_t<NIL, std::tuple<
            num_t<5>,
            num_t<7>,
            num_t<3>,
            num_t<4>,
            num_t<2>,
            num_t<8>
        >, num_comp>;
        
        static_assert(std::is_same<t5, t6>::value, "");
        static_assert(tree_eq<t1,t6>::value, "");
        
        using lwalk = level_walk_t<t6>;
        static_assert(array_eq(tuple_to_array_t<lwalk>::values, {5UL,3UL,7UL,2UL,4UL,8UL}), "");
        
        static_assert(array_eq(tuple_to_array_t<collect_level_t<t6, 1>>::values, {3UL,7UL}), "");
        static_assert(array_eq(tuple_to_array_t<collect_level_t<t6, 2>>::values, {2UL,4UL,8UL}), "");
        
        using min1 = min_node_t<t6>;
        using min2 = min_node_t<typename t6::RT>;
        static_assert(std::is_same<min1, leaf<num_t<2>, num_comp>>::value, "");
        static_assert(std::is_same<min2, typename t6::RT>::value, "");
        
        using tree::remove;
        
        static_assert(children_amount<t6>::value == 2, "");
        static_assert(children_amount<leaf<num_t<999>, num_comp>>::value == 0, "");
        static_assert(children_amount<found6>::value == 1, "");
        
        using rem1 = remove_tuple_t<t6, std::tuple<
            num_t<7>,
            num_t<3>,
            num_t<4>,
            num_t<2>,
            num_t<8>
        >>;
        static_assert(std::is_same<t2, rem1>::value, "");
        
        using rem2 = remove_tuple_t<t6, std::tuple<
            num_t<4>,
            num_t<2>,
            num_t<3>,
            num_t<99>,
            num_t<3>,
            num_t<7>,
            num_t<8>
        >>;
        static_assert(std::is_same<rem2, rem1>::value, "");
        
        using rem3 = remove_tuple_t<t6, std::tuple<num_t<5>>>;
        using rem4 = remove_tuple_t<t6, std::tuple<num_t<5>, num_t<8>>>;
        
        static_assert(!std::is_same<rem3, rem4>::value, "");
        
        using rem5 = remove_tuple_t<t6, std::tuple<num_t<3>, num_t<7>, num_t<5>>>;
        stdcout(print<rem5, value_printer>::str());
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