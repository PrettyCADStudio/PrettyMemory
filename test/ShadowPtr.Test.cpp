#include "PrettyMemoryTest.h"

using namespace prtm;

// ShadowPtr() = default;

DEFINE_TEST_BEGIN(ShadowPtrTest, Constructor, Default)
{
    ShadowPtr<TestableObject> obj;
    EXPECT_EQ(TestableObject::Balance, 0);
    EXPECT_EQ(obj.Get(), nullptr);
    EXPECT_EQ(obj.ShadowCount(), 0);
}
DEFINE_TEST_END

// ShadowPtr(std::nullptr_t) {}

DEFINE_TEST_BEGIN(ShadowPtrTest, Constructor, FromNullptr)
{
    ShadowPtr<TestableObject> obj{ nullptr };
    EXPECT_EQ(TestableObject::Balance, 0);
    EXPECT_EQ(obj.Get(), nullptr);
    EXPECT_EQ(obj.ShadowCount(), 0);
}
DEFINE_TEST_END

// ShadowPtr(const ShadowPtr<T2>& other)

DEFINE_TEST_BEGIN(ShadowPtrTest, Constructor, Copy)
{
    OwnerPtr<TestableObject> owner{ new TestableObject };
    ShadowPtr<TestableObject> obj1 = owner.Shadow();
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_NE(obj1.Get(), nullptr);
    EXPECT_EQ(owner.ShadowCount(), 1);

    ShadowPtr<TestableObject> obj2{ obj1 };
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_EQ(obj2.Get(), obj1.Get());
    EXPECT_EQ(owner.ShadowCount(), 2);
    EXPECT_EQ(obj1.ShadowCount(), 2);
    EXPECT_EQ(obj2.ShadowCount(), 2);
}
DEFINE_TEST_END

DEFINE_TEST_BEGIN(ShadowPtrTest, Constructor, CopyFromEmpty)
{
    ShadowPtr<TestableObject> obj1;
    EXPECT_EQ(TestableObject::Balance, 0);
    EXPECT_EQ(obj1.Get(), nullptr);

    ShadowPtr<TestableObject> obj2{ obj1 };
    EXPECT_EQ(TestableObject::Balance, 0);
    EXPECT_EQ(obj1.Get(), nullptr);
    EXPECT_EQ(obj2.Get(), nullptr);
}
DEFINE_TEST_END

DEFINE_TEST_BEGIN(ShadowPtrTest, Constructor, CopyFromDerived)
{
    class DerivedObject : public TestableObject
    {
    };

    OwnerPtr<DerivedObject> owner{ new DerivedObject };
    ShadowPtr<DerivedObject> obj1 = owner.Shadow();
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_NE(obj1.Get(), nullptr);
    EXPECT_EQ(owner.ShadowCount(), 1);

    ShadowPtr<TestableObject> obj2{ obj1 };
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_EQ(obj2.Get(), obj1.Get());
    EXPECT_EQ(owner.ShadowCount(), 2);
    EXPECT_EQ(obj2.ShadowCount(), 2);
}
DEFINE_TEST_END

// ShadowPtr(ShadowPtr<T2>&& other) noexcept

DEFINE_TEST_BEGIN(ShadowPtrTest, Constructor, Move)
{
    OwnerPtr<TestableObject> owner{ new TestableObject };
    ShadowPtr<TestableObject> obj1 = owner.Shadow();
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_NE(obj1.Get(), nullptr);
    EXPECT_EQ(owner.ShadowCount(), 1);

    ShadowPtr<TestableObject> obj2{ std::move(obj1) };
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_EQ(obj1.Get(), nullptr);
    EXPECT_NE(obj2.Get(), nullptr);
    EXPECT_EQ(owner.ShadowCount(), 1);
}
DEFINE_TEST_END

DEFINE_TEST_BEGIN(ShadowPtrTest, Constructor, MoveFromEmpty)
{
    ShadowPtr<TestableObject> obj1;
    EXPECT_EQ(TestableObject::Balance, 0);
    EXPECT_EQ(obj1.Get(), nullptr);

    ShadowPtr<TestableObject> obj2{ std::move(obj1) };
    EXPECT_EQ(TestableObject::Balance, 0);
    EXPECT_EQ(obj1.Get(), nullptr);
    EXPECT_EQ(obj2.Get(), nullptr);
}
DEFINE_TEST_END

