#include "money.h"
#include <gtest/gtest.h>

TEST(MoneyTest, create) {
    Money m(5, "USD");
    EXPECT_TRUE(m.isValid());
    EXPECT_EQ(5, m.amount());
    EXPECT_EQ(std::string("USD"), m.currency());
}

TEST(MoneyTest, negative) {
    Money m(-5, "USD");
    EXPECT_FALSE(m.isValid());
}

TEST(MoneyTest, zero) {
    Money m(0, "USD");
    EXPECT_TRUE(m.isValid());
    EXPECT_EQ(0, m.amount());
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
