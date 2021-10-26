//
// Created by unknown on 10/23/21.
//

#include <string>
#include <gtest/gtest.h>
#include "Utils.h"
#include "EventSystem.h"
#include "events/EntityEventArgs.h"

TEST(UtilsTest, IsNumericBasic) {
    bool actualResult1 = Utils::IsNumeric("1234");
    bool actualResult2 = Utils::IsNumeric("abcd");
    bool actualResult3 = Utils::IsNumeric("123b");

    EXPECT_EQ(true, actualResult1);
    EXPECT_EQ(false, actualResult2);
    EXPECT_EQ(false, actualResult3);
}

TEST(UtilsTest, RgbCodeTest) {
    int actual = Utils::Rgb(255, 255, 255);
    EXPECT_EQ(16777215, actual);
}

TEST(UtilsTest, RTest) {
    int actual = Utils::RedVal(16777215);
    EXPECT_EQ(255, actual);
}
TEST(UtilsTest, GTest) {
    int actual = Utils::GreenVal(16777215);
    EXPECT_EQ(255, actual);
}
TEST(UtilsTest, BTest) {
    int actual = Utils::BlueVal(16777215);
    EXPECT_EQ(255, actual);
}

TEST(UtilsTest, TrimTest) {
    std::string givenString = "I am a test string                                 \r\n";
    std::string expectedString = "I am a test string";
    Utils::TrimString(givenString);
    EXPECT_EQ(expectedString, givenString);
}

TEST(UtilsTest, InsensitivityTest) {
    std::string givenString = "I am a test STRING";
    std::string expectedString = "I aM a TeST StriNG";
    bool actualResult = Utils::InsensitiveCompare(givenString, expectedString);
    EXPECT_EQ(true, actualResult);
}