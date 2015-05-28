/*
 * hash_set.cpp
 *
 *  Created on: Feb 12, 2015
 *      Author: masha
 */

#include <xtomic/hash_set.hpp>

#include "gtest/gtest.h"
#include "uniform_hash_set_test.hpp"

typedef xtomic::my::int_wrapper<int> key_type;

MAKE_ALL_TESTS_FOR_SET(key_type, Generic)

