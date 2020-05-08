#pragma once

#include "Iyp/WaitFreeRingBufferUtilities/ring-buffer-base.inl"

#include <cstdlib>

namespace Iyp
{
namespace WaitFreeRingBufferUtilities
{
template <typename T, std::size_t Count>
struct RingBuffer<T,
                  AccessRequirements::SINGLE_CONSUMER | AccessRequirements::SINGLE_PRODUCER,
                  Count> : SingleProducerBehaviour<SingleConsumerBehaviour<RingBufferStateBase<Element<T>, Count>>>
{
};
} // namespace WaitFreeRingBufferUtilities
} // namespace Iyp
