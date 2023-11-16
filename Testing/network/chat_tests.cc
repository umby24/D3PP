//
// Created by Wande on 11/15/2023.
//

#include <gtest/gtest.h>
#include <string>
#include "network/Chat.h"

TEST(ChatTests, HandleChatEscapes_NothingToEscape) {
    std::string givenInput = "Just a test string, nothing to see.!";
    std::string expectedOutput = "Just a test string, nothing to see.!";

    Chat::HandleChatEscapes(givenInput);
    EXPECT_EQ(expectedOutput, givenInput);
}

TEST(ChatTests, HandleChatEscapes_EscapePercent) {
    std::string givenInput = "Just a %5test string, %cnothing to see.!";
    std::string expectedOutput = "Just a &5test string, &cnothing to see.!";

    Chat::HandleChatEscapes(givenInput);
    EXPECT_EQ(expectedOutput, givenInput);
}

TEST(ChatTests, HandleChatEscapes_EscapeDoublePercent) {
    std::string givenInput = "Just a %% test string, %% nothing to see.!";
    std::string expectedOutput = "Just a % test string, % nothing to see.!";

    Chat::HandleChatEscapes(givenInput);
    EXPECT_EQ(expectedOutput, givenInput);
}

TEST(ChatTests, HandleChatEscapes_EscapeBR) {
    std::string givenInput = "Just a <br> test string, %% nothing to see.!";
    std::string expectedOutput = "Just a \n test string, % nothing to see.!";

    Chat::HandleChatEscapes(givenInput);
    EXPECT_EQ(expectedOutput, givenInput);
}

//TEST(ChatTests, MultiLine_Simple) {
//    std::string givenInput = "This is a string that is longer than the minecraft classic maximum";
//    std::string expectedOutput = "This is a string that is longer than the minecraft classic\n maximum";
//
//    Chat::StringMultiline(givenInput);
//    EXPECT_EQ(expectedOutput, givenInput);
//}
//
//TEST(ChatTests, MultiLine_Complex) {
//    std::string givenInput = "&cThis is a string that is longer than the minecraft classic maximum and &3seems to just go on forever and &6ever and ever and ever yeah man";
//    std::string expectedOutput = "&cThis is a string that is longer than the minecraft classic maximum and &3seems to just go on forever and &6ever and ever and ever yeah man";
//
//    Chat::StringMultiline(givenInput);
//    EXPECT_EQ(expectedOutput, givenInput);
//}