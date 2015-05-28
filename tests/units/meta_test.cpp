/*
 * meta_test.cpp
 *
 *  Created on: Apr 2, 2015
 *      Author: masha
 */

#include <xtomic/impl/xtraits.hpp>

#include <gtest/gtest.h>


#if !LFDS_USE_CPP11
template<typename T>
struct is_int
{
    static const bool value = false;
};

template<>
struct is_int<int>
{
    static const bool value = true;
};

TEST(IntegralTypes, self_test)
{
    EXPECT_FALSE(is_int<const int>::value);
    EXPECT_FALSE(is_int<volatile int>::value);
    EXPECT_FALSE(is_int<volatile const int>::value);
    EXPECT_TRUE(is_int<int>::value);
}
TEST(IntegralTypes, remove_const)
{
    EXPECT_TRUE(is_int<xtomic::remove_const<int>::type>::value);
    EXPECT_TRUE(is_int<xtomic::remove_const<const int>::type>::value);
}
TEST(IntegralTypes, remove_volatile)
{
    EXPECT_TRUE(is_int<xtomic::remove_volatile<int>::type>::value);
    EXPECT_TRUE(is_int<xtomic::remove_volatile<volatile int>::type>::value);
}

TEST(IntegralTypes, remove_cv)
{
    EXPECT_TRUE(is_int<xtomic::remove_cv<int>::type>::value);
    EXPECT_TRUE(is_int<xtomic::remove_cv<volatile int>::type>::value);
    EXPECT_TRUE(is_int<xtomic::remove_cv<const int>::type>::value);
    EXPECT_TRUE(is_int<xtomic::remove_cv<const volatile int>::type>::value);
    EXPECT_TRUE(is_int<xtomic::remove_cv<volatile const int>::type>::value);
}
TEST(IntegralTypes, unquallified_int)
{
    EXPECT_TRUE(xtomic::is_unqualified_integer<char>::value);
    EXPECT_TRUE(xtomic::is_unqualified_integer<signed char>::value);
    EXPECT_TRUE(xtomic::is_unqualified_integer<unsigned char>::value);
    EXPECT_TRUE(xtomic::is_unqualified_integer<short>::value);
    EXPECT_TRUE(xtomic::is_unqualified_integer<short int>::value);
    EXPECT_TRUE(xtomic::is_unqualified_integer<unsigned short>::value);
    EXPECT_TRUE(xtomic::is_unqualified_integer<unsigned short int>::value);
    EXPECT_TRUE(xtomic::is_unqualified_integer<signed short>::value);
    EXPECT_TRUE(xtomic::is_unqualified_integer<signed short int>::value);
    EXPECT_TRUE(xtomic::is_unqualified_integer<int>::value);
    EXPECT_TRUE(xtomic::is_unqualified_integer<unsigned int>::value);
    EXPECT_TRUE(xtomic::is_unqualified_integer<signed int>::value);
    EXPECT_TRUE(xtomic::is_unqualified_integer<long>::value);
    EXPECT_TRUE(xtomic::is_unqualified_integer<long int>::value);
    EXPECT_TRUE(xtomic::is_unqualified_integer<unsigned long>::value);
    EXPECT_TRUE(xtomic::is_unqualified_integer<unsigned long int>::value);
    EXPECT_TRUE(xtomic::is_unqualified_integer<signed long>::value);
    EXPECT_TRUE(xtomic::is_unqualified_integer<signed long int>::value);
    EXPECT_FALSE(xtomic::is_unqualified_integer<double>::value);
    EXPECT_FALSE(xtomic::is_unqualified_integer<bool>::value);
}
TEST(IntegralTypes, is_integer)
{
    EXPECT_TRUE(xtomic::is_integer<char>::value);
    EXPECT_TRUE(xtomic::is_integer<signed char>::value);
    EXPECT_TRUE(xtomic::is_integer<unsigned char>::value);
    EXPECT_TRUE(xtomic::is_integer<const char>::value);
    EXPECT_TRUE(xtomic::is_integer<const signed char>::value);
    EXPECT_TRUE(xtomic::is_integer<const unsigned char>::value);
    EXPECT_TRUE(xtomic::is_integer<const volatile char>::value);
    EXPECT_TRUE(xtomic::is_integer<const volatile signed char>::value);
    EXPECT_TRUE(xtomic::is_integer<const volatile unsigned char>::value);
    EXPECT_FALSE(xtomic::is_integer<bool>::value);
    EXPECT_FALSE(xtomic::is_integer<const bool>::value);
    EXPECT_FALSE(xtomic::is_integer<volatile bool>::value);
    EXPECT_FALSE(xtomic::is_integer<const volatile bool>::value);
}
TEST(IntegralTypes, is_bool)
{
    EXPECT_TRUE(xtomic::is_bool<bool>::value);
    EXPECT_TRUE(xtomic::is_bool<const bool>::value);
    EXPECT_TRUE(xtomic::is_bool<volatile bool>::value);
    EXPECT_TRUE(xtomic::is_bool<const volatile bool>::value);
    EXPECT_FALSE(xtomic::is_bool<char>::value);
    EXPECT_FALSE(xtomic::is_bool<const char>::value);
    EXPECT_FALSE(xtomic::is_bool<volatile char>::value);
    EXPECT_FALSE(xtomic::is_bool<const volatile char>::value);
}
TEST(IntegralTypes, is_pointer)
{
    EXPECT_TRUE(xtomic::is_pointer<char*>::value);
    EXPECT_TRUE(xtomic::is_pointer<const char*>::value);
    EXPECT_TRUE(xtomic::is_pointer<volatile char*>::value);
    EXPECT_TRUE(xtomic::is_pointer<const volatile char*>::value);
    EXPECT_TRUE(xtomic::is_pointer<double*>::value);
    EXPECT_TRUE(xtomic::is_pointer<void*>::value);
    EXPECT_TRUE(xtomic::is_pointer<xtomic::is_pointer<double>*>::value);
    EXPECT_FALSE(xtomic::is_pointer<double>::value);
    EXPECT_FALSE(xtomic::is_pointer<char>::value);
    EXPECT_FALSE(xtomic::is_pointer<xtomic::is_pointer<double> >::value);
}
#endif

