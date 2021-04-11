#pragma once

#include "Iyp/WaitFreeRingBufferUtilities/optional-type.inl"
#include "Iyp/WaitFreeRingBufferUtilities/details/cache-aligned-and-padded-object.inl"

#include <cstdint>
#include <utility>
#include <limits>
#include <cstddef>
#include <atomic>
#include <array>

namespace Iyp
{
namespace WaitFreeRingBufferUtilities
{
namespace Details
{
namespace ElementState
{
enum : std::uint_fast8_t
{
    IN_PROGRESS,
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

    std::array<CacheAlignedAndPaddedObject<Element<ElementType>>, Count> elements{};

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

template <typename ElementType, std::size_t Count>
struct MultiProducer
{
private:
    CacheAlignedAndPaddedObject<std::atomic_size_t> end{std::size_t(0)};
    CacheAlignedAndPaddedObject<std::atomic<std::int64_t>> push_task_count{static_cast<std::int64_t>(Count)};
    static_assert(Count <= static_cast<std::size_t>(std::numeric_limits<std::int64_t>::max()),
                  "Count exceeds the maximum. Count should fit in a std::int64_t.");

public:
    void notify_pop()
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

            std::uint_fast8_t expected_element_state = ElementState::READY_FOR_PUSH;
            if (std::atomic_compare_exchange_strong(&element.state, &expected_element_state, std::uint_fast8_t(ElementState::IN_PROGRESS)))
            {
                element.value_ptr = new (&element.storage) ElementType(std::forward<Args>(args)...);

                element.state.store(ElementState::READY_FOR_POP, std::memory_order_release);
                ring.notify_push();

                return true;
            }
        }
    }
};

template <typename ElementType, std::size_t Count>
struct MultiConsumer
{
private:
    CacheAlignedAndPaddedObject<std::atomic_size_t> begin{std::size_t(0)};
    CacheAlignedAndPaddedObject<std::atomic<std::int64_t>> pop_task_count{std::int64_t{0}};

public:
    void notify_push()
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

            std::uint_fast8_t expected_element_state = ElementState::READY_FOR_POP;
            if (std::atomic_compare_exchange_strong(&element.state, &expected_element_state, std::uint_fast8_t(ElementState::IN_PROGRESS)))
            {
                OptionalType<ElementType> result{std::move(*element.value_ptr)};
                element.value_ptr->~ElementType();

                element.state.store(ElementState::READY_FOR_PUSH, std::memory_order_release);
                ring.notify_pop();
                return result;
            }
        }
    }
};

template <typename ElementType, std::size_t Count>
struct SingleProducer
{
private:
    struct State
    {
        std::size_t end{0};
    };

    CacheAlignedAndPaddedObject<State> state;

public:
    void notify_pop() const
    {
    }

    template <typename Ring, typename... Args>
    bool push_impl(Ring &ring, Args &&...args)
    {
        auto &element = ring.elements[state.end];

        if (element.state.load(std::memory_order_acquire) == ElementState::READY_FOR_PUSH)
        {
            element.value_ptr = new (&element.storage) ElementType(std::forward<Args>(args)...);

            element.state.store(ElementState::READY_FOR_POP, std::memory_order_release);
            ring.notify_push();

            state.end = (state.end + 1) & Ring::COUNT_MASK;
            return true;
        }
        else
            return false;
    }
};

template <typename ElementType, std::size_t Count>
struct SingleConsumer
{
private:
    struct State
    {
        std::size_t begin{0};
    };

    CacheAlignedAndPaddedObject<State> state;

public:
    void notify_push() const
    {
    }

    template <typename Ring>
    OptionalType<ElementType> pop_impl(Ring &ring)
    {
        auto &element = ring.elements[state.begin];

        if (element.state.load(std::memory_order_acquire) == ElementState::READY_FOR_POP)
        {
            OptionalType<ElementType> result{std::move(*element.value_ptr)};
            element.value_ptr->~ElementType();

            element.state.store(ElementState::READY_FOR_PUSH, std::memory_order_release);
            ring.notify_pop();

            state.begin = (state.begin + 1) & Ring::COUNT_MASK;
            return result;
        }
        else
            return OptionalType<ElementType>{};
    }
};

} // namespace Details
} // namespace WaitFreeRingBufferUtilities
} // namespace Iyp