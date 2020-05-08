#pragma once

#include "Iyp/WaitFreeRingBufferUtilities/ring-buffer-base.inl"

#include <cstdlib>
#include <atomic>

namespace Iyp
{
namespace WaitFreeRingBufferUtilities
{
template <typename T, std::size_t Count>
struct RingBuffer<T,
                  AccessRequirements::MULTI_CONSUMER | AccessRequirements::SINGLE_PRODUCER,
                  Count> : SingleProducerBehaviour<MultiConsumerBehaviour<RingBufferStateBase<Element<T, MultiConsumerElementFeature>,
                                                                                              Count>>>
{
};
} // namespace WaitFreeRingBufferUtilities
} // namespace Iyp
