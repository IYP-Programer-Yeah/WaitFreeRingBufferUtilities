#pragma once

#ifdef __cpp_lib_hardware_interference_size
#include <new>
#endif

#include <utility>
#include <cstdint>

namespace Iyp
{
namespace WaitFreeRingBufferUtilities
{
namespace Details
{
namespace Private
{
constexpr std::size_t greatest_common_divisor(const std::size_t a, const std::size_t b)
{
    return a ? greatest_common_divisor(b % a, a) : b;
}

constexpr std::size_t least_common_multiple(const std::size_t a, const std::size_t b)
{
    return (!a || !b) ? 0 : (a * b / greatest_common_divisor(b, a));
}

constexpr bool is_diviseble_by(const std::size_t number, const std::size_t divisor)
{
    return (number % divisor) == 0;
}

template <typename T, std::size_t PadToMultipleOf, std::size_t AlignmentSize, bool NeedsPadding>
struct alignas(least_common_multiple(AlignmentSize, alignof(T))) AlignedPaddedObjectImpl : T
{
    AlignedPaddedObjectImpl() = default;

    template <typename Arg, typename... Args>
    explicit AlignedPaddedObjectImpl(Arg &&arg, Args &&... args) : T{std::forward<Arg>(arg),
                                                                     std::forward<Args>(args)...}
    {
    }
};

template <typename T, std::size_t PadToMultipleOf, std::size_t AlignmentSize>
struct alignas(least_common_multiple(AlignmentSize, alignof(T))) AlignedPaddedObjectImpl<T, PadToMultipleOf, AlignmentSize, true> : T
{
    std::uint8_t padding[PadToMultipleOf - (sizeof(T) % PadToMultipleOf)];

    AlignedPaddedObjectImpl() = default;

    template <typename Arg, typename... Args>
    explicit AlignedPaddedObjectImpl(Arg &&arg, Args &&... args) : T{std::forward<Arg>(arg),
                                                                     std::forward<Args>(args)...}
    {
    }
};

template <typename T, std::size_t PadToMultipleOf, std::size_t AlignmentSize>
using AlignedPaddedObject = AlignedPaddedObjectImpl<T, PadToMultipleOf,
                                                    AlignmentSize, !is_diviseble_by(sizeof(T), PadToMultipleOf)>;
} // namespace Private

enum : std::size_t
{
#ifndef __cpp_lib_hardware_interference_size
    CACHE_LINE_SIZE = 64,
    CACHE_ALIGNMENT = 64,
#else
    CACHE_LINE_SIZE = std::hardware_constructive_interference_size,
    CACHE_ALIGNMENT = std::hardware_destructive_interference_size,
#endif
};

template <typename T>
using CacheAlignedAndPaddedObject = Private::AlignedPaddedObject<T,
                                                                 Private::least_common_multiple(CACHE_LINE_SIZE,
                                                                                                CACHE_ALIGNMENT),
                                                                 CACHE_ALIGNMENT>;
} // namespace Details
} // namespace WaitFreeRingBufferUtilities
} // namespace Iyp
