#include <Iyp/WaitFreeRingBufferUtilities/wait-free-ring-buffer-utilities.inl>

#include <vector>
#include <thread>
#include <atomic>
#include <iostream>
#include <chrono>
#include <cstdlib>
#include <utility>

template <std::size_t NumberOfPusherThreads, std::size_t NumberOfPopperThreads, typename RingType>
std::pair<std::size_t, std::chrono::nanoseconds> run_benchmark(RingType &ring)
{
    static constexpr std::size_t PushCount = 1024 * 1024;

    std::vector<std::thread> pushers;
    std::vector<std::thread> poppers;

    for (std::size_t thread_number = 0; thread_number < NumberOfPopperThreads; thread_number++)
    {
        poppers.emplace_back([&ring]() {
            for (std::size_t i = 0; i < PushCount * NumberOfPusherThreads;)
                if (ring.pop())
                    i++;
        });
    }

    std::atomic<bool> do_push;

    for (std::size_t thread_number = 0; thread_number < NumberOfPusherThreads; thread_number++)
    {
        pushers.emplace_back([&ring, &do_push]() mutable {
            while (!do_push.load(std::memory_order_relaxed))
            {
            }

            for (std::size_t i = 0; i < PushCount * NumberOfPopperThreads;)
                if (ring.push(i))
                    i++;
        });
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

template <std::size_t NumberOfPusherThreads, std::size_t NumberOfPopperThreads, typename RingType>
void run_benchmark_and_print_results(RingType &ring)
{
    std::cout << "Running benchmark with " << NumberOfPusherThreads
              << " pushers, and " << NumberOfPopperThreads << " poppers." << std::endl;
    const auto benchmark_result = run_benchmark<NumberOfPusherThreads, NumberOfPopperThreads>(ring);

    std::cout << "Processed " << benchmark_result.first << " elements in "
              << benchmark_result.second.count() << " nano-seconds." << std::endl;
}

int main()
{
    static constexpr std::size_t RingSize = 1024;

    {
        std::cout << "Running benchmark on MCMP queue." << std::endl;
        using McmpRingBufferType = Iyp::WaitFreeRingBufferUtilities::RingBuffer<std::size_t,
                                                                                Iyp::WaitFreeRingBufferUtilities::AccessRequirements::MULTI_CONSUMER |
                                                                                    Iyp::WaitFreeRingBufferUtilities::AccessRequirements::MULTI_PRODUCER,
                                                                                RingSize>;
        McmpRingBufferType ring;
        run_benchmark_and_print_results<4, 4>(ring);
    }

    {
        std::cout << "Running benchmark on MCSP queue." << std::endl;
        using ScspRingBufferType = Iyp::WaitFreeRingBufferUtilities::RingBuffer<std::size_t,
                                                                                Iyp::WaitFreeRingBufferUtilities::AccessRequirements::MULTI_CONSUMER |
                                                                                    Iyp::WaitFreeRingBufferUtilities::AccessRequirements::SINGLE_PRODUCER,
                                                                                RingSize>;
        ScspRingBufferType ring;
        run_benchmark_and_print_results<1, 7>(ring);
    }

    {
        std::cout << "Running benchmark on SCMP queue." << std::endl;
        using ScspRingBufferType = Iyp::WaitFreeRingBufferUtilities::RingBuffer<std::size_t,
                                                                                Iyp::WaitFreeRingBufferUtilities::AccessRequirements::SINGLE_CONSUMER |
                                                                                    Iyp::WaitFreeRingBufferUtilities::AccessRequirements::MULTI_PRODUCER,
                                                                                RingSize>;
        ScspRingBufferType ring;
        run_benchmark_and_print_results<7, 1>(ring);
    }

    {
        std::cout << "Running benchmark on SCSP queue." << std::endl;
        using ScspRingBufferType = Iyp::WaitFreeRingBufferUtilities::RingBuffer<std::size_t,
                                                                                Iyp::WaitFreeRingBufferUtilities::AccessRequirements::SINGLE_CONSUMER |
                                                                                    Iyp::WaitFreeRingBufferUtilities::AccessRequirements::SINGLE_PRODUCER,
                                                                                RingSize>;
        ScspRingBufferType ring;
        run_benchmark_and_print_results<1, 1>(ring);
    }
}
