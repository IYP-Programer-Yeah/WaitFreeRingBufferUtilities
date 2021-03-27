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

template <typename Element, std::size_t Count>
struct RingBufferStateBase
{
protected:
    using ElementType = typename Element::ElementType;

    enum : std::size_t
    {
        COUNT_MASK = Count - 1,
    };

    std::array<CacheAlignedAndPaddedObject<Element>, Count> elements{};

public:
    enum : std::size_t
    {
        COUNT = Count,
    };

    static_assert(Count > 0 && !(COUNT_MASK & Count), "Count should be a power of two.");
};

namespace Private
{
template <std::size_t Count>
struct CountInt64CompatibilityCheck
{
    static_assert(Count <= static_cast<std::size_t>(std::numeric_limits<std::int64_t>::max()), "Count exceeds the maximum. Count should fit in a std::int64_t.");
};

struct DummyAtomicSizeT
{
    std::size_t fetch_add(const std::size_t, const std::memory_order)
    {
        return 0;
    }
};
} // namespace Private

template <typename ProducerTypeTraits, typename ConsumerTypeTraits, typename T, std::size_t Count>
struct RingBufferTypeConstructor : ProducerTypeTraits::template Behavior<
                                       typename ConsumerTypeTraits::template Behavior<
                                           typename ProducerTypeTraits::template SharedState<
                                               typename ConsumerTypeTraits::template SharedState<
                                                   RingBufferStateBase<Element<T>, Count>>>>>
{
};

struct MultiProducerTypeTraits
{
    template <typename BaseType>
    struct Behavior : BaseType
    {
    private:
        CacheAlignedAndPaddedObject<std::atomic_size_t> end{std::size_t(0)};

    public:
        using ElementType = typename BaseType::ElementType;

        template <typename... Args>
        bool push(Args &&... args)
        {
            if (BaseType::push_task_count.fetch_sub(1, std::memory_order_acq_rel) <= std::int64_t(0))
            {
                BaseType::push_task_count.fetch_add(1, std::memory_order_relaxed);
                return false;
            }

            while (true)
            {
                const std::size_t element_index = end.fetch_add(1, std::memory_order_relaxed) & BaseType::COUNT_MASK;
                auto &element = BaseType::elements[element_index];

                std::uint_fast8_t expected_element_state = ElementState::READY_FOR_PUSH;
                if (std::atomic_compare_exchange_strong(&element.state, &expected_element_state, std::uint_fast8_t(ElementState::IN_PROGRESS)))
                {
                    element.value_ptr = new (&element.storage) ElementType(std::forward<Args>(args)...);

                    element.state.store(ElementState::READY_FOR_POP, std::memory_order_release);
                    BaseType::pop_task_count.fetch_add(1, std::memory_order_release);

                    return true;
                }
            }
        }
    };

    template <typename BaseType, typename = Private::CountInt64CompatibilityCheck<BaseType::COUNT>>
    struct SharedState : BaseType
    {
    protected:
        CacheAlignedAndPaddedObject<std::atomic<std::int64_t>> push_task_count{static_cast<std::int64_t>(BaseType::COUNT)};
    };
};

struct MultiConsumerTypeTraits
{
    template <typename BaseType>
    struct Behavior : BaseType
    {
    private:
        CacheAlignedAndPaddedObject<std::atomic_size_t> begin{std::size_t(0)};

    public:
        using ElementType = typename BaseType::ElementType;

        OptionalType<ElementType> pop()
        {
            if (BaseType::pop_task_count.fetch_sub(1, std::memory_order_acq_rel) <= std::int64_t(0))
            {
                BaseType::pop_task_count.fetch_add(1, std::memory_order_relaxed);
                return OptionalType<ElementType>{};
            }

            while (true)
            {
                const std::size_t element_index = begin.fetch_add(1, std::memory_order_relaxed) & BaseType::COUNT_MASK;
                auto &element = BaseType::elements[element_index];

                std::uint_fast8_t expected_element_state = ElementState::READY_FOR_POP;
                if (std::atomic_compare_exchange_strong(&element.state, &expected_element_state, std::uint_fast8_t(ElementState::IN_PROGRESS)))
                {
                    OptionalType<ElementType> result{std::move(*element.value_ptr)};
                    element.value_ptr->~ElementType();

                    element.state.store(ElementState::READY_FOR_PUSH, std::memory_order_release);
                    BaseType::push_task_count.fetch_add(1, std::memory_order_release);
                    return result;
                }
            }
        }
    };

    template <typename BaseType, typename = Private::CountInt64CompatibilityCheck<BaseType::COUNT>>
    struct SharedState : BaseType
    {
    protected:
        CacheAlignedAndPaddedObject<std::atomic<std::int64_t>> pop_task_count{std::int64_t{0}};
    };
};

