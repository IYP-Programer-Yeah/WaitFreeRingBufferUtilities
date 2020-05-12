#pragma once

#ifdef __cpp_lib_optional
#include <optional>
#else
#include <boost/optional.hpp>
#endif

namespace Iyp
{
namespace WaitFreeRingBufferUtilities
{
#ifdef __cpp_lib_optional
template <typename T>
using OptionalType = std::optional<T>;
#else
template <typename T>
using OptionalType = boost::optional<T>;
#endif
} // namespace WaitFreeRingBufferUtilities
} // namespace Iyp