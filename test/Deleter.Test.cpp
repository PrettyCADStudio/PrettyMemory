#include "PrettyMemoryTest.h"

using namespace prtm;

namespace
{
    template<typename T>
    struct TrackingDeleter
    {
        static inline int DeleteCount{ 0 };

        void operator()(void* ptr) const
        {
            ++DeleteCount;
            delete static_cast<T*>(ptr);
        }
    };
}

// OwnerPtr custom deleter behavior

DEFINE_TEST_BEGIN(DeleterTest, Constructor, FromRawPointer)
{
    using Deleter = TrackingDeleter<TestableObject>;
    Deleter::DeleteCount = 0;

    {
        TestableObject* pRaw = new TestableObject;
        EXPECT_EQ(TestableObject::Balance, 1);

        OwnerPtr<TestableObject, Deleter> obj{ pRaw };
        EXPECT_EQ(obj.Get(), pRaw);
        EXPECT_EQ(Deleter::DeleteCount, 0);
    }

    EXPECT_EQ(Deleter::DeleteCount, 1);
}
DEFINE_TEST_END

DEFINE_TEST_BEGIN(DeleterTest, Nullify, 0)
{
    using Deleter = TrackingDeleter<TestableObject>;
    Deleter::DeleteCount = 0;

    OwnerPtr<TestableObject, Deleter> obj{ new TestableObject };
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_NE(obj.Get(), nullptr);
    EXPECT_EQ(Deleter::DeleteCount, 0);

    obj.Nullify();
    EXPECT_EQ(TestableObject::Balance, 0);
    EXPECT_EQ(obj.Get(), nullptr);
    EXPECT_EQ(Deleter::DeleteCount, 1);
}
DEFINE_TEST_END

DEFINE_TEST_BEGIN(DeleterTest, Reset, ReplaceRawPointer)
{
    using Deleter = TrackingDeleter<TestableObject>;
    Deleter::DeleteCount = 0;

    {
        OwnerPtr<TestableObject, Deleter> obj{ new TestableObject };
        EXPECT_EQ(TestableObject::Balance, 1);
        EXPECT_EQ(Deleter::DeleteCount, 0);

        TestableObject* pRaw = new TestableObject;
        EXPECT_EQ(TestableObject::Balance, 2);

        obj.Reset(pRaw);
        EXPECT_EQ(TestableObject::Balance, 1);
        EXPECT_EQ(obj.Get(), pRaw);
        EXPECT_EQ(Deleter::DeleteCount, 1);
    }

    EXPECT_EQ(Deleter::DeleteCount, 2);
}
DEFINE_TEST_END

DEFINE_TEST_BEGIN(DeleterTest, Release, 0)
{
    using Deleter = TrackingDeleter<TestableObject>;
    Deleter::DeleteCount = 0;

    OwnerPtr<TestableObject, Deleter> obj{ new TestableObject };
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_NE(obj.Get(), nullptr);

    TestableObject* pReleased = obj.Release();
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_EQ(obj.Get(), nullptr);
    EXPECT_NE(pReleased, nullptr);
    EXPECT_EQ(Deleter::DeleteCount, 0);

    delete pReleased;
    EXPECT_EQ(TestableObject::Balance, 0);
    EXPECT_EQ(Deleter::DeleteCount, 0);
}
DEFINE_TEST_END

DEFINE_TEST_BEGIN(DeleterTest, Transfer, PreserveCustomDeleter)
{
    using Deleter = TrackingDeleter<TestableObject>;
    Deleter::DeleteCount = 0;

    {
        OwnerPtr<TestableObject, Deleter> obj1{ new TestableObject };
        EXPECT_EQ(TestableObject::Balance, 1);
        EXPECT_NE(obj1.Get(), nullptr);

        OwnerPtr<TestableObject> obj2 = obj1.Transfer();
        EXPECT_EQ(TestableObject::Balance, 1);
        EXPECT_EQ(obj1.Get(), nullptr);
        EXPECT_NE(obj2.Get(), nullptr);
        EXPECT_EQ(Deleter::DeleteCount, 0);
    }

    EXPECT_EQ(Deleter::DeleteCount, 1);
}
DEFINE_TEST_END

DEFINE_TEST_BEGIN(DeleterTest, Cast, ToUnrelatedType)
{
    class UnrelatedObject
    {
    };

    using Deleter = TrackingDeleter<TestableObject>;
    Deleter::DeleteCount = 0;

    OwnerPtr<TestableObject, Deleter> obj1{ new TestableObject };
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_NE(obj1.Get(), nullptr);

    OwnerPtr<UnrelatedObject> obj2 = obj1.Cast<UnrelatedObject>();
    EXPECT_EQ(TestableObject::Balance, 0);
    EXPECT_EQ(obj1.Get(), nullptr);
    EXPECT_EQ(obj2.Get(), nullptr);
    EXPECT_EQ(Deleter::DeleteCount, 1);
}
DEFINE_TEST_END
