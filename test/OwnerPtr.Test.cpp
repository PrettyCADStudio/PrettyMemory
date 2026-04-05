#include "PrettyMemoryTest.h"

#include <memory>

using namespace prtm;

// static OwnerPtr Create(...)

DEFINE_TEST_BEGIN(OwnerPtrTest, Create, Default)
{
    OwnerPtr<TestableObject> obj = OwnerPtr<TestableObject>::Create();
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_NE(obj.Get(), nullptr);
}
DEFINE_TEST_END

DEFINE_TEST_BEGIN(OwnerPtrTest, Create, ForwardMoveOnlyArgument)
{
    class DerivedObject : public TestableObject
    {
    public:
        DerivedObject(int value, std::unique_ptr<int> movedValue)
            : Value(value), MovedValue(*movedValue)
        {
        }

        int Value;
        int MovedValue;
    };

    OwnerPtr<DerivedObject> obj = OwnerPtr<DerivedObject>::Create(42, std::make_unique<int>(7));
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_NE(obj.Get(), nullptr);
    EXPECT_EQ(obj.Get()->Value, 42);
    EXPECT_EQ(obj.Get()->MovedValue, 7);
}
DEFINE_TEST_END

// OwnerPtr() = default;

DEFINE_TEST_BEGIN(OwnerPtrTest, Constructor, Default)
{
    OwnerPtr<TestableObject> obj;
    EXPECT_EQ(TestableObject::Balance, 0);
    EXPECT_EQ(obj.Get(), nullptr);
}
DEFINE_TEST_END

// OwnerPtr(std::nullptr_t) {}

DEFINE_TEST_BEGIN(OwnerPtrTest, Constructor, FromNullptr)
{
    OwnerPtr<TestableObject> opObj{ nullptr };
    EXPECT_EQ(TestableObject::Balance, 0);
    EXPECT_EQ(opObj.Get(), nullptr);
}
DEFINE_TEST_END

// OwnerPtr(typename OwnerPtr<VT2, DT2>::Pointer pOther)

DEFINE_TEST_BEGIN(OwnerPtrTest, Constructor, FromRawPointer)
{
    TestableObject* pRaw = new TestableObject;
    EXPECT_EQ(TestableObject::Balance, 1);
    OwnerPtr<TestableObject> opObj{ pRaw };
    EXPECT_EQ(TestableObject::Balance, 1);
}
DEFINE_TEST_END

DEFINE_TEST_BEGIN(OwnerPtrTest, Constructor, FromRawPointerNullptr)
{
    TestableObject* pRaw = nullptr;
    OwnerPtr<TestableObject> opObj{ pRaw };
    EXPECT_EQ(TestableObject::Balance, 0);
    EXPECT_EQ(opObj.Get(), nullptr);
}
DEFINE_TEST_END

DEFINE_TEST_BEGIN(OwnerPtrTest, Constructor, FromRawDerivedPointer)
{
    class DerivedObject : public TestableObject
    {
    };

    TestableObject* pRaw = new DerivedObject;
    EXPECT_EQ(TestableObject::Balance, 1);
    OwnerPtr<TestableObject> opObj{ pRaw };
    EXPECT_EQ(TestableObject::Balance, 1);
}
DEFINE_TEST_END

// OwnerPtr(OwnerPtr<VT2, DT2>&& other) noexcept

DEFINE_TEST_BEGIN(OwnerPtrTest, Constructor, Move)
{
    OwnerPtr<TestableObject> opObj1{ new TestableObject };
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_NE(opObj1.Get(), nullptr);

    OwnerPtr<TestableObject> opObj2{ std::move(opObj1) };
    EXPECT_EQ(TestableObject::Balance, 1);

    EXPECT_EQ(opObj1.Get(), nullptr);
    EXPECT_NE(opObj2.Get(), nullptr);
}
DEFINE_TEST_END

