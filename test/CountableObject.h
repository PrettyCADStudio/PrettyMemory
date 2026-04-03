#pragma once

template<typename TC>
class CountableObject
{
public:

    static inline int Balance{ 0 };

    CountableObject()
    {
        ++Balance;
    }

    CountableObject(const CountableObject&)
    {
        ++Balance;
    }

    CountableObject(CountableObject&&) noexcept
    {
        ++Balance;
    }

    ~CountableObject()
    {
        --Balance;
    }
};

#ifdef GTEST_TEST_
#undef GTEST_TEST_
#endif

#define GTEST_TEST_(test_suite_name, test_name, parent_class, parent_id)                            \
  static_assert(sizeof(GTEST_STRINGIFY_(test_suite_name)) > 1,                                      \
                "test_suite_name must not be empty");                                               \
  static_assert(sizeof(GTEST_STRINGIFY_(test_name)) > 1,                                            \
                "test_name must not be empty");                                                     \
  class GTEST_TEST_CLASS_NAME_(test_suite_name, test_name)                                          \
      : public parent_class {                                                                       \
   public:                                                                                          \
    GTEST_TEST_CLASS_NAME_(test_suite_name, test_name)() = default;                                 \
    ~GTEST_TEST_CLASS_NAME_(test_suite_name, test_name)() override = default;                       \
    GTEST_TEST_CLASS_NAME_(test_suite_name, test_name)                                              \
    (const GTEST_TEST_CLASS_NAME_(test_suite_name, test_name) &) = delete;                          \
    GTEST_TEST_CLASS_NAME_(test_suite_name, test_name) & operator=(                                 \
        const GTEST_TEST_CLASS_NAME_(test_suite_name,                                               \
                                     test_name) &) = delete; /* NOLINT */                           \
    GTEST_TEST_CLASS_NAME_(test_suite_name, test_name)                                              \
    (GTEST_TEST_CLASS_NAME_(test_suite_name, test_name) &&) noexcept = delete;                      \
    GTEST_TEST_CLASS_NAME_(test_suite_name, test_name) & operator=(                                 \
        GTEST_TEST_CLASS_NAME_(test_suite_name,                                                     \
                               test_name) &&) noexcept = delete; /* NOLINT */                       \
                                                                                                    \
   using TestableObject = CountableObject<GTEST_TEST_CLASS_NAME_(test_suite_name, test_name)>;      \
                                                                                                    \
   private:                                                                                         \
    void TestBody() override;                                                                       \
    static ::testing::TestInfo* const test_info_ GTEST_ATTRIBUTE_UNUSED_;                           \
  };                                                                                                \
                                                                                                    \
  ::testing::TestInfo* const GTEST_TEST_CLASS_NAME_(test_suite_name,                                \
                                                    test_name)::test_info_ =                        \
      ::testing::internal::MakeAndRegisterTestInfo(                                                 \
          #test_suite_name, #test_name, nullptr, nullptr,                                           \
          ::testing::internal::CodeLocation(__FILE__, __LINE__), (parent_id),                       \
          ::testing::internal::SuiteApiResolver<                                                    \
              parent_class>::GetSetUpCaseOrSuite(__FILE__, __LINE__),                               \
          ::testing::internal::SuiteApiResolver<                                                    \
              parent_class>::GetTearDownCaseOrSuite(__FILE__, __LINE__),                            \
          new ::testing::internal::TestFactoryImpl<GTEST_TEST_CLASS_NAME_(                          \
              test_suite_name, test_name)>);                                                        \
  void GTEST_TEST_CLASS_NAME_(test_suite_name, test_name)::TestBody()
