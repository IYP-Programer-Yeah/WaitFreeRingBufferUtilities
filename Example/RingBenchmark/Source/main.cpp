#include <Iyp/WaitFreeRingBufferUtilities/wait-free-ring-buffer-utilities.inl>

#include <benchmark/benchmark.h>

static constexpr std::size_t RingSize = 1024;

using McmpRingBufferType = Iyp::WaitFreeRingBufferUtilities::RingBuffer<std::size_t,
                                                                        Iyp::WaitFreeRingBufferUtilities::AccessRequirements::MULTI_CONSUMER |
                                                                            Iyp::WaitFreeRingBufferUtilities::AccessRequirements::MULTI_PRODUCER,
                                                                        RingSize>;

using ScmpRingBufferType = Iyp::WaitFreeRingBufferUtilities::RingBuffer<std::size_t,
                                                                        Iyp::WaitFreeRingBufferUtilities::AccessRequirements::SINGLE_CONSUMER |
                                                                            Iyp::WaitFreeRingBufferUtilities::AccessRequirements::MULTI_PRODUCER,
                                                                        RingSize>;

using McspRingBufferType = Iyp::WaitFreeRingBufferUtilities::RingBuffer<std::size_t,
                                                                        Iyp::WaitFreeRingBufferUtilities::AccessRequirements::MULTI_CONSUMER |
                                                                            Iyp::WaitFreeRingBufferUtilities::AccessRequirements::SINGLE_PRODUCER,
                                                                        RingSize>;

using ScspRingBufferType = Iyp::WaitFreeRingBufferUtilities::RingBuffer<std::size_t,
                                                                        Iyp::WaitFreeRingBufferUtilities::AccessRequirements::SINGLE_CONSUMER |
                                                                            Iyp::WaitFreeRingBufferUtilities::AccessRequirements::SINGLE_PRODUCER,
                                                                        RingSize>;

template <typename RingType>
struct RingFixture : benchmark::Fixture
{
    RingType ring;

    void SetUp(const ::benchmark::State &state)
    {
        if (state.thread_index == 0)
        {
            for (std::size_t i = 0; i < RingSize / 2; i++)
                ring.push(i);
        }
    }

    void TearDown(const ::benchmark::State &state)
    {
        if (state.thread_index == 0)
        {
            while (ring.pop())
                ;
        }
    }
};

BENCHMARK_TEMPLATE_DEFINE_F(RingFixture, scsp_multithreaded_benchmark, ScspRingBufferType)
(benchmark::State &state)
{
    if (state.thread_index == 0)
        for (auto _ : state)
            ring.push(0);
    else
        for (auto _ : state)
            ring.pop();
}
BENCHMARK_REGISTER_F(RingFixture, scsp_multithreaded_benchmark)->Threads(2);

BENCHMARK_TEMPLATE_DEFINE_F(RingFixture, mcsp_multithreaded_benchmark, McspRingBufferType)
(benchmark::State &state)
{
    if (state.thread_index == 0)
        for (auto _ : state)
            ring.push(0);
    else
        for (auto _ : state)
            ring.pop();
}
BENCHMARK_REGISTER_F(RingFixture, mcsp_multithreaded_benchmark)->Threads(8);

BENCHMARK_TEMPLATE_DEFINE_F(RingFixture, scmp_multithreaded_benchmark, ScmpRingBufferType)
(benchmark::State &state)
{
    if (state.thread_index == 0)
        for (auto _ : state)
            ring.pop();
    else
        for (auto _ : state)
            ring.push();
}
BENCHMARK_REGISTER_F(RingFixture, scmp_multithreaded_benchmark)->Threads(8);

BENCHMARK_TEMPLATE_DEFINE_F(RingFixture, mcmp_multithreaded_benchmark, McmpRingBufferType)
(benchmark::State &state)
{
    if (state.thread_index < (state.threads / 2))
        for (auto _ : state)
            ring.push(0);
    else
        for (auto _ : state)
            ring.pop();
}
BENCHMARK_REGISTER_F(RingFixture, mcmp_multithreaded_benchmark)->Threads(8);