DEFINE_TEST_BEGIN(OwnerPtrTest, Constructor, MoveNullptr)
{
    OwnerPtr<TestableObject> opObj1{ nullptr };
    EXPECT_EQ(TestableObject::Balance, 0);
    EXPECT_EQ(opObj1.Get(), nullptr);

    OwnerPtr<TestableObject> opObj2{ std::move(opObj1) };
    EXPECT_EQ(TestableObject::Balance, 0);
    EXPECT_EQ(opObj1.Get(), nullptr);
    EXPECT_EQ(opObj2.Get(), nullptr);
}
DEFINE_TEST_END

DEFINE_TEST_BEGIN(OwnerPtrTest, Constructor, MoveFromEmpty)
{
    OwnerPtr<TestableObject> opObj1;
    EXPECT_EQ(TestableObject::Balance, 0);
    EXPECT_EQ(opObj1.Get(), nullptr);

    OwnerPtr<TestableObject> opObj2{ std::move(opObj1) };
    EXPECT_EQ(TestableObject::Balance, 0);
    EXPECT_EQ(opObj1.Get(), nullptr);
    EXPECT_EQ(opObj2.Get(), nullptr);
}
DEFINE_TEST_END

DEFINE_TEST_BEGIN(OwnerPtrTest, Constructor, MoveFromDerived)
{
    class DerivedObject : public TestableObject
    {
    };

    OwnerPtr<TestableObject> opObj1{ new DerivedObject };
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_NE(opObj1.Get(), nullptr);

    OwnerPtr<TestableObject> opObj2{ std::move(opObj1) };
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_EQ(opObj1.Get(), nullptr);
    EXPECT_NE(opObj2.Get(), nullptr);
}
DEFINE_TEST_END

// OwnerPtr& operator=(OwnerPtr<VT2, DT2>&& other) noexcept

DEFINE_TEST_BEGIN(OwnerPtrTest, Assignment, Move)
{
    OwnerPtr<TestableObject> opObj1{ new TestableObject };
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_NE(opObj1.Get(), nullptr);

    OwnerPtr<TestableObject> opObj2;
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_EQ(opObj2.Get(), nullptr);

    opObj2 = std::move(opObj1);
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_EQ(opObj1.Get(), nullptr);
    EXPECT_NE(opObj2.Get(), nullptr);
}
DEFINE_TEST_END

DEFINE_TEST_BEGIN(OwnerPtrTest, Assignment, MoveNullptr)
{
    OwnerPtr<TestableObject> opObj1{ nullptr };
    EXPECT_EQ(TestableObject::Balance, 0);
    EXPECT_EQ(opObj1.Get(), nullptr);

    OwnerPtr<TestableObject> opObj2;
    EXPECT_EQ(TestableObject::Balance, 0);
    EXPECT_EQ(opObj2.Get(), nullptr);

    opObj2 = std::move(opObj1);
    EXPECT_EQ(TestableObject::Balance, 0);
    EXPECT_EQ(opObj1.Get(), nullptr);
    EXPECT_EQ(opObj2.Get(), nullptr);
}
DEFINE_TEST_END

DEFINE_TEST_BEGIN(OwnerPtrTest, Assignment, MoveFromEmpty)
{
    OwnerPtr<TestableObject> opObj1;
    EXPECT_EQ(TestableObject::Balance, 0);
    EXPECT_EQ(opObj1.Get(), nullptr);

    OwnerPtr<TestableObject> opObj2;
    EXPECT_EQ(TestableObject::Balance, 0);
    EXPECT_EQ(opObj2.Get(), nullptr);

    opObj2 = std::move(opObj1);
    EXPECT_EQ(TestableObject::Balance, 0);
    EXPECT_EQ(opObj1.Get(), nullptr);
    EXPECT_EQ(opObj2.Get(), nullptr);
}
DEFINE_TEST_END

DEFINE_TEST_BEGIN(OwnerPtrTest, Assignment, MoveFromDerived)
{
    class DerivedObject : public TestableObject
    {
    };

    OwnerPtr<TestableObject> opObj1{ new DerivedObject };
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_NE(opObj1.Get(), nullptr);

    OwnerPtr<TestableObject> opObj2;
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_EQ(opObj2.Get(), nullptr);

    opObj2 = std::move(opObj1);
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_EQ(opObj1.Get(), nullptr);
    EXPECT_NE(opObj2.Get(), nullptr);
}
DEFINE_TEST_END

