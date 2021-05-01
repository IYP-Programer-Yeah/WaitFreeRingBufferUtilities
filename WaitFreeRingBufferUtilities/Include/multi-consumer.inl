#pragma once

#include "Iyp/WaitFreeRingBufferUtilities/optional-type.inl"
#include "Iyp/WaitFreeRingBufferUtilities/details/cache-aligned-and-padded-object.inl"

#include <cstdint>
#include <utility>
#include <cstddef>
#include <atomic>

namespace Iyp
{
namespace WaitFreeRingBufferUtilities
{
template <typename ElementType, std::size_t Count>
class MultiConsumer
{
private:
    Details::CacheAlignedAndPaddedObject<std::atomic_size_t> begin{std::size_t(0)};
    Details::CacheAlignedAndPaddedObject<std::atomic<std::int64_t>> pop_task_count{std::int64_t{0}};

public:
    template <typename Ring>
    void notify_push(Ring &)
    {
        pop_task_count.fetch_add(1, std::memory_order_release);
    }

    template <typename Ring>
    OptionalType<ElementType> pop_impl(Ring &ring)
    {
        if (pop_task_count.fetch_sub(1, std::memory_order_acq_rel) <= std::int64_t(0))
        {
            pop_task_count.fetch_add(1, std::memory_order_relaxed);
            return OptionalType<ElementType>{};
        }

        while (true)
        {
            const std::size_t element_index = begin.fetch_add(1, std::memory_order_relaxed) & Ring::COUNT_MASK;
            auto &element = ring.elements[element_index];

            std::uint_fast8_t expected_element_state = Private::ElementState::READY_FOR_POP;
            if (std::atomic_compare_exchange_strong(&element.state, &expected_element_state, std::uint_fast8_t(Private::ElementState::IN_PROGRESS)))
            {
                OptionalType<ElementType> result{std::move(*element.value_ptr)};
                element.value_ptr->~ElementType();

                element.state.store(Private::ElementState::READY_FOR_PUSH, std::memory_order_release);
                ring.notify_pop(ring);
                return result;
            }
        }
    }
};

} // namespace WaitFreeRingBufferUtilities
} // namespace Iyp
