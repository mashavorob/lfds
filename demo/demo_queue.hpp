/*
 * demo_queue.hpp
 *
 *  Created on: Dec 16, 2014
 *      Author: masha
 */

#ifndef DEMO_MP_MC_QUEUE_HPP_
#define DEMO_MP_MC_QUEUE_HPP_

void DemoQueues(bool ManyProducers, bool ManyConsumers);

void ProfileQueue(bool FixedSize, bool ManyProducers, bool ManyConsumers);

void WaitFreeVsLockFreeQueues();

#endif /* DEMO_MP_MC_QUEUE_HPP_ */
