#include <cassert>
#include <iostream>
#include <ostream>

#include "../include/interval.hpp"
#include <boost/logic/tribool_io.hpp>

bool test1(){
    using namespace boost::numeric;
    interval<std::int16_t> x = {-64, 63};
    std::cout << "x = " << x;
    interval<std::int16_t> y(-128, 126);
    std::cout << "y = " << y;
    assert(operator+<std::int16_t>(x,x) == y);
    std::cout << "x + x =" << operator+<std::int16_t>(x, x);
    std::cout << "x - x = " << operator-<std::int16_t>(x, x);
    return true;
}

bool test2(){
    using namespace boost::numeric;
    boost::numeric::interval<std::int16_t> x = {-64, 63};
    std::cout << "x = " << x;
    std::cout << std::boolalpha << "(x == x) = " << (x == x) << std::endl;
    return true;
}

bool test3(){
    using namespace boost::numeric;
    interval<std::int8_t> t;
    std::cout << "t = " << t;
    interval<std::uint64_t> u;
    std::cout << "u = " << u;
    using max_t = unsigned long long;
    interval< max_t> r = operator+<max_t>(t, u);
    std::cout << "r = " << r << std::endl;
    return true;
}


int main(){
    return (
        test1() &&
        test2() &&
        test3()
    ) ? EXIT_SUCCESS : EXIT_FAILURE;
}