// ConstPointer Get() const

DEFINE_TEST_BEGIN(OwnerPtrTest, ConstGet, 0)
{
    const OwnerPtr<TestableObject> obj;
    EXPECT_EQ(TestableObject::Balance, 0);
    EXPECT_EQ(obj.Get(), nullptr);
}
DEFINE_TEST_END

// Pointer Data()

DEFINE_TEST_BEGIN(OwnerPtrTest, Data, 0)
{
    class DerivedObject : public TestableObject
    {
    public:
        int Value{ 42 };
    };

    OwnerPtr<DerivedObject> obj{ new DerivedObject };
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_NE(obj.Data(), nullptr);
    EXPECT_EQ(obj.Data()->Value, 42);
}
DEFINE_TEST_END

// ConstPointer Data() const

DEFINE_TEST_BEGIN(OwnerPtrTest, ConstData, 0)
{
    class DerivedObject : public TestableObject
    {
    public:
        int Value{ 42 };
    };

    const OwnerPtr<DerivedObject> obj{ new DerivedObject };
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_NE(obj.Data(), nullptr);
    EXPECT_EQ(obj.Data()->Value, 42);
}
DEFINE_TEST_END

// Reference operator*()

DEFINE_TEST_BEGIN(OwnerPtrTest, Dereference, 0)
{
    class DerivedObject : public TestableObject
    {
    public:
        int Value{ 42 };
    };

    OwnerPtr<DerivedObject> obj{ new DerivedObject };
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_EQ((*obj).Value, 42);

    (*obj).Value = 100;
    EXPECT_EQ(obj.Get()->Value, 100);
}
DEFINE_TEST_END

// ConstReference operator*() const

DEFINE_TEST_BEGIN(OwnerPtrTest, ConstDereference, 0)
{
    class DerivedObject : public TestableObject
    {
    public:
        int Value{ 42 };
    };

    const OwnerPtr<DerivedObject> obj{ new DerivedObject };
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_EQ((*obj).Value, 42);
}
DEFINE_TEST_END

// operator bool() const

DEFINE_TEST_BEGIN(OwnerPtrTest, Bool, 0)
{
    OwnerPtr<TestableObject> emptyObj;
    EXPECT_EQ(TestableObject::Balance, 0);
    EXPECT_FALSE(emptyObj);

    OwnerPtr<TestableObject> obj{ new TestableObject };
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_TRUE(obj);
}
DEFINE_TEST_END

// bool IsNull() const

DEFINE_TEST_BEGIN(OwnerPtrTest, IsNull, 0)
{
    OwnerPtr<TestableObject> emptyObj;
    EXPECT_EQ(TestableObject::Balance, 0);
    EXPECT_TRUE(emptyObj.IsNull());

    OwnerPtr<TestableObject> obj{ new TestableObject };
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_FALSE(obj.IsNull());
}
DEFINE_TEST_END

// void Nullify()

DEFINE_TEST_BEGIN(OwnerPtrTest, Nullify, 0)
{
    OwnerPtr<TestableObject> obj{ new TestableObject };
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_NE(obj.Get(), nullptr);

    obj.Nullify();
    EXPECT_EQ(TestableObject::Balance, 0);
    EXPECT_EQ(obj.Get(), nullptr);
}
DEFINE_TEST_END

// void Reset()

DEFINE_TEST_BEGIN(OwnerPtrTest, Reset, 0)
{
    OwnerPtr<TestableObject> obj{ new TestableObject };
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_NE(obj.Get(), nullptr);

    obj.Reset();
    EXPECT_EQ(TestableObject::Balance, 0);
    EXPECT_EQ(obj.Get(), nullptr);
}
DEFINE_TEST_END

// void Reset(std::nullptr_t)

