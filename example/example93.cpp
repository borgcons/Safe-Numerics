//////////////////////////////////////////////////////////////////
// example93.cpp
//
// Copyright (c) 2015 Robert Ramey
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <limits>
#include <boost/integer.hpp>

// include headers to support safe integers
#include "../include/cpp.hpp"
#include "../include/exception.hpp"
#include "../include/safe_integer.hpp"
#include "../include/safe_integer_range.hpp"
#include "../include/safe_integer_literal.hpp"

// use same type promotion as used by the pic compiler
// target compiler XC8 supports:
using pic16_promotion = boost::numeric::cpp<
    8,  // char      8 bits
    16, // short     16 bits
    16, // int       16 bits
    16, // long      16 bits
    32  // long long 32 bits
>;

// ***************************
// 1. Specify exception policies so we will generate a
// compile time error whenever an operation MIGHT fail.

// ***************************
// generate compile time errors if operation could fail 
using trap_policy = boost::numeric::loose_trap_policy;
// generate runtime errors if operation could fail
using exception_policy = boost::numeric::default_exception_policy;

// ***************************
// 2. Create a macro named literal an integral value
// that can be evaluated at compile time.
#define literal(n) make_safe_literal(n, void, void)

// For min speed of 2 mm / sec (24.8 format)
// sec / step = sec / 2 mm * 2 mm / rotation * rotation / 200 steps
#define C0    (5000 << 8)

// For max speed of 400 mm / sec
// sec / step = sec / 400 mm * 2 mm / rotation * rotation / 200 steps
#define C_MIN  (25 << 8)

static_assert(
    boost::numeric::base_value(C0) < 0xffffff,
    "Largest step too long"
);
static_assert(
    boost::numeric::base_value(C_MIN) > 0,
    "Smallest step must be greater than zero"
);
static_assert(
    boost::numeric::base_value(C_MIN) < boost::numeric::base_value(C0),
    "Smallest step must be smaller than largest step"
);

// ***************************
// 3. Create special ranged types for the motor program
// These wiil guarantee that values are in the expected
// ranges and permit compile time determination of when
// exceptional conditions might occur.

using pic_register_t = boost::numeric::safe<
    uint8_t,
    pic16_promotion,
    trap_policy // use for compiling and running tests
>;

// number of steps
using step_t = boost::numeric::safe<
    std::int32_t,
    pic16_promotion,
    exception_policy
>;

// position
using position_t = boost::numeric::safe_unsigned_range<
    0,
    50000, // 500 mm / 2 mm/rotation * 200 steps/rotation
    pic16_promotion,
    exception_policy
>;

// next end of step timer value in format 24.8
// where the .8 is the number of bits in the fractional part.
using ccpr_t = boost::numeric::safe<
    uint32_t,
    pic16_promotion,
    exception_policy
>;

// pulse length in format 24.8
// note: this value is constrainted to be a positive value. But
// we still need to make it a signed type. We get an arithmetic
// error when moving to a negative step number.
using c_t = boost::numeric::safe_signed_range<
    C_MIN,
    C0,
    pic16_promotion,
    exception_policy
>;

// index into phase table
using phase_ix_t = boost::numeric::safe_signed_range<
    0,
    3,
    pic16_promotion,
    exception_policy
>;

// settings for control value output
using phase_t = boost::numeric::safe<
    uint16_t,
    pic16_promotion,
    trap_policy
>;

// direction of rotation
using direction_t = boost::numeric::safe_signed_range<
    -1,
    +1,
    pic16_promotion,
    exception_policy
>;

// some number of microseconds
using microseconds = boost::numeric::safe<
    uint32_t,
    pic16_promotion,
    trap_policy
>;

// *************************** 
// emulate PIC features on the desktop

// filter out special keyword used only by XC8 compiler
#define __interrupt
// filter out XC8 enable/disable global interrupts
#define ei()
#define di()

// emulate PIC special registers
pic_register_t RCON;
pic_register_t INTCON;
pic_register_t CCP1IE;
pic_register_t CCP2IE;
pic_register_t PORTC;
pic_register_t TRISC;
pic_register_t T3CON;
pic_register_t T1CON;

