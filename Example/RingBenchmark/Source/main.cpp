#include <Iyp/WaitFreeRingBufferUtilities/wait-free-ring-buffer-utilities.inl>

#include <benchmark/benchmark.h>

#include <cstddef>
#include <cstdlib>
#include <ctime>
#include <list>
#include <atomic>
#include <thread>
#include <cstdint>
#include <utility>

static constexpr std::size_t RingSize = 1024;

using McmpRingBufferType = Iyp::WaitFreeRingBufferUtilities::RingBuffer<Iyp::WaitFreeRingBufferUtilities::MultiProducer,
                                                                        Iyp::WaitFreeRingBufferUtilities::MultiConsumer,
                                                                        std::size_t,
                                                                        RingSize>;

using ScmpRingBufferType = Iyp::WaitFreeRingBufferUtilities::RingBuffer<Iyp::WaitFreeRingBufferUtilities::MultiProducer,
                                                                        Iyp::WaitFreeRingBufferUtilities::SingleConsumer,
                                                                        std::size_t,
                                                                        RingSize>;

using McspRingBufferType = Iyp::WaitFreeRingBufferUtilities::RingBuffer<Iyp::WaitFreeRingBufferUtilities::SingleProducer,
                                                                        Iyp::WaitFreeRingBufferUtilities::MultiConsumer,
                                                                        std::size_t,
                                                                        RingSize>;

using ScspRingBufferType = Iyp::WaitFreeRingBufferUtilities::RingBuffer<Iyp::WaitFreeRingBufferUtilities::SingleProducer,
                                                                        Iyp::WaitFreeRingBufferUtilities::SingleConsumer,
                                                                        std::size_t,
                                                                        RingSize>;

enum class ThreadType
{
    PRODUCER,
    CONSUMER,
};

template <typename RingType>
class Thread
{
    enum : std::uint8_t
    {
        START_ITERATION,
        ENDED_ITERATION,
    };

    RingType &ring;
    std::atomic<std::uint8_t> signal;
    std::atomic<bool> should_stop;
    std::size_t number_of_processed_elements_per_iteration;
    std::thread thread;

    void producer_thread()
    {
        while (!should_stop)
            if (signal == START_ITERATION)
            {
                for (std::size_t i = 0; i < number_of_processed_elements_per_iteration; i++)
                    while (!ring.push(i))
                    {
                    }

                signal = ENDED_ITERATION;
            }
    }

    void consumer_thread()
    {
        while (!should_stop)
            if (signal == START_ITERATION)
            {
                for (std::size_t i = 0; i < number_of_processed_elements_per_iteration; i++)
                    while (!ring.pop())
                    {
                    }

                signal = ENDED_ITERATION;
            }
    }

public:
    Thread(RingType &i_ring,
           const ThreadType thread_type,
           const std::size_t i_number_of_processed_elements_per_iteration)
        : ring(i_ring),
          signal(ENDED_ITERATION),
          should_stop(false),
          number_of_processed_elements_per_iteration(i_number_of_processed_elements_per_iteration),
          thread([this, thread_type]() { thread_type == ThreadType::PRODUCER ? producer_thread() : consumer_thread(); })
    {
    }

    void run_an_iteration()
    {
        signal = START_ITERATION;
    }

    void wait_for_iteration_to_end()
    {
        while (signal != ENDED_ITERATION)
        {
        }
    }

    ~Thread()
    {
        should_stop = true;
        while (!thread.joinable())
        {
        }
        thread.join();
    }
};

template <typename RingType>
void throughput_benchmark(benchmark::State &state)
{
    constexpr std::size_t NumberOfProcessedElementsPerIteration = RingSize * 8;
    RingType ring;
    for (std::size_t i = 0; i < RingSize / 2; i++)
        ring.push(i);

    std::list<Thread<RingType>> threads;

    for (std::size_t i = 0; i < state.range(0); i++)
        threads.emplace_back(ring, ThreadType::PRODUCER, NumberOfProcessedElementsPerIteration * state.range(1));

    for (std::size_t i = 0; i < state.range(1); i++)
        threads.emplace_back(ring, ThreadType::CONSUMER, NumberOfProcessedElementsPerIteration * state.range(0));

    for (auto _ : state)
    {
        for (auto &thread : threads)
            thread.run_an_iteration();
        for (auto &thread : threads)
            thread.wait_for_iteration_to_end();
    }

    state.SetComplexityN((state.range(0) + state.range(1)) * NumberOfProcessedElementsPerIteration * state.range(0) * state.range(1));
}

BENCHMARK_TEMPLATE(throughput_benchmark, ScmpRingBufferType)->ArgsProduct({{1, 2, 3, 4, 5, 6, 7}, {1}})->ArgNames({"Producer Count", "Consumer Count"})->Complexity();
BENCHMARK_TEMPLATE(throughput_benchmark, McspRingBufferType)->ArgsProduct({{1}, {1, 2, 3, 4, 5, 6, 7}})->ArgNames({"Producer Count", "Consumer Count"})->Complexity();
BENCHMARK_TEMPLATE(throughput_benchmark, McmpRingBufferType)->ArgsProduct({{1, 2, 3, 4}, {1, 2, 3, 4}})->ArgNames({"Producer Count", "Consumer Count"})->Complexity();
BENCHMARK_TEMPLATE(throughput_benchmark, ScspRingBufferType)->ArgsProduct({{1}, {1}})->ArgNames({"Producer Count", "Consumer Count"});
