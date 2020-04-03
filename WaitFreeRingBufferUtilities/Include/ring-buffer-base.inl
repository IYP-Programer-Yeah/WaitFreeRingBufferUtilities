#pragma once

#include <cstdint>
#include <utility>
#include <limits>
#include <cstdlib>
#include <atomic>
#include <array>
#include <type_traits>

#include "Iyp/WaitFreeRingBufferUtilities/details/cache-aligned-and-padded-object.inl"

#ifdef __cpp_lib_optional
#include <optional>
#else
#include <boost/optional.hpp>
#endif

namespace Iyp
{
namespace WaitFreeRingBufferUtilities
{
enum class AccessRequirements : std::uint8_t
{
    SINGLE_CONSUMER = 0x00,
    SINGLE_PRODUCER = 0x00,
    MULTI_CONSUMER = 0x01,
    MULTI_PRODUCER = 0x02,
};

constexpr AccessRequirements operator|(const AccessRequirements lhs, const AccessRequirements rhs)
{
    return static_cast<AccessRequirements>(static_cast<std::underlying_type<AccessRequirements>::type>(lhs) | static_cast<std::underlying_type<AccessRequirements>::type>(rhs));
}

constexpr AccessRequirements operator&(const AccessRequirements lhs, const AccessRequirements rhs)
{
    return static_cast<AccessRequirements>(static_cast<std::underlying_type<AccessRequirements>::type>(lhs) & static_cast<std::underlying_type<AccessRequirements>::type>(rhs));
}

inline AccessRequirements &operator|=(AccessRequirements &lhs, const AccessRequirements rhs)
{
    return (lhs = static_cast<AccessRequirements>(static_cast<std::underlying_type<AccessRequirements>::type>(lhs) | static_cast<std::underlying_type<AccessRequirements>::type>(rhs)));
}

inline AccessRequirements &operator&=(AccessRequirements &lhs, const AccessRequirements rhs)
{
    return (lhs = static_cast<AccessRequirements>(static_cast<std::underlying_type<AccessRequirements>::type>(lhs) & static_cast<std::underlying_type<AccessRequirements>::type>(rhs)));
}

template <typename T, AccessRequirements RingType, std::size_t Count>
struct RingBuffer;

struct MultiProducerElementFeature
{
    Details::CacheAlignedAndPaddedObject<std::atomic_bool> is_pusher_processing;

    MultiProducerElementFeature() : is_pusher_processing(false)
    {
    }
};

struct MultiConsumerElementFeature
{
    Details::CacheAlignedAndPaddedObject<std::atomic_bool> is_popper_processing;

    MultiConsumerElementFeature() : is_popper_processing(false)
    {
    }
};

template <typename T, typename... ElementFeatures>
struct Element : ElementFeatures...
{
    using ElementType = T;

    std::atomic<T *> value_ptr;
    typename std::aligned_storage<sizeof(T), alignof(T)>::type storage;

    Element() : value_ptr(nullptr)
    {
    }

    Element(const Element &) = delete;
    Element(Element &&) = delete;

    Element &operator=(const Element &) = delete;
    Element &operator=(Element &&) = delete;

    ~Element()
    {
        const auto local_value_ptr = value_ptr.load(std::memory_order_relaxed);
        if (local_value_ptr)
            local_value_ptr->~T();
    }
};

struct SizeTWrapper
{
    std::size_t value;

    SizeTWrapper(const std::size_t i_value) : value(i_value)
    {
    }
};

template <typename Element, std::size_t Count, typename BeginType, typename EndType>
struct RingBufferStateBase
{
protected:
    using ElementType = typename Element::ElementType;

    enum : std::size_t
    {
        COUNT_MASK = Count - 1,
    };

    Details::CacheAlignedAndPaddedObject<BeginType> begin;
    Details::CacheAlignedAndPaddedObject<EndType> end;
    Details::CacheAlignedAndPaddedObject<std::atomic<std::int64_t>> pop_task_count;
    Details::CacheAlignedAndPaddedObject<std::atomic<std::int64_t>> push_task_count;
    std::array<Details::CacheAlignedAndPaddedObject<Element>, Count> elements;

public:
    static_assert(Count > 0 && !(COUNT_MASK & Count), "Count should be a power of two.");
    static_assert(Count <= static_cast<std::size_t>(std::numeric_limits<std::int64_t>::max()), "Count exceeds the maximum. Count should fit in a std::int64_t.");

    RingBufferStateBase() : begin(std::size_t(0)), end(std::size_t(0)),
                            pop_task_count(std::int64_t(0)), push_task_count(static_cast<std::int64_t>(Count)), elements{}
    {
    }
};

#ifdef __cpp_lib_optional
template <typename T>
using OptionalType = std::optional<T>;
#else
template <typename T>
using OptionalType = boost::optional<T>;
#endif

template <typename BaseType>
struct MultiProducerBehaviour : BaseType
{
    using ElementType = typename BaseType::ElementType;

