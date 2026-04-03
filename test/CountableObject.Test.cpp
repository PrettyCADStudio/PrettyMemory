#include <gtest/gtest.h>
#include "CountableObject.h"

TEST(a, b)
{
    TestableObject obj1;
    EXPECT_EQ(TestableObject::Balance, 1);
}

TEST(a, c)
{
    TestableObject obj1;
    TestableObject obj2;
    EXPECT_EQ(TestableObject::Balance, 2);
}
