/*
 * hash_map_integral_value.cpp
 *
 *  Created on: Apr 20, 2015
 *      Author: masha
 */
#include <gtest/gtest.h>

#include <xtomic/hash_map.hpp>
#include <utils/my-int-wrapper.hpp>
#include "uniform_hash_map_test.hpp"

typedef lfds::my::int_wrapper<int> key_type;
typedef int mapped_type;

typedef make_map_uniform_tests<key_type, mapped_type, map_type::IntegralValue> test_maker_type;

MAKE_ALL_TESTS_FOR_MAP(test_maker_type, IntegralValue)