TEST(IntegralTypes, is_integral)
{
    EXPECT_TRUE(xtomic::is_integral<char*>::value);
    EXPECT_TRUE(xtomic::is_integral<const char*>::value);
    EXPECT_TRUE(xtomic::is_integral<volatile char*>::value);
    EXPECT_TRUE(xtomic::is_integral<const volatile char*>::value);
    EXPECT_TRUE(xtomic::is_integral<double*>::value);
    EXPECT_TRUE(xtomic::is_integral<void*>::value);
    EXPECT_TRUE(xtomic::is_integral<char>::value);
    EXPECT_TRUE(xtomic::is_integral<const char>::value);
    EXPECT_TRUE(xtomic::is_integral<volatile char>::value);
    EXPECT_TRUE(xtomic::is_integral<signed char>::value);
    EXPECT_TRUE(xtomic::is_integral<const signed char>::value);
    EXPECT_TRUE(xtomic::is_integral<volatile signed char>::value);
    EXPECT_TRUE(xtomic::is_integral<const volatile signed char>::value);
    EXPECT_TRUE(xtomic::is_integral<unsigned char>::value);
    EXPECT_TRUE(xtomic::is_integral<const unsigned char>::value);
    EXPECT_TRUE(xtomic::is_integral<volatile unsigned char>::value);
    EXPECT_TRUE(xtomic::is_integral<const volatile unsigned char>::value);
    EXPECT_TRUE(xtomic::is_integral<int>::value);
    EXPECT_TRUE(xtomic::is_integral<const int>::value);
    EXPECT_TRUE(xtomic::is_integral<volatile int>::value);
    EXPECT_TRUE(xtomic::is_integral<signed int>::value);
    EXPECT_TRUE(xtomic::is_integral<const signed int>::value);
    EXPECT_TRUE(xtomic::is_integral<volatile signed int>::value);
    EXPECT_TRUE(xtomic::is_integral<const volatile signed int>::value);
    EXPECT_TRUE(xtomic::is_integral<unsigned int>::value);
    EXPECT_TRUE(xtomic::is_integral<const unsigned int>::value);
    EXPECT_TRUE(xtomic::is_integral<volatile unsigned int>::value);
    EXPECT_TRUE(xtomic::is_integral<const volatile unsigned int>::value);
    EXPECT_TRUE(xtomic::is_integral<bool>::value);
    EXPECT_TRUE(xtomic::is_integral<const bool>::value);
    EXPECT_TRUE(xtomic::is_integral<volatile bool>::value);
    EXPECT_FALSE(xtomic::is_integral<double>::value);
}