DEFINE_TEST_BEGIN(ShadowPtrTest, Constructor, MoveFromDerived)
{
    class DerivedObject : public TestableObject
    {
    };

    OwnerPtr<DerivedObject> owner{ new DerivedObject };
    ShadowPtr<DerivedObject> obj1 = owner.Shadow();
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_NE(obj1.Get(), nullptr);
    EXPECT_EQ(owner.ShadowCount(), 1);

    ShadowPtr<TestableObject> obj2{ std::move(obj1) };
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_EQ(obj1.Get(), nullptr);
    EXPECT_NE(obj2.Get(), nullptr);
    EXPECT_EQ(owner.ShadowCount(), 1);
}
DEFINE_TEST_END

// ShadowPtr& operator=(const ShadowPtr<T2>& other)

DEFINE_TEST_BEGIN(ShadowPtrTest, Assignment, Copy)
{
    OwnerPtr<TestableObject> owner{ new TestableObject };
    ShadowPtr<TestableObject> obj1 = owner.Shadow();
    ShadowPtr<TestableObject> obj2;
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_EQ(owner.ShadowCount(), 1);

    obj2 = obj1;
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_EQ(obj2.Get(), obj1.Get());
    EXPECT_EQ(owner.ShadowCount(), 2);
    EXPECT_EQ(obj2.ShadowCount(), 2);
}
DEFINE_TEST_END

DEFINE_TEST_BEGIN(ShadowPtrTest, Assignment, CopyReplaceExisting)
{
    OwnerPtr<TestableObject> owner1{ new TestableObject };
    OwnerPtr<TestableObject> owner2{ new TestableObject };
    ShadowPtr<TestableObject> obj1 = owner1.Shadow();
    ShadowPtr<TestableObject> obj2 = owner2.Shadow();
    EXPECT_EQ(TestableObject::Balance, 2);
    EXPECT_EQ(owner1.ShadowCount(), 1);
    EXPECT_EQ(owner2.ShadowCount(), 1);

    obj2 = obj1;
    EXPECT_EQ(TestableObject::Balance, 2);
    EXPECT_EQ(obj2.Get(), obj1.Get());
    EXPECT_EQ(owner1.ShadowCount(), 2);
    EXPECT_EQ(owner2.ShadowCount(), 0);
    EXPECT_EQ(obj2.ShadowCount(), 2);
}
DEFINE_TEST_END

DEFINE_TEST_BEGIN(ShadowPtrTest, Assignment, CopyFromEmpty)
{
    ShadowPtr<TestableObject> obj1;
    ShadowPtr<TestableObject> obj2;
    EXPECT_EQ(TestableObject::Balance, 0);

    obj2 = obj1;
    EXPECT_EQ(TestableObject::Balance, 0);
    EXPECT_EQ(obj1.Get(), nullptr);
    EXPECT_EQ(obj2.Get(), nullptr);
}
DEFINE_TEST_END

DEFINE_TEST_BEGIN(ShadowPtrTest, Assignment, CopyFromDerived)
{
    class DerivedObject : public TestableObject
    {
    };

    OwnerPtr<DerivedObject> owner{ new DerivedObject };
    ShadowPtr<DerivedObject> obj1 = owner.Shadow();
    ShadowPtr<TestableObject> obj2;
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_EQ(owner.ShadowCount(), 1);

    obj2 = obj1;
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_EQ(obj2.Get(), obj1.Get());
    EXPECT_EQ(owner.ShadowCount(), 2);
    EXPECT_EQ(obj2.ShadowCount(), 2);
}
DEFINE_TEST_END

DEFINE_TEST_BEGIN(ShadowPtrTest, Assignment, CopySelf)
{
    OwnerPtr<TestableObject> owner{ new TestableObject };
    ShadowPtr<TestableObject> obj = owner.Shadow();
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_EQ(owner.ShadowCount(), 1);

    obj = obj;
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_FALSE(obj.IsNull());
    EXPECT_EQ(owner.ShadowCount(), 1);
    EXPECT_EQ(obj.ShadowCount(), 1);
}
DEFINE_TEST_END

// ShadowPtr& operator=(ShadowPtr<T2>&& other) noexcept

