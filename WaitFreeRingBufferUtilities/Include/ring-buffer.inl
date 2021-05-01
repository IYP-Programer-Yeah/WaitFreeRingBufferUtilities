#pragma once

#include "Iyp/WaitFreeRingBufferUtilities/optional-type.inl"
#include "Iyp/WaitFreeRingBufferUtilities/details/cache-aligned-and-padded-object.inl"

#include <cstdint>
#include <utility>
#include <cstddef>
#include <atomic>
#include <array>

namespace Iyp
{
namespace WaitFreeRingBufferUtilities
{
namespace Private
{
namespace ElementState
{
enum : std::uint_fast8_t // Use enum instead of inline constexpr variable to support C++11
{
    IN_PROGRESS, // Optional
    READY_FOR_PUSH,
    READY_FOR_POP,
};
} // namespace ElementState

template <typename T>
struct Element
{
    using ElementType = T;
    std::atomic<std::uint_fast8_t> state{ElementState::READY_FOR_PUSH};
    T *value_ptr;
    typename std::aligned_storage<sizeof(T), alignof(T)>::type storage;

    Element() = default;

    Element(const Element &) = delete;
    Element(Element &&) = delete;

    Element &operator=(const Element &) = delete;
    Element &operator=(Element &&) = delete;

    ~Element()
    {
        if (state.load(std::memory_order_acquire) == ElementState::READY_FOR_POP)
            value_ptr->~T();
    }
};

template <template <typename, std::size_t> class Producer,
          template <typename, std::size_t> class Consumer,
          typename ElementType, std::size_t Count>
struct RingBufferTypeConstructor : Producer<ElementType, Count>, Consumer<ElementType, Count>
{
    enum : std::size_t
    {
        COUNT_MASK = Count - 1,
    };
    static_assert(Count && !(COUNT_MASK & Count), "Count should be a power of two.");

    std::array<Details::CacheAlignedAndPaddedObject<Element<ElementType>>, Count> elements{};

    template <typename... Args>
    bool push(Args &&...args)
    {
        return this->push_impl(*this, std::forward<Args>(args)...);
    }

    OptionalType<ElementType> pop()
    {
        return this->pop_impl(*this);
    }
};
} // namespace Private

template <template <typename, std::size_t> class Producer,
          template <typename, std::size_t> class Consumer,
          typename ElementType, std::size_t Count>
class RingBuffer : Private::RingBufferTypeConstructor<Producer, Consumer, ElementType, Count>
{
    using Parrent = Private::RingBufferTypeConstructor<Producer, Consumer, ElementType, Count>;

public:
    using Parrent::push;
    using Parrent::pop;
};

} // namespace WaitFreeRingBufferUtilities
} // namespace Iyp