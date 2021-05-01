#pragma once

#include "Iyp/WaitFreeRingBufferUtilities/optional-type.inl"
#include "Iyp/WaitFreeRingBufferUtilities/details/cache-aligned-and-padded-object.inl"

#include <cstdint>
#include <utility>
#include <limits>
#include <cstddef>
#include <atomic>

namespace Iyp
{
namespace WaitFreeRingBufferUtilities
{

template <typename ElementType, std::size_t Count>
class MultiProducer
{
    Details::CacheAlignedAndPaddedObject<std::atomic_size_t> end{std::size_t(0)};
    Details::CacheAlignedAndPaddedObject<std::atomic<std::int64_t>> push_task_count{static_cast<std::int64_t>(Count)};
    static_assert(Count <= static_cast<std::size_t>(std::numeric_limits<std::int64_t>::max()),
                  "Count exceeds the maximum. Count should fit in a std::int64_t.");

public:
    template <typename Ring>
    void notify_pop(const Ring &)
    {
        push_task_count.fetch_add(1, std::memory_order_release);
    }

    template <typename Ring, typename... Args>
    bool push_impl(Ring &ring, Args &&...args)
    {
        if (push_task_count.fetch_sub(1, std::memory_order_acq_rel) <= std::int64_t(0))
        {
            push_task_count.fetch_add(1, std::memory_order_relaxed);
            return false;
        }

        while (true)
        {
            const std::size_t element_index = end.fetch_add(1, std::memory_order_relaxed) & Ring::COUNT_MASK;
            auto &element = ring.elements[element_index];

            std::uint_fast8_t expected_element_state = Private::ElementState::READY_FOR_PUSH;
            if (std::atomic_compare_exchange_strong(&element.state, &expected_element_state, std::uint_fast8_t(Private::ElementState::IN_PROGRESS)))
            {
                element.value_ptr = new (&element.storage) ElementType(std::forward<Args>(args)...);

                element.state.store(Private::ElementState::READY_FOR_POP, std::memory_order_release);
                ring.notify_push(ring);

                return true;
            }
        }
    }
};

} // namespace WaitFreeRingBufferUtilities
} // namespace Iyp