DEFINE_TEST_BEGIN(ShadowPtrTest, Assignment, Move)
{
    OwnerPtr<TestableObject> owner{ new TestableObject };
    ShadowPtr<TestableObject> obj1 = owner.Shadow();
    ShadowPtr<TestableObject> obj2;
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_EQ(owner.ShadowCount(), 1);

    obj2 = std::move(obj1);
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_EQ(obj1.Get(), nullptr);
    EXPECT_NE(obj2.Get(), nullptr);
    EXPECT_EQ(owner.ShadowCount(), 1);
}
DEFINE_TEST_END

DEFINE_TEST_BEGIN(ShadowPtrTest, Assignment, MoveReplaceExisting)
{
    OwnerPtr<TestableObject> owner1{ new TestableObject };
    OwnerPtr<TestableObject> owner2{ new TestableObject };
    ShadowPtr<TestableObject> obj1 = owner1.Shadow();
    ShadowPtr<TestableObject> obj2 = owner2.Shadow();
    EXPECT_EQ(TestableObject::Balance, 2);
    EXPECT_EQ(owner1.ShadowCount(), 1);
    EXPECT_EQ(owner2.ShadowCount(), 1);

    obj2 = std::move(obj1);
    EXPECT_EQ(TestableObject::Balance, 2);
    EXPECT_EQ(obj1.Get(), nullptr);
    EXPECT_NE(obj2.Get(), nullptr);
    EXPECT_EQ(obj2.Get(), owner1.Get());
    EXPECT_EQ(owner1.ShadowCount(), 1);
    EXPECT_EQ(owner2.ShadowCount(), 0);
    EXPECT_EQ(obj2.ShadowCount(), 1);
}
DEFINE_TEST_END

DEFINE_TEST_BEGIN(ShadowPtrTest, Assignment, MoveFromEmpty)
{
    ShadowPtr<TestableObject> obj1;
    ShadowPtr<TestableObject> obj2;
    EXPECT_EQ(TestableObject::Balance, 0);

    obj2 = std::move(obj1);
    EXPECT_EQ(TestableObject::Balance, 0);
    EXPECT_EQ(obj1.Get(), nullptr);
    EXPECT_EQ(obj2.Get(), nullptr);
}
DEFINE_TEST_END

DEFINE_TEST_BEGIN(ShadowPtrTest, Assignment, MoveFromDerived)
{
    class DerivedObject : public TestableObject
    {
    };

    OwnerPtr<DerivedObject> owner{ new DerivedObject };
    ShadowPtr<DerivedObject> obj1 = owner.Shadow();
    ShadowPtr<TestableObject> obj2;
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_EQ(owner.ShadowCount(), 1);

    obj2 = std::move(obj1);
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_EQ(obj1.Get(), nullptr);
    EXPECT_NE(obj2.Get(), nullptr);
    EXPECT_EQ(owner.ShadowCount(), 1);
}
DEFINE_TEST_END

DEFINE_TEST_BEGIN(ShadowPtrTest, Assignment, MoveSelf)
{
    OwnerPtr<TestableObject> owner{ new TestableObject };
    ShadowPtr<TestableObject> obj = owner.Shadow();
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_EQ(owner.ShadowCount(), 1);

    ShadowPtr<TestableObject>& self = obj;
    obj = std::move(self);
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_FALSE(obj.IsNull());
    EXPECT_EQ(obj.Get(), owner.Get());
    EXPECT_EQ(owner.ShadowCount(), 1);
    EXPECT_EQ(obj.ShadowCount(), 1);
}
DEFINE_TEST_END

// ConstPointer Get() const

DEFINE_TEST_BEGIN(ShadowPtrTest, ConstGet, 0)
{
    const ShadowPtr<TestableObject> obj;
    EXPECT_EQ(TestableObject::Balance, 0);
    EXPECT_EQ(obj.Get(), nullptr);
}
DEFINE_TEST_END

// Pointer Data()

DEFINE_TEST_BEGIN(ShadowPtrTest, Data, 0)
{
    class DerivedObject : public TestableObject
    {
    public:
        int Value{ 42 };
    };

    OwnerPtr<DerivedObject> owner{ new DerivedObject };
    ShadowPtr<DerivedObject> obj = owner.Shadow();
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_NE(obj.Data(), nullptr);
    EXPECT_EQ(obj->Value, 42);
}
DEFINE_TEST_END

// ConstPointer Data() const

