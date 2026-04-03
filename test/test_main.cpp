#include <gtest/gtest.h>

#include "PrettyMemory.h"

TEST(ControlBlockTests, StartsWithNullPointersAndZeroShadowCount) {
    prtm::detail::ControlBlock controlBlock;

    EXPECT_EQ(controlBlock.Data, nullptr);
    EXPECT_FALSE(static_cast<bool>(controlBlock.Deleter));
    EXPECT_EQ(controlBlock.ShadowCount, 0U);
}

TEST(DefaultDeleterTests, DeletesTypedPointerViaVoidPointerInterface) {
    int* value = new int(42);

    prtm::detail::DefaultDeleter<int> deleter;
    deleter(value);

    SUCCEED();
}

TEST(ControlBlockTests, StoresDataDeleterAndShadowCount) {
    int value = 42;
    int deletedValue = 0;

    prtm::detail::ControlBlock controlBlock;
    controlBlock.Data = &value;
    controlBlock.ShadowCount = 3;
    controlBlock.Deleter = [&deletedValue](void* rawPtr) {
        deletedValue = *static_cast<int*>(rawPtr);
    };

    ASSERT_TRUE(static_cast<bool>(controlBlock.Deleter));
    controlBlock.Deleter(controlBlock.Data);

    EXPECT_EQ(controlBlock.Data, &value);
    EXPECT_EQ(controlBlock.ShadowCount, 3U);
    EXPECT_EQ(deletedValue, 42);
}
