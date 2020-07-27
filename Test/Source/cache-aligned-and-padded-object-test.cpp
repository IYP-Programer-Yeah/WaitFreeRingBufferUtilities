#define _ENABLE_EXTENDED_ALIGNED_STORAGE

#include <Iyp/WaitFreeRingBufferUtilities/details/cache-aligned-and-padded-object.inl>
#include <gtest/gtest.h>

#include <type_traits>

namespace Iyp
{
namespace CacheAlignedAndPaddedObjectTest
{
TEST(CacheAlignedAndPaddedObjectTest, UnalignedWithUndevisableSizeObject)
{
    static constexpr std::size_t Alignment = Iyp::WaitFreeRingBufferUtilities::Details::DESTRUCTIVE_INTERFERENCE_SIZE / 2;
    static constexpr std::size_t Size = Iyp::WaitFreeRingBufferUtilities::Details::DESTRUCTIVE_INTERFERENCE_SIZE / 2;

    struct UnalignedWithUndevisableSizeObjectType
    {
        typename std::aligned_storage<Size, Alignment>::type data;
    };
    EXPECT_EQ(Alignment, alignof(UnalignedWithUndevisableSizeObjectType));
    EXPECT_EQ(Size, sizeof(UnalignedWithUndevisableSizeObjectType));

    static constexpr std::size_t ExpectedAlignment = Iyp::WaitFreeRingBufferUtilities::Details::DESTRUCTIVE_INTERFERENCE_SIZE;
    static constexpr std::size_t ExpectedSize = Iyp::WaitFreeRingBufferUtilities::Details::DESTRUCTIVE_INTERFERENCE_SIZE;
    using CacheAlignedAndPaddedObjectType = Iyp::WaitFreeRingBufferUtilities::Details::CacheAlignedAndPaddedObject<UnalignedWithUndevisableSizeObjectType>;
    EXPECT_EQ(ExpectedAlignment, alignof(CacheAlignedAndPaddedObjectType));
    EXPECT_EQ(ExpectedSize, sizeof(CacheAlignedAndPaddedObjectType));
}

TEST(CacheAlignedAndPaddedObjectTest, UnalignedWithDevisableSizeObject)
{
    static constexpr std::size_t Alignment = Iyp::WaitFreeRingBufferUtilities::Details::DESTRUCTIVE_INTERFERENCE_SIZE / 2;
    static constexpr std::size_t Size = Iyp::WaitFreeRingBufferUtilities::Details::DESTRUCTIVE_INTERFERENCE_SIZE * 3;

    struct UnalignedWithUndevisableSizeObjectType
    {
        typename std::aligned_storage<Size, Alignment>::type data;
    };
    EXPECT_EQ(Alignment, alignof(UnalignedWithUndevisableSizeObjectType));
    EXPECT_EQ(Size, sizeof(UnalignedWithUndevisableSizeObjectType));

    static constexpr std::size_t ExpectedAlignment = Iyp::WaitFreeRingBufferUtilities::Details::DESTRUCTIVE_INTERFERENCE_SIZE;
    static constexpr std::size_t ExpectedSize = Size;
    using CacheAlignedAndPaddedObjectType = Iyp::WaitFreeRingBufferUtilities::Details::CacheAlignedAndPaddedObject<UnalignedWithUndevisableSizeObjectType>;
    EXPECT_EQ(ExpectedAlignment, alignof(CacheAlignedAndPaddedObjectType));
    EXPECT_EQ(ExpectedSize, sizeof(CacheAlignedAndPaddedObjectType));
}

TEST(CacheAlignedAndPaddedObjectTest, DISABLED_AlignedWithUndevisableSizeObject)
{
    static constexpr std::size_t Alignment = Iyp::WaitFreeRingBufferUtilities::Details::DESTRUCTIVE_INTERFERENCE_SIZE;
    static constexpr std::size_t Size = (Iyp::WaitFreeRingBufferUtilities::Details::DESTRUCTIVE_INTERFERENCE_SIZE / 2) * 3;

    struct UnalignedWithUndevisableSizeObjectType
    {
        typename std::aligned_storage<Size, Alignment>::type data;
    };
    EXPECT_EQ(Alignment, alignof(UnalignedWithUndevisableSizeObjectType));
    EXPECT_EQ(Size, sizeof(UnalignedWithUndevisableSizeObjectType));

    static constexpr std::size_t ExpectedAlignment = Alignment;
    static constexpr std::size_t ExpectedSize = Iyp::WaitFreeRingBufferUtilities::Details::DESTRUCTIVE_INTERFERENCE_SIZE * 2;
    using CacheAlignedAndPaddedObjectType = Iyp::WaitFreeRingBufferUtilities::Details::CacheAlignedAndPaddedObject<UnalignedWithUndevisableSizeObjectType>;
    EXPECT_EQ(ExpectedAlignment, alignof(CacheAlignedAndPaddedObjectType));
    EXPECT_EQ(ExpectedSize, sizeof(CacheAlignedAndPaddedObjectType));
}

TEST(CacheAlignedAndPaddedObjectTest, AlignedWithDevisableSizeObject)
{
    static constexpr std::size_t Alignment = Iyp::WaitFreeRingBufferUtilities::Details::DESTRUCTIVE_INTERFERENCE_SIZE;
    static constexpr std::size_t Size = Iyp::WaitFreeRingBufferUtilities::Details::DESTRUCTIVE_INTERFERENCE_SIZE * 3;

    struct UnalignedWithUndevisableSizeObjectType
    {
        typename std::aligned_storage<Size, Alignment>::type data;
    };
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
