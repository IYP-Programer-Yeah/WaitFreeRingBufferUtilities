#pragma once

#include "Iyp/WaitFreeRingBufferUtilities/ring-buffer-base.inl"
#include "Iyp/WaitFreeRingBufferUtilities/details/cache-aligned-and-padded-object.inl"

#include <limits>
#include <cstdlib>
#include <cstdint>
#include <atomic>
#include <array>

namespace Iyp
{
namespace WaitFreeRingBufferUtilities
{
template <typename T, std::size_t Count>
class RingBuffer<T, AccessRequirements::MULTI_CONSUMER | AccessRequirements::MULTI_PRODUCER, Count>
{
	enum : std::size_t
	{
		COUNT_MASK = Count - 1,
	};

	struct Element
	{
		Details::CacheAlignedAndPaddedObject<std::atomic_bool> is_popper_processing;
		Details::CacheAlignedAndPaddedObject<std::atomic_bool> is_pusher_processing;
		std::atomic_bool is_free;
		alignas(alignof(T)) char memory[sizeof(T)];

		Element() : is_popper_processing(false), is_pusher_processing(false), is_free(true)
		{
		}

		Element(const Element &) = delete;
		Element(Element &&) = delete;

		Element &operator=(const Element &) = delete;
		Element &operator=(Element &&) = delete;

		~Element()
		{
			if (!is_free.load(std::memory_order_relaxed))
				reinterpret_cast<T *>(memory)->~T();
		}
	};

	Details::CacheAlignedAndPaddedObject<std::atomic_size_t> begin;
	Details::CacheAlignedAndPaddedObject<std::atomic_size_t> end;
	Details::CacheAlignedAndPaddedObject<std::atomic<std::int64_t>> pop_task_count;
	Details::CacheAlignedAndPaddedObject<std::atomic<std::int64_t>> push_task_count;
	std::array<Details::CacheAlignedAndPaddedObject<Element>, Count> elements;

public:
	static_assert(Count > 0 && !(COUNT_MASK & Count), "Count should be a power of two.");
	static_assert(Count <= static_cast<std::size_t>(std::numeric_limits<std::int64_t>::max()), "Count exceeds the maximum. Count should fit in a std::int64_t.");

	RingBuffer() : begin(0), end(0), pop_task_count(0), push_task_count(static_cast<std::int64_t>(Count)), elements()
	{
	}

	template <typename... Args>
	bool push(Args &&... args)
	{
		if (push_task_count.fetch_sub(1, std::memory_order_acquire) <= 0)
		{
			push_task_count.fetch_add(1, std::memory_order_relaxed);
			return false;
		}

		while (true)
		{
			const std::size_t local_end = end.fetch_add(1, std::memory_order_acquire);
			const std::size_t element_index = local_end & COUNT_MASK;
			bool expected_is_pusher_processing = false;
			if (std::atomic_compare_exchange_strong(&elements[element_index].is_pusher_processing, &expected_is_pusher_processing, true))
			{
				if (!elements[element_index].is_free.load(std::memory_order_relaxed))
				{
					elements[element_index].is_pusher_processing.store(false, std::memory_order_release);
					continue;
				}

				new (&elements[element_index].memory) T(std::forward<Args>(args)...);

				elements[element_index].is_free.store(false, std::memory_order_release);
				pop_task_count.fetch_add(1, std::memory_order_release);

				elements[element_index].is_pusher_processing.store(false, std::memory_order_release);
				return true;
			}
		}
	}

	OptionalType<T> pop()
	{
		if (pop_task_count.fetch_sub(1, std::memory_order_acquire) <= std::int64_t(0))
		{
			pop_task_count.fetch_add(1, std::memory_order_relaxed);
			return OptionalType<T>{};
		}

		while (true)
		{
			const std::size_t local_begin = begin.fetch_add(1, std::memory_order_acquire);

			const std::size_t element_index = local_begin & COUNT_MASK;
			bool expected_is_popper_processing = false;
			if (std::atomic_compare_exchange_strong(&elements[element_index].is_popper_processing, &expected_is_popper_processing, true))
			{
				if (elements[element_index].is_free.load(std::memory_order_relaxed))
				{
					elements[element_index].is_popper_processing.store(false, std::memory_order_release);
					continue;
				}

				auto &back = (*reinterpret_cast<T *>(&elements[element_index].memory));
				OptionalType<T> result{std::move(back)};
				back.~T();

				elements[element_index].is_free.store(true, std::memory_order_release);
				push_task_count.fetch_add(1, std::memory_order_release);

				elements[element_index].is_popper_processing.store(false, std::memory_order_release);
				return result;
			}
		}
	}
};
} // namespace WaitFreeRingBufferUtilities
} // namespace Iyp