pic_register_t CCPR2H;
pic_register_t CCPR2L;
pic_register_t CCPR1H;
pic_register_t CCPR1L;
pic_register_t CCP1CON;
pic_register_t CCP2CON;
pic_register_t TMR1H;
pic_register_t TMR1L;

// ***************************
// special checked type for bits - values restricted to 0 or 1
using safe_bit_t = boost::numeric::safe_unsigned_range<
    0,
    1,
    pic16_promotion,
    trap_policy
>;

// create type used to map PIC bit names to
// correct bit in PIC register
template<typename T, std::int8_t N>
struct bit {
    T & m_word;
    constexpr explicit bit(T & rhs) :
        m_word(rhs)
    {}
    // special functions for assignment of literal
    constexpr bit & operator=(decltype(literal(1))){
        m_word |= literal(1 << N);
        return *this;
    }
    constexpr bit & operator=(decltype(literal(0))){
        m_word &= ~literal(1 << N);
        return *this;
    }
    // operator to convert to 0 or 1
    constexpr operator safe_bit_t () const {
        return m_word >> literal(N) & literal(1);
    }
};

// define bits for T1CON register
struct  {
    bit<pic_register_t, 7> RD16{T1CON};
    bit<pic_register_t, 5> T1CKPS1{T1CON};
    bit<pic_register_t, 4> T1CKPS0{T1CON};
    bit<pic_register_t, 3> T1OSCEN{T1CON};
    bit<pic_register_t, 2> T1SYNC{T1CON};
    bit<pic_register_t, 1> TMR1CS{T1CON};
    bit<pic_register_t, 0> TMR1ON{T1CON};
} T1CONbits;

// define bits for T1CON register
struct  {
    bit<pic_register_t, 7> GEI{INTCON};
    bit<pic_register_t, 5> PEIE{INTCON};
    bit<pic_register_t, 4> TMR0IE{INTCON};
    bit<pic_register_t, 3> RBIE{INTCON};
    bit<pic_register_t, 2> TMR0IF{INTCON};
    bit<pic_register_t, 1> INT0IF{INTCON};
    bit<pic_register_t, 0> RBIF{INTCON};
} INTCONbits;

#include "motor3.c"

#include <chrono>
#include <thread>

// round 24.8 format to microseconds
microseconds to_microseconds(ccpr_t t){
    return (t + literal(128)) / literal(256);
}

using result_t = uint8_t;
const result_t success = 1;
const result_t fail = 0;

// move motor to the indicated target position in steps
result_t test(position_t m){
    try {
        std::cout << "move motor to " << m << '\n';
        motor_run(m);
        std::cout
        << "step #" << ' '
        << "delay(us)(24.8)" << ' '
        << "delay(us)" << ' '
        << "CCPR" << ' '
        << "motor position" << '\n';
        while(busy()){
            std::this_thread::sleep_for(std::chrono::microseconds(to_microseconds(c)));
            c_t last_c = c;
            ccpr_t last_ccpr = ccpr;
            isr_motor_step();
            std::cout << i << ' '
            << last_c << ' '
            << to_microseconds(last_c) << ' '
            << std::hex << last_ccpr << std::dec << ' '
            << motor_position << '\n';
        };
    }
    catch(std::exception & e){
        std::cout << e.what() << '\n';
        return fail;
    }
    return success;
}

int main(){
    std::cout << "start test\n";
    result_t result = success;
    try {
        initialize();
        // move motor to position 1000
        result &= test(1000);
        // move to the left before zero position
        // fails to compile !
        // result &= ! test(-10);
        // move motor to position 200
        result &= test(200);
        // move motor to position 200 again! Should result in no movement.
        result &= test(200);
        // move motor to position 50000.
        result &= test(50000);
        // move motor back to position 0.
        result &= test(0);
    }
    catch(...){
        std::cout << "test interrupted\n";
        return EXIT_FAILURE;
    }
    std::cout << "end test\n";
    return result == success ? EXIT_SUCCESS : EXIT_FAILURE;
} 
