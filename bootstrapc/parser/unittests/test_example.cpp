#include <gtest/gtest.h>

#include <string>

TEST(SampleTest, AssertionTrue) {
  ASSERT_TRUE(true);
}

TEST(SampleTest, AssertionEqual) {
  ASSERT_EQ(1, 1);
}

TEST(SampleTest, AssertionString) {
    std::string a = "abc";
    std::string b = "abd";

    ASSERT_EQ(a, b);
}

