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
                  AccessRequirements::MULTI_CONSUMER | AccessRequirements::SINGLE_PRODUCER,
                  Count> : Details::RingBufferTypeConstructor<Details::SingleProducer,
                                                              Details::MultiConsumer,
                                                              T, Count>
{
};
} // namespace WaitFreeRingBufferUtilities
} // namespace Iyp
