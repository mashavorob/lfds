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
#include "virtualbenchmark.hpp"


const char* usage =
		"Use: demo <parameter1> [parameter2]...\n"
		"where parameterN is one of:\n"
		"--demo-queue                  run all demos for queues\n"
		"--demo-queue-mp               run all demos for many producers queues\n"
		"--demo-queue-mc               run all demos for many consumers queues\n"
		"--demo-queue-mp-mc            run demo for many producers many consumers queues\n"
		"--demo-queue-mp-sc            run demo for many producers single consumer queues\n"
		"--demo-queue-sp-mc            run demo for single producer many consumers queues\n"
		"--demo-queue-mp-sc            run demo for single producer single consumer queues\n"
		"--demo-wait-free-queue        run demo for wait free queue\n"
		"--demo-stack                  run demo for lock free stack\n"
		"--profile-queue-fixed-mp-mc   run profile for fixed size many producers many consumers queue\n"
		"--profile-queue-fixed-mp-sc   run profile for fixed size many producers single consumer queue\n"
		"--profile-queue-fixed-sp-mc   run profile for fixed size single producer many consumers queue\n"
		"--profile-queue-fixed-sp-sc   run profile for fixed size single producer single consumer queue\n"
		"--profile-queue-dyn-mp-mc     run profile for dynamic size many producers many consumers queue\n"
		"--profile-queue-dyn-mp-sc     run profile for dynamic size many producers single consumer queue\n"
		"--profile-queue-dyn-sp-mc     run profile for dynamic size single producer many consumers queue\n"
		"--profile-queue-dyn-sp-sc     run profile for dynamic size single producer single consumer queue\n"
		"--profile-cas                 run profile for compare and swap operations\n"
		"--profile-virtual-func        run profile for virtual function\n";

int main(int argc, char** argv)
{
	std::cout << "Lock Free Data Structures Demo v"
			<< LFDS_VERSION_MAJOR  << "." << LFDS_VERSION_MINOR << std::endl << std::endl;


	static const int FixedSize = 0x1;
	static const int ManyProducers = 0x2;
	static const int ManyConsumers = 0x4;


	bool doCASBenchmark = false;
	bool doWaitFreeQueuesDemo = false;
	bool doDemoStack = false;
	bool doVirtualFunc = false;
	std::set<int>	queueDemos;
	std::set<int>	queueProfiles;

	bool showCommandLineOptions = (argc == 1);
	int exitCode = 0;

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
		} else if ( strcmp(argv[i], "--demo-wait-free-queue") == 0 ) {
			doWaitFreeQueuesDemo = true;
		} else if ( strcmp(argv[i], "--demo-stack") == 0 ) {
			doDemoStack = true;
		} else if ( strcmp(argv[i], "--profile-virtual-func") == 0 ) {
			doVirtualFunc = true;
		} else if (strcmp(argv[i], "--help") == 0 ) {
			showCommandLineOptions = true;
		} else if (strcmp(argv[i], "-?") == 0 ) {
			showCommandLineOptions = true;
		} else if (strcmp(argv[i], "/?") == 0 ) {
			showCommandLineOptions = true;
	    } else {
			std::cerr << "Unknown command line option: " << argv[i] << std::endl;
			showCommandLineOptions = true;
			exitCode = 1;
		}
	}
	if ( showCommandLineOptions ) {
		std::cout << usage << std::endl;
		return exitCode;
	}


	if ( doCASBenchmark ) {
		CASBenchMark();
	}
	if ( doVirtualFunc ) {
		BenchmarkVirtualFunction();
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
	if ( doDemoStack ) {
		DemoStack();
	}
	return 0;
}

