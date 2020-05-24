#include <Iyp/WaitFreeRingBufferUtilities/details/cache-aligned-and-padded-object.inl>
#include <gtest/gtest.h>

namespace Iyp
{
namespace CacheAlignedAndPaddedObjectTest
{
template <std::size_t Alignment, std::size_t Size>
struct alignas(Alignment) SizableAlignableObject
{
    std::uint8_t padding[Size];
};

TEST(CacheAlignedAndPaddedObjectTest, UnalignedWithUndevisableSizeObject)
{
    static constexpr std::size_t Alignment = Iyp::WaitFreeRingBufferUtilities::Details::CACHE_ALIGNMENT / 2;
    static constexpr std::size_t Size = Iyp::WaitFreeRingBufferUtilities::Details::CACHE_LINE_SIZE / 2;

    using UnalignedWithUndevisableSizeObjectType = SizableAlignableObject<Alignment, Size>;
    EXPECT_EQ(Alignment, alignof(UnalignedWithUndevisableSizeObjectType));
    EXPECT_EQ(Size, sizeof(UnalignedWithUndevisableSizeObjectType));

    static constexpr std::size_t ExpectedAlignment = Iyp::WaitFreeRingBufferUtilities::Details::CACHE_ALIGNMENT;
    static constexpr std::size_t ExpectedSize = Iyp::WaitFreeRingBufferUtilities::Details::CACHE_LINE_SIZE;
    using CacheAlignedAndPaddedObjectType = Iyp::WaitFreeRingBufferUtilities::Details::CacheAlignedAndPaddedObject<UnalignedWithUndevisableSizeObjectType>;
    EXPECT_EQ(ExpectedAlignment, alignof(CacheAlignedAndPaddedObjectType));
    EXPECT_EQ(ExpectedSize, sizeof(CacheAlignedAndPaddedObjectType));
}

TEST(CacheAlignedAndPaddedObjectTest, UnalignedWithDevisableSizeObject)
{
    static constexpr std::size_t Alignment = Iyp::WaitFreeRingBufferUtilities::Details::CACHE_ALIGNMENT / 2;
    static constexpr std::size_t Size = Iyp::WaitFreeRingBufferUtilities::Details::CACHE_LINE_SIZE * 3;

    using UnalignedWithUndevisableSizeObjectType = SizableAlignableObject<Alignment, Size>;
    EXPECT_EQ(Alignment, alignof(UnalignedWithUndevisableSizeObjectType));
    EXPECT_EQ(Size, sizeof(UnalignedWithUndevisableSizeObjectType));

    static constexpr std::size_t ExpectedAlignment = Iyp::WaitFreeRingBufferUtilities::Details::CACHE_ALIGNMENT;
    static constexpr std::size_t ExpectedSize = Size;
    using CacheAlignedAndPaddedObjectType = Iyp::WaitFreeRingBufferUtilities::Details::CacheAlignedAndPaddedObject<UnalignedWithUndevisableSizeObjectType>;
    EXPECT_EQ(ExpectedAlignment, alignof(CacheAlignedAndPaddedObjectType));
    EXPECT_EQ(ExpectedSize, sizeof(CacheAlignedAndPaddedObjectType));
}

TEST(CacheAlignedAndPaddedObjectTest, DISABLED_AlignedWithUndevisableSizeObject)
{
    static constexpr std::size_t Alignment = Iyp::WaitFreeRingBufferUtilities::Details::CACHE_ALIGNMENT;
    static constexpr std::size_t Size = (Iyp::WaitFreeRingBufferUtilities::Details::CACHE_LINE_SIZE / 2) * 3;

    using UnalignedWithUndevisableSizeObjectType = SizableAlignableObject<Alignment, Size>;
    EXPECT_EQ(Alignment, alignof(UnalignedWithUndevisableSizeObjectType));
    EXPECT_EQ(Size, sizeof(UnalignedWithUndevisableSizeObjectType));

    static constexpr std::size_t ExpectedAlignment = Alignment;
    static constexpr std::size_t ExpectedSize = Iyp::WaitFreeRingBufferUtilities::Details::CACHE_LINE_SIZE * 2;
    using CacheAlignedAndPaddedObjectType = Iyp::WaitFreeRingBufferUtilities::Details::CacheAlignedAndPaddedObject<UnalignedWithUndevisableSizeObjectType>;
    EXPECT_EQ(ExpectedAlignment, alignof(CacheAlignedAndPaddedObjectType));
    EXPECT_EQ(ExpectedSize, sizeof(CacheAlignedAndPaddedObjectType));
}

TEST(CacheAlignedAndPaddedObjectTest, AlignedWithDevisableSizeObject)
{
    static constexpr std::size_t Alignment = Iyp::WaitFreeRingBufferUtilities::Details::CACHE_ALIGNMENT;
    static constexpr std::size_t Size = Iyp::WaitFreeRingBufferUtilities::Details::CACHE_LINE_SIZE * 3;

    using UnalignedWithUndevisableSizeObjectType = SizableAlignableObject<Alignment, Size>;
    EXPECT_EQ(Alignment, alignof(UnalignedWithUndevisableSizeObjectType));
    EXPECT_EQ(Size, sizeof(UnalignedWithUndevisableSizeObjectType));

    static constexpr std::size_t ExpectedAlignment = Alignment;
    static constexpr std::size_t ExpectedSize = Size;
    using CacheAlignedAndPaddedObjectType = Iyp::WaitFreeRingBufferUtilities::Details::CacheAlignedAndPaddedObject<UnalignedWithUndevisableSizeObjectType>;
    EXPECT_EQ(ExpectedAlignment, alignof(CacheAlignedAndPaddedObjectType));
    EXPECT_EQ(ExpectedSize, sizeof(CacheAlignedAndPaddedObjectType));
}
} // namespace CacheAlignedAndPaddedObjectTest
} // namespace Iyp