struct MultiProducerForSingleConsumerTypeTraits
{
    template <typename BaseType>
    struct Behavior : BaseType
    {
    private:
        CacheAlignedAndPaddedObject<std::atomic_size_t> end{std::size_t(0)};

    public:
        using ElementType = typename BaseType::ElementType;

        template <typename... Args>
        bool push(Args &&... args)
        {
            if (BaseType::push_task_count.fetch_sub(1, std::memory_order_acquire) <= std::int64_t(0))
            {
                BaseType::push_task_count.fetch_add(1, std::memory_order_relaxed);
                return false;
            }

            const std::size_t element_index = end.fetch_add(1, std::memory_order_acquire) & BaseType::COUNT_MASK;
            auto &element = BaseType::elements[element_index];

            element.value_ptr = new (&element.storage) ElementType(std::forward<Args>(args)...);
            element.state.store(ElementState::READY_FOR_POP, std::memory_order_release);
            BaseType::pop_task_count.fetch_add(1, std::memory_order_release);
            return true;
        }
    };

    template <typename BaseType, typename = Private::CountInt64CompatibilityCheck<BaseType::COUNT>>
    struct SharedState : BaseType
    {
    protected:
        CacheAlignedAndPaddedObject<std::atomic<std::int64_t>> push_task_count{static_cast<std::int64_t>(BaseType::COUNT)};
    };
};

struct MultiConsumerForSingleProducerTypeTraits
{
    template <typename BaseType>
    struct Behavior : BaseType
    {
    private:
        CacheAlignedAndPaddedObject<std::atomic_size_t> begin{std::size_t(0)};

    public:
        using ElementType = typename BaseType::ElementType;

        OptionalType<ElementType> pop()
        {
            if (BaseType::pop_task_count.fetch_sub(1, std::memory_order_acquire) <= std::int64_t(0))
            {
                BaseType::pop_task_count.fetch_add(1, std::memory_order_relaxed);
                return OptionalType<ElementType>{};
            }

            const std::size_t element_index = begin.fetch_add(1, std::memory_order_acquire) & BaseType::COUNT_MASK;
            auto &element = BaseType::elements[element_index];

            OptionalType<ElementType> result{std::move(*element.value_ptr)};
            element.value_ptr->~ElementType();

            element.state.store(ElementState::READY_FOR_PUSH, std::memory_order_release);
            return result;
        }
    };

    template <typename BaseType, typename = Private::CountInt64CompatibilityCheck<BaseType::COUNT>>
    struct SharedState : BaseType
    {
    protected:
        CacheAlignedAndPaddedObject<std::atomic<std::int64_t>> pop_task_count{std::int64_t{0}};
    };
};

struct SingleProducerTypeTraits
{
    template <typename BaseType>
    struct Behavior : BaseType
    {
    private:
        struct State
        {
            std::size_t end{0};
        };

        CacheAlignedAndPaddedObject<State> state;

    public:
        using ElementType = typename BaseType::ElementType;

        template <typename... Args>
        bool push(Args &&... args)
        {
            auto &element = BaseType::elements[state.end];

            if (element.state.load(std::memory_order_acquire) == ElementState::READY_FOR_PUSH)
            {
                element.value_ptr = new (&element.storage) ElementType(std::forward<Args>(args)...);

                element.state.store(ElementState::READY_FOR_POP, std::memory_order_release);
                BaseType::pop_task_count.fetch_add(1, std::memory_order_release);

                state.end = (state.end + 1)  & BaseType::COUNT_MASK;
                return true;
            }
            else
                return false;
        }
    };

    template <typename BaseType>
    struct SharedState : BaseType
    {
    protected:
        Private::DummyAtomicSizeT push_task_count{};
    };
};

struct SingleConsumerTypeTraits
{
    template <typename BaseType>
    struct Behavior : BaseType
    {
    private:
        struct State
        {
            std::size_t begin{0};
        };

        CacheAlignedAndPaddedObject<State> state;

    public:
        using ElementType = typename BaseType::ElementType;

        OptionalType<ElementType> pop()
        {
            auto &element = BaseType::elements[state.begin];

            if (element.state.load(std::memory_order_acquire) == ElementState::READY_FOR_POP)
            {
                OptionalType<ElementType> result{std::move(*element.value_ptr)};
                element.value_ptr->~ElementType();

                element.state.store(ElementState::READY_FOR_PUSH, std::memory_order_release);
                BaseType::push_task_count.fetch_add(1, std::memory_order_release);

                state.begin = (state.begin + 1) & BaseType::COUNT_MASK;
                return result;
            }
            else
                return OptionalType<ElementType>{};
        }
    };

    template <typename BaseType>
    struct SharedState : BaseType
    {
    protected:
        Private::DummyAtomicSizeT pop_task_count{};
    };
};

} // namespace Details
} // namespace WaitFreeRingBufferUtilities
} // namespace Iyp