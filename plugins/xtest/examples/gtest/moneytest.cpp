/*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
* 02110-1301, USA.
*/

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
