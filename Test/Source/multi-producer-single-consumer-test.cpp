#include <Iyp/WaitFreeRingBufferUtilities/wait-free-ring-buffer-utilities.inl>
#include <gtest/gtest.h>

#include <vector>
#include <array>
#include <thread>
#include <atomic>

namespace Iyp
{
namespace MultiProducerSingleConsumerRingBufferTest
{
static constexpr std::size_t RingSize = 4096;
static constexpr std::size_t NumberOfTries = 1024;
using TestRingBufferType = WaitFreeRingBufferUtilities::RingBuffer<WaitFreeRingBufferUtilities::MultiProducer,
                                                                   WaitFreeRingBufferUtilities::SingleConsumer,
                                                                   std::size_t,
                                                                   RingSize>;

TEST(MultiProducerSingleConsumerRingBufferTest, EmptyAndFullRingTest)
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

TEST(MultiProducerSingleConsumerRingBufferTest, PushPopIntegrity)
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

TEST(MultiProducerSingleConsumerRingBufferTest, OrderedPushPop)
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

TEST(MultiProducerSingleConsumerRingBufferTest, MultiProducerSingleConsumerPushPopIntergrity)
{
    static constexpr std::size_t NumberOfPusherThreads = 7;

    std::vector<std::thread> pushers;
    std::vector<std::thread> poppers;
    TestRingBufferType ring;

    std::array<std::size_t, RingSize> pop_counts;

    for (auto &pop_count : pop_counts)
        pop_count = 0;

    for (std::size_t thread_number = 0; thread_number < NumberOfPusherThreads; thread_number++)
    {
        pushers.emplace_back([&ring]() {
            for (std::size_t try_index = 0; try_index < NumberOfTries; try_index++)
                for (std::size_t i = 0; i < RingSize;)
                    if (ring.push(i))
                        i++;
        });
    }

    for (std::size_t try_index = 0; try_index < NumberOfTries * NumberOfPusherThreads; try_index++)
        for (std::size_t i = 0; i < RingSize;)
        {
            const auto popped_value = ring.pop();
            if (popped_value)
            {
                pop_counts[*popped_value]++;
                i++;
            }
        }

    for (auto &pusher : pushers)
        pusher.join();

    for (const auto &pop_count : pop_counts)
        EXPECT_EQ(pop_count, NumberOfTries * NumberOfPusherThreads);
}
} // namespace MultiProducerSingleConsumerRingBufferTest
} // namespace Iyp
