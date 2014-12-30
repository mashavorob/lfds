/*
 * demo_queue.cpp
 *
 *  Created on: Dec 16, 2014
 *      Author: masha
 */

#include <iostream>
#include <queue>
#include <chrono>

#include "demo_queue.hpp"
#include "counted_collection_wrapper.hpp"
#include "std_queue_wrapper.hpp"
#include "benchmark.hpp"
#include "queue.hpp"

template<bool fixedSize>
struct QueueCaptionImpl {
	const char* operator()();
};

template<>
struct QueueCaptionImpl<true> {
	const char* operator()() {
		return "fixed size lock free queue";
	}
};

template<>
struct QueueCaptionImpl<false> {
	const char* operator()() {
		return "dynamic size lock free queue";
	}
};

template<bool fixedSize>
struct QueueRunnerImpl {
	typedef lfds::queue<int, fixedSize> collection_type;
	typedef counted_collection_wrapper<collection_type> counted_collection_type;

	std::size_t operator()() {
		typedef std::chrono::seconds seconds_type;

		static const seconds_type duration(120);
		static const std::size_t capacity = 100;
		static const unsigned numproducers = 2;
		static const unsigned numconsumers = 2;

		counted_collection_type coll(capacity);

		std::cout << "Profiling of " << QueueCaptionImpl<fixedSize>()() << ":"
				<< std::endl;
		std::size_t perf = benchmark(coll, duration, numproducers,
				numconsumers);
		std::cout << std::endl;
		std::cout << "    performance counter: " << perf << std::endl;
		std::cout << "collection size at exit: " << coll.size() << std::endl
				<< std::endl;

		return perf;
	}

};

void ProfileQueue(bool fixedSize) {
	if (fixedSize) {
		QueueRunnerImpl<true>()();
	} else {
		QueueRunnerImpl<false>()();
	}
}

template<bool ManyProducers, bool ManyConsumers>
class QueueDemonstrator {
public:
	typedef std_queue_wrapper<std::queue<int> > ref_collection_type;
	typedef lfds::queue<int, true, ManyProducers, ManyConsumers> fixed_collection_type;
	typedef lfds::queue<int, false, ManyProducers, ManyConsumers> dynamic_collection_type;

public:
	static const unsigned duration_seconds = 10;
	static const unsigned profile_duration_seconds = 10;
	static const unsigned num_producers = ManyProducers ? 2 : 1;
	static const unsigned num_consumers = ManyConsumers ? 2 : 1;
	static const std::size_t capacity = 100;

public:
	static void demonstrate() {
		const std::chrono::seconds duration(duration_seconds);

		ref_collection_type ref_coll(capacity);
		fixed_collection_type fixed_coll(capacity);
		dynamic_collection_type dynamic_coll(capacity);

		std::cout << "std::queue:" << std::endl;
		double ref_perf = benchmark(ref_coll, duration, num_producers,
				num_consumers);
		std::cout << "size at exit: " << ref_coll.size() << std::endl
				<< std::endl;

		static const char* prefix[] = { "lock free", "wait free" };

		std::cout << prefix[fixed_collection_type::wait_free] << " fixed size queue:" << std::endl;
		double fixed_perf = benchmark(fixed_coll, duration, num_producers,
				num_consumers);
		std::cout << "size at exit: " << fixed_coll.size() << std::endl
				<< std::endl;

		std::cout << prefix[dynamic_collection_type::wait_free] << " dynamic size queue:" << std::endl;
		double dynamic_perf = benchmark(dynamic_coll, duration, num_producers,
				num_consumers);
		std::cout << "size at exit: " << dynamic_coll.size() << std::endl
				<< std::endl;

		std::streamsize prec = std::cout.precision(3);
		std::cout << "Reference performance: " << static_cast<int>(1e9/ref_perf)
				<< " ns per item " << std::endl;
		std::cout << "     Fixed size queue: " << static_cast<int>(1e9/fixed_perf)
				<< " ns per item " << fixed_perf / ref_perf
				<< " of reference performance" << std::endl;
		std::cout << "   Dynamic size queue: " << static_cast<int>(1e9/dynamic_perf)
				<< " ns per item " << dynamic_perf / ref_perf
				<< " of reference performance " << std::endl;
		std::cout << std::endl;
		std::cout.precision(prec);
	}