DEFINE_TEST_BEGIN(OwnerPtrTest, Reset, Nullptr)
{
    OwnerPtr<TestableObject> obj{ new TestableObject };
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_NE(obj.Get(), nullptr);

    obj.Reset(nullptr);
    EXPECT_EQ(TestableObject::Balance, 0);
    EXPECT_EQ(obj.Get(), nullptr);
}
DEFINE_TEST_END

// void Reset(typename OwnerPtr<VT2, DT2>::Pointer pNew)

DEFINE_TEST_BEGIN(OwnerPtrTest, Reset, FromRawPointer)
{
    OwnerPtr<TestableObject> obj;
    EXPECT_EQ(TestableObject::Balance, 0);
    EXPECT_EQ(obj.Get(), nullptr);

    TestableObject* pRaw = new TestableObject;
    EXPECT_EQ(TestableObject::Balance, 1);

    obj.Reset(pRaw);
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_EQ(obj.Get(), pRaw);
}
DEFINE_TEST_END

DEFINE_TEST_BEGIN(OwnerPtrTest, Reset, ReplaceRawPointer)
{
    OwnerPtr<TestableObject> obj{ new TestableObject };
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_NE(obj.Get(), nullptr);

    TestableObject* pRaw = new TestableObject;
    EXPECT_EQ(TestableObject::Balance, 2);

    obj.Reset(pRaw);
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_EQ(obj.Get(), pRaw);
}
DEFINE_TEST_END

DEFINE_TEST_BEGIN(OwnerPtrTest, Reset, FromRawPointerNullptr)
{
    OwnerPtr<TestableObject> obj;
    EXPECT_EQ(TestableObject::Balance, 0);
    EXPECT_EQ(obj.Get(), nullptr);

    TestableObject* pRaw = nullptr;
    obj.Reset(pRaw);
    EXPECT_EQ(TestableObject::Balance, 0);
    EXPECT_EQ(obj.Get(), nullptr);
}
DEFINE_TEST_END

DEFINE_TEST_BEGIN(OwnerPtrTest, Reset, FromRawDerivedPointer)
{
    class DerivedObject : public TestableObject
    {
    };

    OwnerPtr<TestableObject> obj;
    EXPECT_EQ(TestableObject::Balance, 0);
    EXPECT_EQ(obj.Get(), nullptr);

    TestableObject* pRaw = new DerivedObject;
    EXPECT_EQ(TestableObject::Balance, 1);

    obj.Reset(pRaw);
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_EQ(obj.Get(), pRaw);
}
DEFINE_TEST_END

// [[nodiscard]] Pointer Release()

DEFINE_TEST_BEGIN(OwnerPtrTest, Release, 0)
{
    OwnerPtr<TestableObject> obj{ new TestableObject };
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_NE(obj.Get(), nullptr);

    TestableObject* pReleased = obj.Release();
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_EQ(obj.Get(), nullptr);
    EXPECT_NE(pReleased, nullptr);

    delete pReleased;
    EXPECT_EQ(TestableObject::Balance, 0);
}
DEFINE_TEST_END

DEFINE_TEST_BEGIN(OwnerPtrTest, Release, FromEmpty)
{
    OwnerPtr<TestableObject> obj;
    EXPECT_EQ(TestableObject::Balance, 0);
    EXPECT_EQ(obj.Get(), nullptr);

    TestableObject* pReleased = obj.Release();
    EXPECT_EQ(TestableObject::Balance, 0);
    EXPECT_EQ(obj.Get(), nullptr);
    EXPECT_EQ(pReleased, nullptr);
}
DEFINE_TEST_END

// void Swap(OwnerPtr<VT, DT2>& other) noexcept

DEFINE_TEST_BEGIN(OwnerPtrTest, Swap, 0)
{
    TestableObject* pRaw1 = new TestableObject;
    TestableObject* pRaw2 = new TestableObject;
    EXPECT_EQ(TestableObject::Balance, 2);

    OwnerPtr<TestableObject> obj1{ pRaw1 };
    OwnerPtr<TestableObject> obj2{ pRaw2 };
    EXPECT_EQ(TestableObject::Balance, 2);
    EXPECT_EQ(obj1.Get(), pRaw1);
    EXPECT_EQ(obj2.Get(), pRaw2);

    obj1.Swap(obj2);
    EXPECT_EQ(TestableObject::Balance, 2);
    EXPECT_EQ(obj1.Get(), pRaw2);
    EXPECT_EQ(obj2.Get(), pRaw1);
}
DEFINE_TEST_END

