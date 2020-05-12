#pragma once

#include <cstdint>
#include <cstddef>
#include <type_traits>

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

} // namespace WaitFreeRingBufferUtilities
} // namespace Iyp
