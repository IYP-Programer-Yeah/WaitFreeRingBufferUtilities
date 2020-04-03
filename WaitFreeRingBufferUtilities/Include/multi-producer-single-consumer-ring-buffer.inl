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
                  AccessRequirements::SINGLE_CONSUMER | AccessRequirements::MULTI_PRODUCER,
                  Count> : MultiProducerBehaviour<SingleConsumerBehaviour<RingBufferStateBase<Element<T, MultiProducerElementFeature>,
                                                                                              Count, SizeTWrapper, std::atomic_size_t>>>
{
};
} // namespace WaitFreeRingBufferUtilities
} // namespace Iyp