	static void profile(bool fixedSize, const unsigned numproducers =
			num_producers, const unsigned numconsumers = num_consumers) {
		static const std::chrono::seconds duration(profile_duration_seconds);

		fixed_collection_type fixed_coll(capacity);
		dynamic_collection_type dynamic_coll(capacity);

		static const char* prefix[] = { "Lock Free", "Wait Free" };
		static const char* pr[] = { "Single Producer", "Many Producers" };
		static const char* cs[] = { "Single Consumer", "Many Consumers" };
		static const char* ss[] = { "Dynamic Size", "Fixed Size" };

		double perf;
		if (fixedSize) {
			std::cout << "Profiling " << prefix[fixed_collection_type::wait_free] << " "
					<< pr[ManyProducers != false] << " "
					<< cs[ManyConsumers != false] << " " << ss[fixedSize != false]
					<< " queue" << std::endl;
			perf = benchmark(fixed_coll, duration, numproducers,
					numconsumers);
		} else {
			std::cout << "Profiling " << prefix[dynamic_collection_type::wait_free] << " "
					<< pr[ManyProducers != false] << " "
					<< cs[ManyConsumers != false] << " " << ss[fixedSize != false]
					<< " queue" << std::endl;
			perf = benchmark(dynamic_coll, duration, numproducers,
					numconsumers);
		}
		std::cout << "Finished: " << static_cast<int>(1e9/perf) << " ns per item" << std::endl;
	}
};

void ProfileQueue(bool FixedSize, bool ManyProducers, bool ManyConsumers) {
	if (ManyProducers && ManyConsumers) {
		QueueDemonstrator<true, true>::profile(FixedSize);
	} else if (ManyProducers && !ManyConsumers) {
		QueueDemonstrator<true, false>::profile(FixedSize);
	} else if (!ManyProducers && ManyConsumers) {
		QueueDemonstrator<false, true>::profile(FixedSize);
	} else {
		QueueDemonstrator<false, false>::profile(FixedSize);
	}
}

void DemoQueues(bool ManyProducers, bool ManyConsumers) {
	if (ManyProducers && ManyConsumers) {
		std::cout << "Many Producers Many Consumers:" << std::endl
				  << "------------------------------" << std::endl;
		QueueDemonstrator<true, true>::demonstrate();
	} else if (ManyProducers && !ManyConsumers) {
		std::cout << "Many Producers Single Consumer:" << std::endl
				  << "-------------------------------" << std::endl;
		QueueDemonstrator<true, false>::demonstrate();
	} else if (!ManyProducers && ManyConsumers) {
		std::cout << "Single Producer Many Consumers:" << std::endl
				  << "-------------------------------" << std::endl;
		QueueDemonstrator<false, true>::demonstrate();
	} else {
		std::cout << "Single Producer Single Consumer:" << std::endl
				  << "--------------------------------" << std::endl;
		QueueDemonstrator<false, false>::demonstrate();
	}
}

void WaitFreeVsLockFreeQueues() {
	std::cout << "Comparing wait free and lock free queues:"<< std::endl
			  << "-----------------------------------------" << std::endl;
	QueueDemonstrator<false, false>::profile(true, 1, 1); 	// wait free
	std::cout << std::endl;
	QueueDemonstrator<true, false>::profile(true, 1, 1);
	std::cout << std::endl;
	QueueDemonstrator<false, false>::profile(false, 1, 1);
	std::cout << std::endl;
	QueueDemonstrator<true, false>::profile(false, 1, 1);
	std::cout << std::endl;
}