DEFINE_TEST_BEGIN(ShadowPtrTest, ConstData, 0)
{
    class DerivedObject : public TestableObject
    {
    public:
        int Value{ 42 };
    };

    OwnerPtr<DerivedObject> owner{ new DerivedObject };
    const ShadowPtr<DerivedObject> obj = owner.Shadow();
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_NE(obj.Data(), nullptr);
    EXPECT_EQ(obj->Value, 42);
}
DEFINE_TEST_END

// Pointer operator->()

DEFINE_TEST_BEGIN(ShadowPtrTest, Arrow, 0)
{
    class DerivedObject : public TestableObject
    {
    public:
        int Value{ 42 };
    };

    OwnerPtr<DerivedObject> owner{ new DerivedObject };
    ShadowPtr<DerivedObject> obj = owner.Shadow();
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_EQ(obj->Value, 42);

    obj->Value = 100;
    EXPECT_EQ(owner->Value, 100);
}
DEFINE_TEST_END

// ConstPointer operator->() const

DEFINE_TEST_BEGIN(ShadowPtrTest, ConstArrow, 0)
{
    class DerivedObject : public TestableObject
    {
    public:
        int Value{ 42 };
    };

    OwnerPtr<DerivedObject> owner{ new DerivedObject };
    const ShadowPtr<DerivedObject> obj = owner.Shadow();
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_EQ(obj->Value, 42);
}
DEFINE_TEST_END

// Reference operator*()

DEFINE_TEST_BEGIN(ShadowPtrTest, Dereference, 0)
{
    class DerivedObject : public TestableObject
    {
    public:
        int Value{ 42 };
    };

    OwnerPtr<DerivedObject> owner{ new DerivedObject };
    ShadowPtr<DerivedObject> obj = owner.Shadow();
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_EQ((*obj).Value, 42);

    (*obj).Value = 100;
    EXPECT_EQ(owner->Value, 100);
}
DEFINE_TEST_END

// ConstReference operator*() const

DEFINE_TEST_BEGIN(ShadowPtrTest, ConstDereference, 0)
{
    class DerivedObject : public TestableObject
    {
    public:
        int Value{ 42 };
    };

    OwnerPtr<DerivedObject> owner{ new DerivedObject };
    const ShadowPtr<DerivedObject> obj = owner.Shadow();
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_EQ((*obj).Value, 42);
}
DEFINE_TEST_END

// operator bool() const

DEFINE_TEST_BEGIN(ShadowPtrTest, Bool, 0)
{
    ShadowPtr<TestableObject> emptyObj;
    EXPECT_EQ(TestableObject::Balance, 0);
    EXPECT_FALSE(emptyObj);

    OwnerPtr<TestableObject> owner{ new TestableObject };
    ShadowPtr<TestableObject> obj = owner.Shadow();
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_TRUE(obj);
}
DEFINE_TEST_END

// bool Expired() const

DEFINE_TEST_BEGIN(ShadowPtrTest, Expired, 0)
{
    ShadowPtr<TestableObject> shadow;
    EXPECT_EQ(TestableObject::Balance, 0);
    EXPECT_TRUE(shadow.Expired());

    {
        OwnerPtr<TestableObject> owner{ new TestableObject };
        shadow = owner.Shadow();
        EXPECT_EQ(TestableObject::Balance, 1);
        EXPECT_FALSE(shadow.Expired());
        EXPECT_EQ(owner.ShadowCount(), 1);
    }

    EXPECT_EQ(TestableObject::Balance, 0);
    EXPECT_TRUE(shadow.Expired());
    EXPECT_EQ(shadow.Get(), nullptr);
    EXPECT_EQ(shadow.ShadowCount(), 1);
}
DEFINE_TEST_END

// bool IsNull() const

DEFINE_TEST_BEGIN(ShadowPtrTest, IsNull, 0)
{
    ShadowPtr<TestableObject> emptyObj;
    EXPECT_EQ(TestableObject::Balance, 0);
    EXPECT_TRUE(emptyObj.IsNull());

    OwnerPtr<TestableObject> owner{ new TestableObject };
    ShadowPtr<TestableObject> obj = owner.Shadow();
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_FALSE(obj.IsNull());

    owner.Nullify();
    EXPECT_EQ(TestableObject::Balance, 0);
    EXPECT_TRUE(obj.IsNull());
}
DEFINE_TEST_END

// void Swap(ShadowPtr& other) noexcept

