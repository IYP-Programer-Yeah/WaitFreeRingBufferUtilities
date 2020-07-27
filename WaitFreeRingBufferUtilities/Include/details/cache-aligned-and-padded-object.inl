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
constexpr bool is_diviseble_by(const std::size_t number, const std::size_t divisor)
{
    return (number % divisor) == 0;
}

constexpr std::size_t max(const std::size_t a, const std::size_t b)
{
    return (a < b) ? b : a;
}

template <typename T, std::size_t PadToMultipleOf, std::size_t AlignmentSize, bool NeedsPadding>
struct alignas(max(AlignmentSize, alignof(T))) AlignedPaddedObjectImpl : T
{
    AlignedPaddedObjectImpl() = default;

    template <typename Arg, typename... Args>
    explicit AlignedPaddedObjectImpl(Arg &&arg, Args &&... args) : T{std::forward<Arg>(arg),
                                                                     std::forward<Args>(args)...}
    {
    }
};

template <typename T, std::size_t PadToMultipleOf, std::size_t AlignmentSize>
struct alignas(max(AlignmentSize, alignof(T))) AlignedPaddedObjectImpl<T, PadToMultipleOf, AlignmentSize, true> : T
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
    DESTRUCTIVE_INTERFERENCE_SIZE = 64,
#else
    DESTRUCTIVE_INTERFERENCE_SIZE = std::hardware_destructive_interference_size,
#endif
};

template <typename T>
using CacheAlignedAndPaddedObject = Private::AlignedPaddedObject<T,
                                                                 DESTRUCTIVE_INTERFERENCE_SIZE,
                                                                 DESTRUCTIVE_INTERFERENCE_SIZE>;
} // namespace Details
} // namespace WaitFreeRingBufferUtilities
} // namespace Iyp
