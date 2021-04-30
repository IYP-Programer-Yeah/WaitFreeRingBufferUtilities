#pragma once

#include "Iyp/WaitFreeRingBufferUtilities/optional-type.inl"
#include "Iyp/WaitFreeRingBufferUtilities/details/cache-aligned-and-padded-object.inl"

#include <utility>
#include <cstddef>
#include <atomic>

namespace Iyp
{
namespace WaitFreeRingBufferUtilities
{
template <typename ElementType, std::size_t Count>
struct SingleProducer
{
private:
    struct State
    {
        std::size_t end{0};
    };

    Details::CacheAlignedAndPaddedObject<State> state;

public:
    template <typename Ring>
    void notify_pop(const Ring &) const
    {
    }

    template <typename Ring, typename... Args>
    bool push_impl(Ring &ring, Args &&...args)
    {
        auto &element = ring.elements[state.end];

        if (element.state.load(std::memory_order_acquire) == Private::ElementState::READY_FOR_PUSH)
        {
            element.value_ptr = new (&element.storage) ElementType(std::forward<Args>(args)...);

            element.state.store(Private::ElementState::READY_FOR_POP, std::memory_order_release);
            ring.notify_push(ring);

            state.end = (state.end + 1) & Ring::COUNT_MASK;
            return true;
        }
        else
            return false;
    }
};

} // namespace WaitFreeRingBufferUtilities
} // namespace Iyp