DEFINE_TEST_BEGIN(ShadowPtrTest, Swap, 0)
{
    OwnerPtr<TestableObject> owner1{ new TestableObject };
    OwnerPtr<TestableObject> owner2{ new TestableObject };
    EXPECT_EQ(TestableObject::Balance, 2);

    ShadowPtr<TestableObject> obj1 = owner1.Shadow();
    ShadowPtr<TestableObject> obj2 = owner2.Shadow();
    EXPECT_EQ(obj1.Get(), owner1.Get());
    EXPECT_EQ(obj2.Get(), owner2.Get());

    obj1.Swap(obj2);
    EXPECT_EQ(TestableObject::Balance, 2);
    EXPECT_EQ(obj1.Get(), owner2.Get());
    EXPECT_EQ(obj2.Get(), owner1.Get());
}
DEFINE_TEST_END

// OwnerPtr::Shadow()

DEFINE_TEST_BEGIN(ShadowPtrTest, Shadow, FromOwner)
{
    OwnerPtr<TestableObject> owner{ new TestableObject };
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_EQ(owner.ShadowCount(), 0);

    ShadowPtr<TestableObject> shadow = owner.Shadow();
    EXPECT_NE(shadow.Get(), nullptr);
    EXPECT_EQ(shadow.Get(), owner.Get());
    EXPECT_EQ(owner.ShadowCount(), 1);
    EXPECT_EQ(shadow.ShadowCount(), 1);
}
DEFINE_TEST_END

DEFINE_TEST_BEGIN(ShadowPtrTest, Shadow, FromConstOwner)
{
    const OwnerPtr<TestableObject> owner{ new TestableObject };
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_EQ(owner.ShadowCount(), 0);

    ShadowPtr<TestableObject> shadow = owner.Shadow();
    EXPECT_NE(shadow.Get(), nullptr);
    EXPECT_EQ(shadow.Get(), owner.Get());
    EXPECT_EQ(owner.ShadowCount(), 1);
    EXPECT_EQ(shadow.ShadowCount(), 1);
}
DEFINE_TEST_END

DEFINE_TEST_BEGIN(ShadowPtrTest, Shadow, FromEmptyOwner)
{
    OwnerPtr<TestableObject> owner;
    EXPECT_EQ(TestableObject::Balance, 0);
    EXPECT_EQ(owner.Get(), nullptr);

    ShadowPtr<TestableObject> shadow = owner.Shadow();
    EXPECT_EQ(shadow.Get(), nullptr);
    EXPECT_EQ(owner.ShadowCount(), 0);
    EXPECT_EQ(shadow.ShadowCount(), 0);
}
DEFINE_TEST_END

DEFINE_TEST_BEGIN(ShadowPtrTest, Shadow, FromDerivedOwner)
{
    class DerivedObject : public TestableObject
    {
    };

    OwnerPtr<DerivedObject> owner{ new DerivedObject };
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_EQ(owner.ShadowCount(), 0);

    ShadowPtr<TestableObject> shadow = owner.Shadow<TestableObject>();
    EXPECT_NE(shadow.Get(), nullptr);
    EXPECT_EQ(shadow.Get(), owner.Get());
    EXPECT_EQ(owner.ShadowCount(), 1);
    EXPECT_EQ(shadow.ShadowCount(), 1);
}
DEFINE_TEST_END

DEFINE_TEST_BEGIN(ShadowPtrTest, Shadow, Offset)
{
    class BaseObject
    {
    public:
        BaseObject(int value) : m_value(value) {}
        ~BaseObject() = default;
        int GetValue() const { return m_value; }
    private:
        int m_value{ 0 };
    };

    class DerivedObject : public BaseObject, public TestableObject
    {
    public:
        DerivedObject(int value) : BaseObject{ value } {}
    };

    OwnerPtr<DerivedObject> owner = OwnerPtr<DerivedObject>::Create(42);
    EXPECT_EQ(TestableObject::Balance, 1);

    ShadowPtr<BaseObject> shadow = owner.Shadow<BaseObject>();
    EXPECT_NE(shadow.Get(), nullptr);
    EXPECT_EQ(shadow->GetValue(), 42);
}
DEFINE_TEST_END

// ShadowPtr comparison operators

