#pragma once

#include <cstdint>
#include <utility>

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

AccessRequirements &operator|=(AccessRequirements &lhs, const AccessRequirements rhs)
{
	return (lhs = static_cast<AccessRequirements>(static_cast<std::underlying_type<AccessRequirements>::type>(lhs) | static_cast<std::underlying_type<AccessRequirements>::type>(rhs)));
}

AccessRequirements &operator&=(AccessRequirements &lhs, const AccessRequirements rhs)
{
	return (lhs = static_cast<AccessRequirements>(static_cast<std::underlying_type<AccessRequirements>::type>(lhs) & static_cast<std::underlying_type<AccessRequirements>::type>(rhs)));
}

template <typename T, AccessRequirements RingType, std::size_t Count>
struct RingBuffer;

#ifdef __cpp_lib_optional
template <typename T>
using OptionalType = std::optional<T>;
#else
template <typename T>
using OptionalType = boost::optional<T>;
#endif
} // namespace WaitFreeRingBufferUtilities
} // namespace Iyp
