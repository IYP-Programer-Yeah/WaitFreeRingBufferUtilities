#include <Iyp/WaitFreeRingBufferUtilities/wait-free-ring-buffer-utilities.inl>
#include <gtest/gtest.h>

#include <array>
#include <thread>
#include <atomic>

namespace Iyp
{
namespace SingleProducerSingleConsumerRingBufferTest
{

constexpr static std::size_t RingSize = 4096;
using TestRingBufferType = WaitFreeRingBufferUtilities::RingBuffer<std::size_t,
                                                                   WaitFreeRingBufferUtilities::AccessRequirements::SINGLE_CONSUMER |
                                                                       WaitFreeRingBufferUtilities::AccessRequirements::SINGLE_PRODUCER,
                                                                   RingSize>;

TEST(SingleProducerSingleConsumerRingBufferTest, EmptyAndFullRingTest)
{
    constexpr std::size_t NumberOfTries = 256;

    TestRingBufferType ring;

    EXPECT_FALSE(ring.pop());

    for (std::size_t try_index = 0; try_index < NumberOfTries; try_index++)
    {
        for (std::size_t i = 0; i < RingSize; i++)
            EXPECT_TRUE(ring.push(0));

        EXPECT_FALSE(ring.push(0));

        for (std::size_t i = 0; i < RingSize; i++)
            EXPECT_TRUE(ring.pop());

        EXPECT_FALSE(ring.pop());
    }
}

TEST(SingleProducerSingleConsumerRingBufferTest, PushPopIntegrity)
{
    constexpr std::size_t NumberOfTries = 256;

    TestRingBufferType ring;

    for (std::size_t try_index = 0; try_index < NumberOfTries; try_index++)
    {
        for (std::size_t i = 0; i < RingSize; i++)
            ring.push(i);

        std::array<bool, RingSize> was_popped{false};

        for (std::size_t i = 0; i < RingSize; i++)
        {
            const auto pop_result = ring.pop();
            was_popped[*pop_result] = true;
        }

        for (std::size_t i = 0; i < RingSize; i++)
            EXPECT_TRUE(was_popped[i]);
    }
}

TEST(SingleProducerSingleConsumerRingBufferTest, OrderedPushPop)
{
    constexpr std::size_t NumberOfTries = 256;

    TestRingBufferType ring;

    for (std::size_t try_index = 0; try_index < NumberOfTries; try_index++)
    {
        for (std::size_t i = 0; i < RingSize; i++)
            ring.push(i);

        for (std::size_t i = 0; i < RingSize; i++)
        {
            const auto pop_result = ring.pop();
            EXPECT_EQ(*pop_result, i);
        }
    }
}

TEST(SingleProducerSingleConsumerRingBufferTest, SingleProducerSingleConsumerPushPopIntergrity)
{
    constexpr std::size_t NumberOfTries = 256;

    TestRingBufferType ring;

    std::array<std::atomic_size_t, RingSize> pop_counts;

    for (auto &pop_count : pop_counts)
        pop_count = 0;

    std::thread popper([&ring, &pop_counts, NumberOfTries]() {
        for (std::size_t try_index = 0; try_index < NumberOfTries; try_index++)
        {
            for (std::size_t i = 0; i < RingSize; i++)
            {
                while (true)
                {
                    const auto popped_value = ring.pop();
                    if (popped_value)
                    {
                        pop_counts[*popped_value].fetch_add(1, std::memory_order_relaxed);
                        break;
                    }
                }
            }
        }
    });

    for (std::size_t try_index = 0; try_index < NumberOfTries; try_index++)
    {
        for (std::size_t i = 0; i < RingSize; i++)
        {
            while (!ring.push(i))
            {
            }
        }
    }

    popper.join();

    for (const auto &pop_count : pop_counts)
        EXPECT_EQ(pop_count, NumberOfTries);
}
} // namespace SingleProducerSingleConsumerRingBufferTest
} // namespace Iyp
