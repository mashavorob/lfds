/*
 * demo_stack.cpp
 *
 *  Created on: Dec 17, 2014
 *      Author: masha
 */

#include <iostream>
#include <stack>
#include <chrono>

#include "demo_stack.hpp"
#include "counted_collection_wrapper.hpp"
#include "std_stack_wrapper.hpp"
#include "benchmark.hpp"
#include "stack.hpp"

void DemoStack() {
	static const std::chrono::seconds 	duration(10);
	static const std::size_t 			capacity = 100;
	static const std::size_t 			num_producers=2;
	static const std::size_t 			num_consumers=2;


	typedef std_stack_wrapper< std::stack<int> > ref_collection_type;
	typedef lfds::stack<int, true>				 fixed_collection_type;
	typedef lfds::stack<int, false>				 dynamic_collection_type;

	ref_collection_type ref_coll(capacity);
	fixed_collection_type fixed_coll(capacity);
	dynamic_collection_type dynamic_coll(capacity);

	std::cout << "std::stack:" << std::endl;
	double ref_perf = benchmark(ref_coll, duration, num_producers,
			num_consumers);
	std::cout << "size at exit: " << ref_coll.size() << std::endl
			<< std::endl;

	std::cout << "lock free fixed size stack:" << std::endl;
	double fixed_perf = benchmark(fixed_coll, duration, num_producers,
			num_consumers);
	std::cout << "size at exit: " << fixed_coll.size() << std::endl
			<< std::endl;

	std::cout << "lock free dynamic size stack:" << std::endl;
	double dynamic_perf = benchmark(dynamic_coll, duration, num_producers,
			num_consumers);
	std::cout << "size at exit: " << dynamic_coll.size() << std::endl
			<< std::endl;

	std::streamsize prec = std::cout.precision(3);
	std::cout << "Reference performance: " << static_cast<int>(1e9/ref_perf)
			<< " ns per item " << std::endl;
	std::cout << "     Fixed size stack: " << static_cast<int>(1e9/fixed_perf)
			<< " ns per item " << fixed_perf / ref_perf
			<< " of reference performance" << std::endl;
	std::cout << "   Dynamic size stack: " << static_cast<int>(1e9/dynamic_perf)
			<< " ns per item " << dynamic_perf / ref_perf
			<< " of reference performance " << std::endl;
	std::cout << std::endl;
	std::cout.precision(prec);

}