// [[nodiscard]] OwnerPtr<VT2, DT2> Transfer()

DEFINE_TEST_BEGIN(OwnerPtrTest, Transfer, FromThisType)
{
    OwnerPtr<TestableObject> obj1{ new TestableObject };
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_NE(obj1.Get(), nullptr);

    OwnerPtr<TestableObject> obj2 = obj1.Transfer();
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_EQ(obj1.Get(), nullptr);
    EXPECT_NE(obj2.Get(), nullptr);
}
DEFINE_TEST_END

DEFINE_TEST_BEGIN(OwnerPtrTest, Transfer, FromEmpty)
{
    OwnerPtr<TestableObject> obj1;
    EXPECT_EQ(TestableObject::Balance, 0);
    EXPECT_EQ(obj1.Get(), nullptr);

    OwnerPtr<TestableObject> obj2 = obj1.Transfer();
    EXPECT_EQ(TestableObject::Balance, 0);
    EXPECT_EQ(obj1.Get(), nullptr);
    EXPECT_EQ(obj2.Get(), nullptr);
}
DEFINE_TEST_END

DEFINE_TEST_BEGIN(OwnerPtrTest, Transfer, FromDerivedType)
{
    class DerivedObject : public TestableObject
    {
    };

    OwnerPtr<DerivedObject> obj1{ new DerivedObject };
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_NE(obj1.Get(), nullptr);

    OwnerPtr<TestableObject> obj2 = obj1.Transfer();
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_EQ(obj1.Get(), nullptr);
    EXPECT_NE(obj2.Get(), nullptr);
}
DEFINE_TEST_END

// [[nodiscard]] OwnerPtr<VT2, DT2> Cast()

DEFINE_TEST_BEGIN(OwnerPtrTest, Cast, ToSameType)
{
    OwnerPtr<TestableObject> obj1{ new TestableObject };
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_NE(obj1.Get(), nullptr);

    OwnerPtr<TestableObject> obj2 = obj1.Cast();
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_EQ(obj1.Get(), nullptr);
    EXPECT_NE(obj2.Get(), nullptr);
}
DEFINE_TEST_END

DEFINE_TEST_BEGIN(OwnerPtrTest, Cast, FromEmpty)
{
    OwnerPtr<TestableObject> obj1;
    EXPECT_EQ(TestableObject::Balance, 0);
    EXPECT_EQ(obj1.Get(), nullptr);

    OwnerPtr<TestableObject> obj2 = obj1.Cast();
    EXPECT_EQ(TestableObject::Balance, 0);
    EXPECT_EQ(obj1.Get(), nullptr);
    EXPECT_EQ(obj2.Get(), nullptr);
}
DEFINE_TEST_END

DEFINE_TEST_BEGIN(OwnerPtrTest, Cast, ToBaseType)
{
    class DerivedObject : public TestableObject
    {
    };

    OwnerPtr<DerivedObject> obj1{ new DerivedObject };
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_NE(obj1.Get(), nullptr);

    OwnerPtr<TestableObject> obj2 = obj1.Cast();
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_EQ(obj1.Get(), nullptr);
    EXPECT_NE(obj2.Get(), nullptr);
}
DEFINE_TEST_END

DEFINE_TEST_BEGIN(OwnerPtrTest, Cast, ToDerivedType)
{
    class DerivedObject : public TestableObject
    {
    };

    OwnerPtr<TestableObject> obj1{ new DerivedObject };
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_NE(obj1.Get(), nullptr);

    OwnerPtr<DerivedObject> obj2 = obj1.Cast<DerivedObject>();
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_EQ(obj1.Get(), nullptr);
    EXPECT_NE(obj2.Get(), nullptr);
}
DEFINE_TEST_END

