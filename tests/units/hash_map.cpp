/*
 * hash_map.cpp
 *
 *  Created on: Jan 28, 2015
 *      Author: masha
 */

#include <gtest/gtest.h>

#include <hash_map.hpp>
#include <utils/my-int-wrapper.hpp>
#include "uniform_hash_map_test.hpp"

typedef lfds::my::int_wrapper<int> key_type;
typedef lfds::my::int_wrapper<int> mapped_type;

typedef make_map_uniform_tests<key_type, mapped_type, map_type::Generic> test_maker_type;

MAKE_ALL_TESTS_FOR_MAP(test_maker_type, Generic)