    template <typename... Args>
    bool push(Args &&... args)
    {
        if (BaseType::push_task_count.fetch_sub(1, std::memory_order_acquire) <= std::int64_t(0))
        {
            BaseType::push_task_count.fetch_add(1, std::memory_order_relaxed);
            return false;
        }

        while (true)
        {
            const std::size_t local_end = BaseType::end.fetch_add(1, std::memory_order_acquire);
            const std::size_t element_index = local_end & BaseType::COUNT_MASK;
            bool expected_is_pusher_processing = false;
            if (std::atomic_compare_exchange_strong(&BaseType::elements[element_index].is_pusher_processing, &expected_is_pusher_processing, true))
            {
                if (BaseType::elements[element_index].value_ptr.load(std::memory_order_relaxed))
                {
                    BaseType::elements[element_index].is_pusher_processing.store(false, std::memory_order_release);
                    continue;
                }

                BaseType::elements[element_index].value_ptr.store(new (&BaseType::elements[element_index].storage) ElementType(std::forward<Args>(args)...),
                                                                  std::memory_order_release);
                BaseType::pop_task_count.fetch_add(1, std::memory_order_release);

                BaseType::elements[element_index].is_pusher_processing.store(false, std::memory_order_release);
                return true;
            }
        }
    }
};

template <typename BaseType>
struct MultiConsumerBehaviour : BaseType
{
    using ElementType = typename BaseType::ElementType;

    OptionalType<ElementType> pop()
    {
        if (BaseType::pop_task_count.fetch_sub(1, std::memory_order_acquire) <= std::int64_t(0))
        {
            BaseType::pop_task_count.fetch_add(1, std::memory_order_relaxed);
            return OptionalType<ElementType>{};
        }

        while (true)
        {
            const std::size_t local_begin = BaseType::begin.fetch_add(1, std::memory_order_acquire);

            const std::size_t element_index = local_begin & BaseType::COUNT_MASK;
            bool expected_is_popper_processing = false;
            if (std::atomic_compare_exchange_strong(&BaseType::elements[element_index].is_popper_processing, &expected_is_popper_processing, true))
            {
                const auto value_ptr = BaseType::elements[element_index].value_ptr.load(std::memory_order_relaxed);
                if (!value_ptr)
                {
                    BaseType::elements[element_index].is_popper_processing.store(false, std::memory_order_release);
                    continue;
                }

                OptionalType<ElementType> result{std::move(*value_ptr)};
                value_ptr->~ElementType();

                BaseType::elements[element_index].value_ptr.store(nullptr, std::memory_order_release);
                BaseType::push_task_count.fetch_add(1, std::memory_order_release);

                BaseType::elements[element_index].is_popper_processing.store(false, std::memory_order_release);
                return result;
            }
        }
    }
};

template <typename BaseType>
struct SingleProducerBehaviour : BaseType
{
    using ElementType = typename BaseType::ElementType;

    template <typename... Args>
    bool push(Args &&... args)
    {
        if (BaseType::push_task_count.fetch_sub(1, std::memory_order_acquire) <= std::int64_t(0))
        {
            BaseType::push_task_count.fetch_add(1, std::memory_order_relaxed);
            return false;
        }

        while (true)
        {
            const std::size_t element_index = (BaseType::end.value++) & BaseType::COUNT_MASK;

            if (BaseType::elements[element_index].value_ptr.load(std::memory_order_relaxed))
                continue;

            BaseType::elements[element_index].value_ptr.store(new (&BaseType::elements[element_index].storage) ElementType(std::forward<Args>(args)...),
                                                              std::memory_order_release);
            BaseType::pop_task_count.fetch_add(1, std::memory_order_release);

            return true;
        }
    }
};

template <typename BaseType>
struct SingleConsumerBehaviour : BaseType
{
    using ElementType = typename BaseType::ElementType;

    OptionalType<ElementType> pop()
    {
        if (BaseType::pop_task_count.fetch_sub(1, std::memory_order_acquire) <= std::int64_t(0))
        {
            BaseType::pop_task_count.fetch_add(1, std::memory_order_relaxed);
            return OptionalType<ElementType>{};
        }

        while (true)
        {
            const std::size_t element_index = (BaseType::begin.value++) & BaseType::COUNT_MASK;

            const auto value_ptr = BaseType::elements[element_index].value_ptr.load(std::memory_order_relaxed);
            if (!value_ptr)
                continue;

            OptionalType<ElementType> result{std::move(*value_ptr)};
            value_ptr->~ElementType();

            BaseType::elements[element_index].value_ptr.store(nullptr, std::memory_order_release);
            BaseType::push_task_count.fetch_add(1, std::memory_order_release);

            return result;
        }
    }
};

} // namespace WaitFreeRingBufferUtilities
} // namespace Iyp