DEFINE_TEST_BEGIN(OwnerPtrTest, Cast, ToRelatedType)
{
    class BaseObject
    {
    public:
        virtual ~BaseObject() = default;
    };

    class DerivedObject : public BaseObject, public TestableObject
    {
    };

    OwnerPtr<TestableObject> obj1{ new DerivedObject };
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_NE(obj1.Get(), nullptr);

    OwnerPtr<BaseObject> obj2 = obj1.Cast<BaseObject>();
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_EQ(obj1.Get(), nullptr);
    EXPECT_NE(obj2.Get(), nullptr);
}
DEFINE_TEST_END

DEFINE_TEST_BEGIN(OwnerPtrTest, Cast, ToUnrelatedType)
{
    class UnrelatedObject
    {
    };
    OwnerPtr<TestableObject> obj1{ new TestableObject };
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_NE(obj1.Get(), nullptr);

    OwnerPtr<UnrelatedObject> obj2 = obj1.Cast<UnrelatedObject>();
    EXPECT_EQ(TestableObject::Balance, 0);
    EXPECT_EQ(obj1.Get(), nullptr);
    EXPECT_EQ(obj2.Get(), nullptr);
}
DEFINE_TEST_END

// OwnerPtr comparison operators

DEFINE_TEST_BEGIN(OwnerPtrTest, Comparison, OwnerPtr)
{
    TestableObject* pRaw1 = new TestableObject;
    TestableObject* pRaw2 = new TestableObject;
    EXPECT_EQ(TestableObject::Balance, 2);

    OwnerPtr<TestableObject> obj1{ pRaw1 };
    OwnerPtr<TestableObject> obj2{ pRaw2 };
    EXPECT_EQ(TestableObject::Balance, 2);

    EXPECT_EQ(obj1 == obj2, pRaw1 == pRaw2);
    EXPECT_EQ(obj1 != obj2, pRaw1 != pRaw2);
    EXPECT_EQ(obj1 < obj2, pRaw1 < pRaw2);
    EXPECT_EQ(obj1 > obj2, pRaw1 > pRaw2);
    EXPECT_EQ(obj1 <= obj2, pRaw1 <= pRaw2);
    EXPECT_EQ(obj1 >= obj2, pRaw1 >= pRaw2);
}
DEFINE_TEST_END

DEFINE_TEST_BEGIN(OwnerPtrTest, Comparison, NullptrOnRight)
{
    OwnerPtr<TestableObject> emptyObj;
    EXPECT_EQ(TestableObject::Balance, 0);

    EXPECT_EQ(emptyObj == nullptr, emptyObj.Get() == nullptr);
    EXPECT_EQ(emptyObj != nullptr, emptyObj.Get() != nullptr);
    EXPECT_EQ(emptyObj < nullptr, emptyObj.Get() < nullptr);
    EXPECT_EQ(emptyObj > nullptr, emptyObj.Get() > nullptr);
    EXPECT_EQ(emptyObj <= nullptr, emptyObj.Get() <= nullptr);
    EXPECT_EQ(emptyObj >= nullptr, emptyObj.Get() >= nullptr);
}
DEFINE_TEST_END

DEFINE_TEST_BEGIN(OwnerPtrTest, Comparison, NullptrOnLeft)
{
    OwnerPtr<TestableObject> emptyObj;
    EXPECT_EQ(TestableObject::Balance, 0);

    EXPECT_EQ(nullptr == emptyObj, nullptr == emptyObj.Get());
    EXPECT_EQ(nullptr != emptyObj, nullptr != emptyObj.Get());
    EXPECT_EQ(nullptr < emptyObj, nullptr < emptyObj.Get());
    EXPECT_EQ(nullptr > emptyObj, nullptr > emptyObj.Get());
    EXPECT_EQ(nullptr <= emptyObj, nullptr <= emptyObj.Get());
    EXPECT_EQ(nullptr >= emptyObj, nullptr >= emptyObj.Get());
}
DEFINE_TEST_END
