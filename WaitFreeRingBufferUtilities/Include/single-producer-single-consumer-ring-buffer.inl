#pragma once

#include "Iyp/WaitFreeRingBufferUtilities/ring-buffer-base.inl"
#include "Iyp/WaitFreeRingBufferUtilities/details/ring-buffer-type-constructor.inl"

#include <cstdlib>

namespace Iyp
{
namespace WaitFreeRingBufferUtilities
{
template <typename T, std::size_t Count>
struct RingBuffer<T,
                  AccessRequirements::SINGLE_CONSUMER | AccessRequirements::SINGLE_PRODUCER,
                  Count> : Details::RingBufferTypeConstructor<Details::SingleProducer,
                                                              Details::SingleConsumer,
                                                              T, Count>
{
};
} // namespace WaitFreeRingBufferUtilities
} // namespace Iyp
