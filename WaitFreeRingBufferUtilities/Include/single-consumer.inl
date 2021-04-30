#pragma once

#include "Iyp/WaitFreeRingBufferUtilities/optional-type.inl"
#include "Iyp/WaitFreeRingBufferUtilities/details/cache-aligned-and-padded-object.inl"

#include <cstdint>
#include <utility>
#include <atomic>

namespace Iyp
{
namespace WaitFreeRingBufferUtilities
{

template <typename ElementType, std::size_t Count>
struct SingleConsumer
{
private:
    struct State
    {
        std::size_t begin{0};
    };

    Details::CacheAlignedAndPaddedObject<State> state;

public:
    template <typename Ring>
    void notify_push(const Ring &) const
    {
    }

    template <typename Ring>
    OptionalType<ElementType> pop_impl(Ring &ring)
    {
        auto &element = ring.elements[state.begin];

        if (element.state.load(std::memory_order_acquire) == Private::ElementState::READY_FOR_POP)
        {
            OptionalType<ElementType> result{std::move(*element.value_ptr)};
            element.value_ptr->~ElementType();

            element.state.store(Private::ElementState::READY_FOR_PUSH, std::memory_order_release);
            ring.notify_pop(ring);

            state.begin = (state.begin + 1) & Ring::COUNT_MASK;
            return result;
        }
        else
            return OptionalType<ElementType>{};
    }
};

} // namespace WaitFreeRingBufferUtilities
} // namespace Iyp
