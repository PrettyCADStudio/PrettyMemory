#include "PrettyMemoryTest.h"

using namespace prtm;

// EnableShadowFromThis::ShadowFromThis()

DEFINE_TEST_BEGIN(EnableShadowFromThisTest, ShadowFromThis, WhileAlive)
{
    class ShadowableObject : public TestableObject, public EnableShadowFromThis<ShadowableObject>
    {
    public:
        int Value{ 42 };

        ShadowPtr<ShadowableObject> MakeShadow()
        {
            return ShadowFromThis();
        }
    };

    ShadowableObject obj;
    EXPECT_EQ(TestableObject::Balance, 1);

    ShadowPtr<ShadowableObject> shadow = obj.MakeShadow();
    EXPECT_NE(shadow.Get(), nullptr);
    EXPECT_EQ(shadow.Get(), &obj);
    EXPECT_FALSE(shadow.Expired());
    EXPECT_FALSE(shadow.IsNull());
    EXPECT_EQ(shadow->Value, 42);
    EXPECT_EQ(shadow.ShadowCount(), 1);
}
DEFINE_TEST_END

DEFINE_TEST_BEGIN(EnableShadowFromThisTest, ShadowFromThis, ExpiresAfterStackDestruction)
{
    class ShadowableObject : public TestableObject, public EnableShadowFromThis<ShadowableObject>
    {
    public:
        ShadowPtr<ShadowableObject> MakeShadow()
        {
            return ShadowFromThis();
        }
    };

    ShadowPtr<ShadowableObject> shadow;

    {
        ShadowableObject obj;
        EXPECT_EQ(TestableObject::Balance, 1);

        shadow = obj.MakeShadow();
        EXPECT_NE(shadow.Get(), nullptr);
        EXPECT_EQ(shadow.Get(), &obj);
        EXPECT_FALSE(shadow.Expired());
    }

    EXPECT_EQ(TestableObject::Balance, 0);
    EXPECT_EQ(shadow.Get(), nullptr);
    EXPECT_TRUE(shadow.Expired());
    EXPECT_TRUE(shadow.IsNull());
    EXPECT_EQ(shadow.ShadowCount(), 1);
}
DEFINE_TEST_END

DEFINE_TEST_BEGIN(EnableShadowFromThisTest, ShadowFromThis, ExpiresAfterOwnerReset)
{
    class ShadowableObject : public TestableObject, public EnableShadowFromThis<ShadowableObject>
    {
    public:
        ShadowPtr<ShadowableObject> MakeShadow()
        {
            return ShadowFromThis();
        }
    };

    OwnerPtr<ShadowableObject> owner = OwnerPtr<ShadowableObject>::Create();
    EXPECT_EQ(TestableObject::Balance, 1);
    EXPECT_NE(owner.Get(), nullptr);

    ShadowPtr<ShadowableObject> shadow = owner->MakeShadow();
    EXPECT_NE(shadow.Get(), nullptr);
    EXPECT_EQ(shadow.Get(), owner.Get());
    EXPECT_FALSE(shadow.Expired());

    owner.Reset();
    EXPECT_EQ(TestableObject::Balance, 0);
    EXPECT_EQ(shadow.Get(), nullptr);
    EXPECT_TRUE(shadow.Expired());
    EXPECT_TRUE(shadow.IsNull());
}
DEFINE_TEST_END

DEFINE_TEST_BEGIN(EnableShadowFromThisTest, ShadowFromThis, ConstObject)
{
    class ShadowableObject : public TestableObject, public EnableShadowFromThis<ShadowableObject>
    {
    public:
        int Value{ 42 };

        ShadowPtr<const ShadowableObject> MakeShadow() const
        {
            return ShadowFromThis();
        }
    };

    const ShadowableObject obj;
    EXPECT_EQ(TestableObject::Balance, 1);

    ShadowPtr<const ShadowableObject> shadow = obj.MakeShadow();
    EXPECT_NE(shadow.Get(), nullptr);
    EXPECT_EQ(shadow.Get(), &obj);
    EXPECT_FALSE(shadow.Expired());
    EXPECT_FALSE(shadow.IsNull());
    EXPECT_EQ(shadow->Value, 42);
    EXPECT_EQ(shadow.ShadowCount(), 1);
}
DEFINE_TEST_END

DEFINE_TEST_BEGIN(EnableShadowFromThisTest, ShadowFromThis, MultipleShadowsShareState)
{
    class ShadowableObject : public TestableObject, public EnableShadowFromThis<ShadowableObject>
    {
    public:
        ShadowPtr<ShadowableObject> MakeShadow()
        {
            return ShadowFromThis();
        }
    };

    ShadowableObject obj;
    EXPECT_EQ(TestableObject::Balance, 1);

    ShadowPtr<ShadowableObject> shadow1 = obj.MakeShadow();
    ShadowPtr<ShadowableObject> shadow2 = obj.MakeShadow();
    EXPECT_EQ(shadow1.Get(), &obj);
    EXPECT_EQ(shadow2.Get(), &obj);
    EXPECT_EQ(shadow1.ShadowCount(), 2);
    EXPECT_EQ(shadow2.ShadowCount(), 2);
}
DEFINE_TEST_END
