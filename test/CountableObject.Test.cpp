#include "PrettyMemoryTest.h"

DEFINE_TEST_BEGIN(CountableObject, Balance, 1)
{
    TestableObject obj1;
    EXPECT_EQ(TestableObject::Balance, 1);
}
DEFINE_TEST_END

DEFINE_TEST_BEGIN(CountableObject, Balance, 2)
{
    TestableObject obj1;
    TestableObject obj2;
    EXPECT_EQ(TestableObject::Balance, 2);
}
DEFINE_TEST_END
