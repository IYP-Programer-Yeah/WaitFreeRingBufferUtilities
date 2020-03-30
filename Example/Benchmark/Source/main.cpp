#include <Iyp/WaitFreeRingBufferUtilities/wait-free-ring-buffer-utilities.inl>

#include <vector>
#include <thread>
#include <atomic>
#include <iostream>
#include <chrono>
#include <cstdlib>

int main()
{
	constexpr static std::size_t RingSize = 4096;
	using RingBufferType = Iyp::WaitFreeRingBufferUtilities::RingBuffer<std::size_t,
																		Iyp::WaitFreeRingBufferUtilities::AccessRequirements::MULTI_CONSUMER |
																			Iyp::WaitFreeRingBufferUtilities::AccessRequirements::MULTI_PRODUCER,
																		RingSize>;
	constexpr std::size_t PushCount = 4096 * RingSize;
	constexpr std::size_t NumberOfPusherThreads = 4;
	constexpr std::size_t NumberOfPopperThreads = 4;

	std::vector<std::thread> pushers;
	std::vector<std::thread> poppers;
	RingBufferType ring;

	for (std::size_t thread_number = 0; thread_number < NumberOfPopperThreads; thread_number++)
	{
		poppers.emplace_back([&ring, PushCount, NumberOfPusherThreads, NumberOfPopperThreads]() {
			for (std::size_t i = 0; i < PushCount * NumberOfPusherThreads / NumberOfPopperThreads; i++)
				while (!ring.pop())
				{
				}
		});
	}

	std::atomic<bool> do_push;

	for (std::size_t thread_number = 0; thread_number < NumberOfPusherThreads; thread_number++)
	{
		pushers.emplace_back([&ring, &do_push, PushCount]() mutable {
			while (!do_push.load(std::memory_order_relaxed))
			{
			}

			for (std::size_t i = 0; i < PushCount; i++)
				while (!ring.push(i))
				{
				}
		});
	}

	const auto start_time = std::chrono::steady_clock::now();
	do_push.store(true, std::memory_order_relaxed);

	for (auto &popper : poppers)
		popper.join();

	const auto end_time = std::chrono::steady_clock::now();

	std::cout << "Processed " << PushCount * NumberOfPusherThreads << " elements in ";
	std::cout << std::chrono::nanoseconds{end_time - start_time}.count() << " nano-seconds." << std::endl;

	for (auto &pusher : pushers)
		pusher.join();
}
