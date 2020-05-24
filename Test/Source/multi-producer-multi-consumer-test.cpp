#include <Iyp/WaitFreeRingBufferUtilities/wait-free-ring-buffer-utilities.inl>
#include <gtest/gtest.h>

#include <vector>
#include <array>
#include <thread>
#include <atomic>

namespace Iyp
{
namespace MultiProducerMultiConsumerRingBufferTest
{
static constexpr std::size_t RingSize = 4096;
static constexpr std::size_t NumberOfTries = 4096;
using TestRingBufferType = WaitFreeRingBufferUtilities::RingBuffer<std::size_t,
                                                                   WaitFreeRingBufferUtilities::AccessRequirements::MULTI_CONSUMER |
                                                                       WaitFreeRingBufferUtilities::AccessRequirements::MULTI_PRODUCER,
                                                                   RingSize>;

TEST(MultiProducerMultiConsumerRingBufferTest, EmptyAndFullRingTest)
{
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

TEST(MultiProducerMultiConsumerRingBufferTest, PushPopIntegrity)
{
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

TEST(MultiProducerMultiConsumerRingBufferTest, OrderedPushPop)
{
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

TEST(MultiProducerMultiConsumerRingBufferTest, MultiProducerMultiConsumerPushPopIntergrity)
{
    static constexpr std::size_t NumberOfPusherThreads = 4;
    static constexpr std::size_t NumberOfPopperThreads = 4;

    ASSERT_EQ(NumberOfTries % NumberOfPusherThreads, 0);
    ASSERT_EQ(NumberOfTries % NumberOfPopperThreads, 0);

    std::vector<std::thread> pushers;
    std::vector<std::thread> poppers;
    TestRingBufferType ring;

    std::array<std::atomic_size_t, RingSize> pop_counts;

    for (auto &pop_count : pop_counts)
        pop_count = 0;

    for (std::size_t thread_number = 0; thread_number < NumberOfPopperThreads; thread_number++)
    {
        poppers.emplace_back([&ring, &pop_counts]() {
            for (std::size_t try_index = 0; try_index < NumberOfTries / NumberOfPopperThreads; try_index++)
                for (std::size_t i = 0; i < RingSize;)
                {
                    const auto popped_value = ring.pop();
                    if (popped_value)
                    {
                        pop_counts[*popped_value].fetch_add(1, std::memory_order_relaxed);
                        i++;
                    }
                }
        });
    }

    for (std::size_t thread_number = 0; thread_number < NumberOfPusherThreads; thread_number++)
    {
        pushers.emplace_back([&ring]() {
            for (std::size_t try_index = 0; try_index < NumberOfTries / NumberOfPusherThreads; try_index++)
                for (std::size_t i = 0; i < RingSize;)
                    if (ring.push(i))
                        i++;
        });
    }

    for (auto &popper : poppers)
        popper.join();

    for (auto &pusher : pushers)
        pusher.join();

    for (const auto &pop_count : pop_counts)
        EXPECT_EQ(pop_count, NumberOfTries);
}
} // namespace MultiProducerMultiConsumerRingBufferTest
} // namespace Iyp