DEFINE_TEST_BEGIN(ShadowPtrTest, Comparison, ShadowPtr)
{
    OwnerPtr<TestableObject> owner1{ new TestableObject };
    OwnerPtr<TestableObject> owner2{ new TestableObject };
    ShadowPtr<TestableObject> obj1 = owner1.Shadow();
    ShadowPtr<TestableObject> obj2 = owner2.Shadow();
    EXPECT_EQ(TestableObject::Balance, 2);

    EXPECT_EQ(obj1 == obj2, obj1.Get() == obj2.Get());
    EXPECT_EQ(obj1 != obj2, obj1.Get() != obj2.Get());
    EXPECT_EQ(obj1 < obj2, obj1.Get() < obj2.Get());
    EXPECT_EQ(obj1 > obj2, obj1.Get() > obj2.Get());
    EXPECT_EQ(obj1 <= obj2, obj1.Get() <= obj2.Get());
    EXPECT_EQ(obj1 >= obj2, obj1.Get() >= obj2.Get());
}
DEFINE_TEST_END

DEFINE_TEST_BEGIN(ShadowPtrTest, Comparison, NullptrOnRight)
{
    ShadowPtr<TestableObject> emptyObj;
    EXPECT_EQ(TestableObject::Balance, 0);

    TestableObject* const pNull = nullptr;
    EXPECT_EQ(emptyObj == nullptr, emptyObj.Get() == pNull);
    EXPECT_EQ(emptyObj != nullptr, emptyObj.Get() != pNull);
    EXPECT_EQ(emptyObj < nullptr, emptyObj.Get() < pNull);
    EXPECT_EQ(emptyObj > nullptr, emptyObj.Get() > pNull);
    EXPECT_EQ(emptyObj <= nullptr, emptyObj.Get() <= pNull);
    EXPECT_EQ(emptyObj >= nullptr, emptyObj.Get() >= pNull);
}
DEFINE_TEST_END

DEFINE_TEST_BEGIN(ShadowPtrTest, Comparison, NullptrOnLeft)
{
    ShadowPtr<TestableObject> emptyObj;
    EXPECT_EQ(TestableObject::Balance, 0);

    TestableObject* const pNull = nullptr;
    EXPECT_EQ(nullptr == emptyObj, pNull == emptyObj.Get());
    EXPECT_EQ(nullptr != emptyObj, pNull != emptyObj.Get());
    EXPECT_EQ(nullptr < emptyObj, pNull < emptyObj.Get());
    EXPECT_EQ(nullptr > emptyObj, pNull > emptyObj.Get());
    EXPECT_EQ(nullptr <= emptyObj, pNull <= emptyObj.Get());
    EXPECT_EQ(nullptr >= emptyObj, pNull >= emptyObj.Get());
}
DEFINE_TEST_END

// std helpers for ShadowPtr

DEFINE_TEST_BEGIN(ShadowPtrTest, Std, SwapAndFunctionObjects)
{
    OwnerPtr<TestableObject> owner1{ new TestableObject };
    OwnerPtr<TestableObject> owner2{ new TestableObject };
    TestableObject* pRaw1 = owner1.Get();
    TestableObject* pRaw2 = owner2.Get();
    ShadowPtr<TestableObject> obj1 = owner1.Shadow();
    ShadowPtr<TestableObject> obj2 = owner2.Shadow();
    EXPECT_EQ(TestableObject::Balance, 2);

    std::swap(obj1, obj2);
    EXPECT_EQ(obj1.Get(), pRaw2);
    EXPECT_EQ(obj2.Get(), pRaw1);

    EXPECT_EQ(std::hash<ShadowPtr<TestableObject>>{}(obj1), std::hash<TestableObject*>{}(obj1.Get()));
    EXPECT_EQ(std::equal_to<ShadowPtr<TestableObject>>{}(obj1, obj2), obj1 == obj2);
    EXPECT_EQ(std::not_equal_to<ShadowPtr<TestableObject>>{}(obj1, obj2), obj1 != obj2);
    EXPECT_EQ(std::less<ShadowPtr<TestableObject>>{}(obj1, obj2), obj1 < obj2);
    EXPECT_EQ(std::less_equal<ShadowPtr<TestableObject>>{}(obj1, obj2), obj1 <= obj2);
    EXPECT_EQ(std::greater<ShadowPtr<TestableObject>>{}(obj1, obj2), obj1 > obj2);
    EXPECT_EQ(std::greater_equal<ShadowPtr<TestableObject>>{}(obj1, obj2), obj1 >= obj2);
}
DEFINE_TEST_END
