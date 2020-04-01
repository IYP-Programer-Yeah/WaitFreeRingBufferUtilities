#include <Iyp/WaitFreeRingBufferUtilities/wait-free-ring-buffer-utilities.inl>

#include <vector>
#include <thread>
#include <atomic>
#include <iostream>
#include <chrono>
#include <cstdlib>
#include <memory>
#include <cstdint>

#include <rte_ring.h>
#include <rte_eal.h>
#include <boost/optional/optional.hpp>

#include <pthread.h>

template <std::size_t Count>
struct RteRingWrapperBase
{
	std::unique_ptr<rte_ring, decltype(&rte_ring_free)> ring;
	RteRingWrapperBase() : ring(rte_ring_create("Test ring", Count, SOCKET_ID_ANY, 0), &rte_ring_free)
	{
	}
};

template <std::size_t Count, unsigned>
struct RteRingWrapper;

template <std::size_t Count>
struct RteRingWrapper<Count, 0> : RteRingWrapperBase<Count>
{
	boost::optional<void *> pop()
	{
		void *value;
		if (!rte_ring_mc_dequeue(this->ring.get(), &value))
			return boost::optional<void *>{value};

		return boost::optional<void *>{};
	}

	bool push(void *const value)
	{
		return !rte_ring_mp_enqueue(this->ring.get(), value);
	}
};

template <std::size_t Count>
struct RteRingWrapper<Count, RING_F_SP_ENQ> : RteRingWrapperBase<Count>
{
	boost::optional<void *> pop()
	{
		void *value;
		if (!rte_ring_mc_dequeue(this->ring.get(), &value))
			return boost::optional<void *>{value};

		return boost::optional<void *>{};
	}

	bool push(void *const value)
	{
		return !rte_ring_sp_enqueue(this->ring.get(), value);
	}
};

template <std::size_t Count>
struct RteRingWrapper<Count, RING_F_SC_DEQ> : RteRingWrapperBase<Count>
{
	boost::optional<void *> pop()
	{
		void *value;
		if (!rte_ring_sc_dequeue(this->ring.get(), &value))
			return boost::optional<void *>{value};

		return boost::optional<void *>{};
	}

	bool push(void *const value)
	{
		return !rte_ring_mp_enqueue(this->ring.get(), value);
	}
};

template <std::size_t Count>
struct RteRingWrapper<Count, (RING_F_SP_ENQ | RING_F_SC_DEQ)> : RteRingWrapperBase<Count>
{
	boost::optional<void *> pop()
	{
		void *value;
		if (!rte_ring_sc_dequeue(this->ring.get(), &value))
			return boost::optional<void *>{value};

		return boost::optional<void *>{};
	}

	bool push(void *const value)
	{
		return !rte_ring_sp_enqueue(this->ring.get(), value);
	}
};

template <std::size_t NumberOfPusherThreads, std::size_t NumberOfPopperThreads, typename RingType>
std::pair<std::size_t, std::chrono::nanoseconds> run_benchmark(RingType &ring)
{
	constexpr std::size_t PushCount = 1024 * 1024;

	std::vector<std::thread> pushers;
	std::vector<std::thread> poppers;

	for (std::size_t thread_number = 0; thread_number < NumberOfPopperThreads; thread_number++)
	{
		poppers.emplace_back([&ring, PushCount]() {
			for (std::size_t i = 0; i < PushCount * NumberOfPusherThreads;)
				if (ring.pop())
					i++;
		});
		cpu_set_t cpuset;
		CPU_ZERO(&cpuset);
		CPU_SET(thread_number, &cpuset);
		pthread_setaffinity_np(poppers.back().native_handle(),
							   sizeof(cpu_set_t), &cpuset);
	}

	std::atomic<bool> do_push;

	for (std::size_t thread_number = 0; thread_number < NumberOfPusherThreads; thread_number++)
	{
		pushers.emplace_back([&ring, &do_push, PushCount]() mutable {
			while (!do_push.load(std::memory_order_relaxed))
			{
			}

			for (std::size_t i = 0; i < PushCount * NumberOfPopperThreads;)
				if (ring.push(reinterpret_cast<void *>(static_cast<std::uintptr_t>(i))))
					i++;
		});
		cpu_set_t cpuset;
		CPU_ZERO(&cpuset);
		CPU_SET(thread_number + NumberOfPopperThreads, &cpuset);
		pthread_setaffinity_np(pushers.back().native_handle(),
							   sizeof(cpu_set_t), &cpuset);
	}

	const auto start_time = std::chrono::steady_clock::now();
	do_push.store(true, std::memory_order_relaxed);

	for (auto &popper : poppers)
		popper.join();

	const auto end_time = std::chrono::steady_clock::now();

	for (auto &pusher : pushers)
		pusher.join();

	return std::make_pair(PushCount * NumberOfPopperThreads * NumberOfPusherThreads,
						  std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time));
}

template <std::size_t NumberOfPusherThreads, std::size_t NumberOfPopperThreads, std::size_t NumberOfTries, typename RingType>
void run_benchmark_and_print_results(RingType &ring)
{
	std::cout << "Running benchmark with " << NumberOfPusherThreads
			  << " pushers, and " << NumberOfPopperThreads << " poppers." << std::endl;

	for (std::size_t i = 0; i < NumberOfTries; i++)
	{
		const auto benchmark_result = run_benchmark<NumberOfPusherThreads, NumberOfPopperThreads>(ring);

		std::cout << "Processed " << benchmark_result.first << " elements in "
				  << benchmark_result.second.count() << " nano-seconds." << std::endl;
	}
}

