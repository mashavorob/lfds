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

void ManyProducersManyConsumersStack()
{
	std::cout << "Benchmarking of many producers many consumers stacks: "
			<< std::endl << std::endl;

	typedef std_stack_wrapper< std::stack<int> > ref_collection_type;
	typedef lfds::stack<int, true>				 fixed_collection_type;
	typedef lfds::stack<int, false>				 dynamic_collection_type;

	typedef counted_collection_wrapper< ref_collection_type > 		ref_counted_collection_type;
	typedef counted_collection_wrapper< fixed_collection_type > 	fixed_counted_collection_type;
	typedef counted_collection_wrapper< dynamic_collection_type > 	dynamic_counted_collection_type;

	typedef std::chrono::seconds seconds_type;

	static const seconds_type	duration(10);
	static const std::size_t	capacity = 100;
	static const unsigned		numproducers = 2;
	static const unsigned		numconsumers = 2;

	ref_counted_collection_type 	ref_coll(capacity);
	fixed_counted_collection_type 	fixed_coll(capacity);
	dynamic_counted_collection_type dynamic_coll(capacity);

	std::cout << "std::stack:" << std::endl;
	std::size_t ref_perf = benchmark(ref_coll, duration, numproducers, numconsumers);
	std::cout << std::endl;

	std::cout << "lock free fixed size stack:" << std::endl;
	std::size_t fixed_perf = benchmark(fixed_coll, duration, numproducers, numconsumers);
	std::cout << std::endl;

	std::cout << "lock free dynamic size stack:" << std::endl;
	std::size_t dynamic_perf = benchmark(dynamic_coll, duration, numproducers, 5);
	std::cout << std::endl;

	std::cout << "Reference performance: " << ref_perf << std::endl;
	std::cout << "     Fixed size stack: " << fixed_perf << " "
			<< static_cast<double>(fixed_perf)/static_cast<double>(ref_perf)
			<< " of reference performance" << std::endl;
	std::cout << "     Dynamic size stack: " << dynamic_perf << " "
			<< static_cast<double>(dynamic_perf)/static_cast<double>(ref_perf)
			<< " of reference performance" << std::endl;
}



