#pragma once

#include "Iyp/WaitFreeRingBufferUtilities/ring-buffer-base.inl"
#include "Iyp/WaitFreeRingBufferUtilities/details/ring-buffer-type-constructor.inl"

#include <cstdlib>
#include <atomic>

namespace Iyp
{
namespace WaitFreeRingBufferUtilities
{
template <typename T, std::size_t Count>
struct RingBuffer<T,
                  AccessRequirements::MULTI_CONSUMER | AccessRequirements::MULTI_PRODUCER,
                  Count> : Details::RingBufferTypeConstructor<Details::MultiProducerTypeTraits,
                                                              Details::MultiConsumerTypeTraits,
                                                              T, Count>
{
};
} // namespace WaitFreeRingBufferUtilities
} // namespace Iyp