int main(int argc, char **argv)
{
	rte_eal_init(argc, argv);
	std::cout << "EAL initialized." << std::endl;

	constexpr static std::size_t RingSize = 4096;

	{
		std::cout << "Running benchmark on MCMP queue." << std::endl;
		using RingBufferType = Iyp::WaitFreeRingBufferUtilities::RingBuffer<void *,
																			Iyp::WaitFreeRingBufferUtilities::AccessRequirements::MULTI_CONSUMER |
																				Iyp::WaitFreeRingBufferUtilities::AccessRequirements::MULTI_PRODUCER,
																			RingSize>;

		RteRingWrapper<RingSize, 0> rte_ring;
		std::cout << "Running benchmark on rte_ring." << std::endl;
		run_benchmark_and_print_results<1, 1, 16>(rte_ring);
		run_benchmark_and_print_results<2, 2, 16>(rte_ring);
		run_benchmark_and_print_results<3, 3, 16>(rte_ring);
		run_benchmark_and_print_results<4, 4, 16>(rte_ring);

		RingBufferType iyp_ring;
		std::cout << "Running benchmark on IYP ring." << std::endl;
		run_benchmark_and_print_results<1, 1, 16>(iyp_ring);
		run_benchmark_and_print_results<2, 2, 16>(iyp_ring);
		run_benchmark_and_print_results<3, 3, 16>(iyp_ring);
		run_benchmark_and_print_results<4, 4, 16>(iyp_ring);
	}

	{
		std::cout << "Running benchmark on SCMP queue." << std::endl;
		using RingBufferType = Iyp::WaitFreeRingBufferUtilities::RingBuffer<void *,
																			Iyp::WaitFreeRingBufferUtilities::AccessRequirements::SINGLE_CONSUMER |
																				Iyp::WaitFreeRingBufferUtilities::AccessRequirements::MULTI_PRODUCER,
																			RingSize>;

		RteRingWrapper<RingSize, RING_F_SC_DEQ> rte_ring;
		std::cout << "Running benchmark on rte_ring." << std::endl;
		run_benchmark_and_print_results<1, 1, 16>(rte_ring);
		run_benchmark_and_print_results<2, 1, 16>(rte_ring);
		run_benchmark_and_print_results<3, 1, 16>(rte_ring);
		run_benchmark_and_print_results<4, 1, 16>(rte_ring);
		run_benchmark_and_print_results<5, 1, 16>(rte_ring);
		run_benchmark_and_print_results<6, 1, 16>(rte_ring);
		run_benchmark_and_print_results<7, 1, 16>(rte_ring);

		RingBufferType iyp_ring;
		std::cout << "Running benchmark on IYP ring." << std::endl;
		run_benchmark_and_print_results<1, 1, 16>(iyp_ring);
		run_benchmark_and_print_results<2, 1, 16>(iyp_ring);
		run_benchmark_and_print_results<3, 1, 16>(iyp_ring);
		run_benchmark_and_print_results<4, 1, 16>(iyp_ring);
		run_benchmark_and_print_results<5, 1, 16>(iyp_ring);
		run_benchmark_and_print_results<6, 1, 16>(iyp_ring);
		run_benchmark_and_print_results<7, 1, 16>(iyp_ring);
	}

	{
		std::cout << "Running benchmark on MCSP queue." << std::endl;
		using RingBufferType = Iyp::WaitFreeRingBufferUtilities::RingBuffer<void *,
																			Iyp::WaitFreeRingBufferUtilities::AccessRequirements::MULTI_CONSUMER |
																				Iyp::WaitFreeRingBufferUtilities::AccessRequirements::SINGLE_PRODUCER,
																			RingSize>;

		RteRingWrapper<RingSize, RING_F_SP_ENQ> rte_ring;
		std::cout << "Running benchmark on rte_ring." << std::endl;
		run_benchmark_and_print_results<1, 1, 16>(rte_ring);
		run_benchmark_and_print_results<1, 2, 16>(rte_ring);
		run_benchmark_and_print_results<1, 3, 16>(rte_ring);
		run_benchmark_and_print_results<1, 4, 16>(rte_ring);
		run_benchmark_and_print_results<1, 5, 16>(rte_ring);
		run_benchmark_and_print_results<1, 6, 16>(rte_ring);
		run_benchmark_and_print_results<1, 7, 16>(rte_ring);

		RingBufferType iyp_ring;
		std::cout << "Running benchmark on IYP ring." << std::endl;
		run_benchmark_and_print_results<1, 1, 16>(iyp_ring);
		run_benchmark_and_print_results<1, 2, 16>(iyp_ring);
		run_benchmark_and_print_results<1, 3, 16>(iyp_ring);
		run_benchmark_and_print_results<1, 4, 16>(iyp_ring);
		run_benchmark_and_print_results<1, 5, 16>(iyp_ring);
		run_benchmark_and_print_results<1, 6, 16>(iyp_ring);
		run_benchmark_and_print_results<1, 7, 16>(iyp_ring);
	}

	{
		std::cout << "Running benchmark on SCSP queue." << std::endl;
		using RingBufferType = Iyp::WaitFreeRingBufferUtilities::RingBuffer<void *,
																			Iyp::WaitFreeRingBufferUtilities::AccessRequirements::SINGLE_CONSUMER |
																				Iyp::WaitFreeRingBufferUtilities::AccessRequirements::SINGLE_PRODUCER,
																			RingSize>;

		RteRingWrapper<RingSize, RING_F_SC_DEQ | RING_F_SP_ENQ> rte_ring;
		std::cout << "Running benchmark on rte_ring." << std::endl;
		run_benchmark_and_print_results<1, 1, 16>(rte_ring);

		RingBufferType iyp_ring;
		std::cout << "Running benchmark on IYP ring." << std::endl;
		run_benchmark_and_print_results<1, 1, 16>(iyp_ring);
	}
}
