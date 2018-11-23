//  Copyright (c) 2012 Robert Ramey
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>

#include <boost/safe_numerics/safe_integer.hpp>
#include <boost/safe_numerics/native.hpp>

#include <boost/mp11/list.hpp>
#include <boost/mp11/algorithm.hpp>
#include "test_values.hpp"


// note: These tables presume that the the size of an int is 32 bits.
// This should be changed for a different architecture or better yet
// be dynamically adjusted depending on the indicated architecture

const char *test_division_result[boost::mp11::mp_size<test_values>::value] = {
//      0       0       0       0
//      012345670123456701234567012345670
//      012345678901234567890123456789012
/* 0*/ "................................x",
/* 1*/ "................................x",
/* 2*/ "........................xxxxxxxxx",
/* 3*/ "........................xxxxxxxxx",
/* 4*/ "................................x",
/* 5*/ "................................x",
/* 6*/ "........................xxxxxxxxx",
/* 7*/ "........................xxxxxxxxx",

/* 8*/ "................................x",
/* 9*/ "................................x",
/*10*/ "...x...x...x............xxxxxxxxx",
/*11*/ "........................xxxxxxxxx",
/*12*/ "................................x",
/*13*/ "................................x",
/*14*/ "...x...x...x...x............xxxxx",
/*15*/ "............................xxxxx",

//      0       0       0       0
//      012345670123456701234567012345670
//      012345678901234567890123456789012
/*16*/ "................................x",
/*17*/ "................................x",
/*18*/ "................................x",
/*19*/ "................................x",
/*20*/ "................................x",
/*21*/ "................................x",
/*22*/ "................................x",
/*23*/ "................................x",

/*24*/ "..xx..xx..xx....................x",
/*25*/ "..xx..xx..xx....................x",
/*26*/ "..xx..xx..xx....................x",
/*27*/ "..xx..xx..xx....................x",
/*28*/ "..xx..xx..xx..xx................x",
/*29*/ "..xx..xx..xx..xx................x",
/*30*/ "..xx..xx..xx..xx................x",
/*31*/ "..xx..xx..xx..xx................x",
/*32*/ "................................x"
};

#include <boost/mp11/algorithm.hpp>
#include <boost/core/demangle.hpp>

template <class T>
using safe_t = boost::safe_numerics::safe<
    T,
    boost::safe_numerics::native
>;
#include "test_divide.hpp"

using namespace boost::mp11;

template<typename L>
struct test {
    static_assert(mp_is_list<L>(), "must be a list of integral constants");
    bool m_error;
    test(bool b = true) : m_error(b) {}
    operator bool(){
        return m_error;
    }
    template<typename T>
    void operator()(const T &){
        static_assert(mp_is_list<T>(), "must be a list of two integral constants");
        constexpr size_t i1 = mp_first<T>(); // index of first argument
        constexpr size_t i2 = mp_second<T>();// index of second argument
        std::cout << i1 << ',' << i2 << ',';
        using T1 = typename boost::mp11::mp_at_c<L, i1>::value_type;
        using T2 = typename boost::mp11::mp_at_c<L, i2>::value_type;
        m_error &= test_divide<T1, T2>(
            boost::mp11::mp_at_c<L, i1>(), // value of first argument
            boost::mp11::mp_at_c<L, i2>(), // value of second argument
            boost::core::demangle(typeid(T1).name()).c_str(),
            boost::core::demangle(typeid(T2).name()).c_str(),
            test_division_result[i1][i2]
        );
    }
};

int main(){
    //TEST_EACH_VALUE_PAIR
    test<test_values> rval(true);

    using value_indices = mp_iota_c<mp_size<test_values>::value>;
    mp_for_each<
        mp_product<mp_list, value_indices, value_indices>
    >(rval);

    std::cout << (rval ? "success!" : "failure") << std::endl;
    return ! rval ;
}
