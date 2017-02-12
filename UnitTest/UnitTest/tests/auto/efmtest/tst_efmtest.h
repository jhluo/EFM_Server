#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

using namespace testing;

TEST(EFMTest, EFMTest)
{
    EXPECT_EQ(1, 1);
    ASSERT_THAT(0, Eq(0));
}
