/*
 * demo.cpp
 *
 *  Created on: Dec 16, 2014
 *      Author: masha
 */


#include <iostream>
#include <set>
#include <string.h>

#include "config.hpp"
#include "demo_stack.hpp"
#include "demo_queue.hpp"
#include "casbenchmark.hpp"

int main(int argc, char** argv)
{
	std::cout << "Lock Free Data Structures Demo v"
			<< LFDS_VERSION_MAJOR  << "." << LFDS_VERSION_MINOR << std::endl << std::endl;


	static const int FixedSize = 0x1;
	static const int ManyProducers = 0x2;
	static const int ManyConsumers = 0x4;


	bool doCASBenchmark = false;
	bool doWaitFreeQueuesDemo = false;
	std::set<int>	queueDemos;
	std::set<int>	queueProfiles;

	for ( int i = 1; i < argc; ++i )
	{
		if ( strcmp(argv[i], "--demo-queue") == 0 ) {
			queueDemos.insert(ManyProducers | ManyConsumers);
			queueDemos.insert(ManyProducers);
			queueDemos.insert(ManyConsumers);
			queueDemos.insert(0);
			doWaitFreeQueuesDemo = true;
		} else if ( strcmp(argv[i], "--demo-queue-mp") == 0 ) {
			queueDemos.insert(ManyProducers | ManyConsumers);
			queueDemos.insert(ManyProducers);
		} else if ( strcmp(argv[i], "--demo-queue-mc") == 0 ) {
			queueDemos.insert(ManyProducers | ManyConsumers);
			queueDemos.insert(ManyConsumers);
		} else if ( strcmp(argv[i], "--demo-queue-mp-mc") == 0 ) {
			queueDemos.insert(ManyProducers | ManyConsumers);
		} else if ( strcmp(argv[i], "--demo-queue-mp-sc") == 0 ) {
			queueDemos.insert(ManyProducers);
		} else if ( strcmp(argv[i], "--demo-queue-sp-mc") == 0 ) {
			queueDemos.insert(ManyConsumers);
		} else if ( strcmp(argv[i], "--demo-queue-sp-sc") == 0 ) {
			queueDemos.insert(0);
		} else if ( strcmp(argv[i], "--profile-queue-fixed-mp-mc") == 0 ) {
			queueProfiles.insert(FixedSize | ManyProducers | ManyConsumers);
		} else if ( strcmp(argv[i], "--profile-queue-fixed-mp-sc") == 0 ) {
			queueProfiles.insert(FixedSize | ManyProducers);
		} else if ( strcmp(argv[i], "--profile-queue-fixed-sp-mc") == 0 ) {
			queueProfiles.insert(FixedSize | ManyConsumers);
		} else if ( strcmp(argv[i], "--profile-queue-fixed-sp-sc") == 0 ) {
			queueProfiles.insert(FixedSize);
		} else if ( strcmp(argv[i], "--profile-queue-dyn-mp-mc") == 0 ) {
			queueProfiles.insert(ManyProducers | ManyConsumers);
		} else if ( strcmp(argv[i], "--profile-queue-dyn-mp-sc") == 0 ) {
			queueProfiles.insert(ManyProducers);
		} else if ( strcmp(argv[i], "--profile-queue-dyn-sp-mc") == 0 ) {
			queueProfiles.insert(ManyConsumers);
		} else if ( strcmp(argv[i], "--profile-queue-dyn-sp-sc") == 0 ) {
			queueProfiles.insert(0);
		} else if ( strcmp(argv[i], "--profile-cas") == 0 ) {
			doCASBenchmark = true;
		} else if ( strcmp(argv[i], "--wait-free-queue") == 0 ) {
			doWaitFreeQueuesDemo = true;
		}
	}

	if ( doCASBenchmark ) {
		CASBenchMark();
	}
	if ( doWaitFreeQueuesDemo ) {
		WaitFreeVsLockFreeQueues();
	}
	for ( int mask : queueDemos ) {
		DemoQueues((mask & ManyProducers) != 0, (mask & ManyConsumers) != 0);
	}
	for ( int mask : queueProfiles ) {
		ProfileQueue((mask & FixedSize) != 0, (mask & ManyProducers) != 0, (mask & ManyConsumers) != 0);
	}
}
