#include <string>
#include <gtest/gtest.h>
#include "common/ByteBuffer.h"

TEST(ByteBuffer, SmokeTest) {
{
ByteBuffer testBuffer(nullptr);
}


}

TEST(ByteBuffer, WriteByte) {
unsigned char givenValue = 2;

ByteBuffer testBuffer(nullptr);
testBuffer.Write(givenValue);

ASSERT_EQ(1, testBuffer.Size());
ASSERT_EQ(givenValue, testBuffer.PeekByte());
}

TEST(ByteBuffer, WriteShort) {
short givenValue = 2;

ByteBuffer testBuffer(nullptr);
testBuffer.Write(givenValue);

ASSERT_EQ(2, testBuffer.Size());
ASSERT_EQ(0, testBuffer.PeekByte());
}

TEST(ByteBuffer, WriteInt) {
    int givenValue = 2142;

    ByteBuffer testBuffer(nullptr);
    testBuffer.Write(givenValue);

    ASSERT_EQ(4, testBuffer.Size());
    ASSERT_EQ(0, testBuffer.PeekByte());
}

TEST(ByteBuffer, WriteString) {
    std::string givenString = "testString";

    ByteBuffer testBuffer(nullptr);
    testBuffer.Write(givenString);

    ASSERT_EQ(64, testBuffer.Size());
}

TEST(ByteBuffer, WriteByteArray) {
    std::vector<unsigned char> someArray;
    someArray.push_back(1);
    someArray.push_back(3);
    someArray.push_back(4);

    ByteBuffer testBuffer(nullptr);
    testBuffer.Write(someArray, 3);

    ASSERT_EQ(3, testBuffer.Size());
    ASSERT_EQ(1, testBuffer.PeekByte());
}

TEST(ByteBuffer, WriteIncorrectArray) {
    std::vector<unsigned char> someArray;
    someArray.push_back(1);
    someArray.push_back(3);
    someArray.push_back(4);

    ByteBuffer testBuffer(nullptr);
    testBuffer.Write(someArray, 7);

    ASSERT_EQ(3, testBuffer.Size());
    ASSERT_EQ(1, testBuffer.PeekByte());
}

TEST(ByteBuffer, Shift) {
    std::vector<unsigned char> someArray;
    someArray.push_back(1);
    someArray.push_back(3);
    someArray.push_back(4);

    ByteBuffer testBuffer(nullptr);
    testBuffer.Write(someArray, 7);
    testBuffer.Shift(1);

    ASSERT_EQ(2, testBuffer.Size());
    ASSERT_EQ(3, testBuffer.PeekByte());
}

TEST(ByteBuffer, ShiftZero) {
    std::vector<unsigned char> someArray;
    someArray.push_back(1);
    someArray.push_back(3);
    someArray.push_back(4);

    ByteBuffer testBuffer(nullptr);
    testBuffer.Write(someArray, 7);
    testBuffer.Shift(0);

    ASSERT_EQ(3, testBuffer.Size());
    ASSERT_EQ(1, testBuffer.PeekByte());
}

TEST(ByteBuffer, ShiftNegative) {
    std::vector<unsigned char> someArray;
    someArray.push_back(1);
    someArray.push_back(3);
    someArray.push_back(4);

    ByteBuffer testBuffer(nullptr);
    testBuffer.Write(someArray, 7);
    testBuffer.Shift(-3);

    ASSERT_EQ(3, testBuffer.Size());
    ASSERT_EQ(1, testBuffer.PeekByte());
}

TEST(ByteBuffer, Purge) {

    bool wasCalled = false;
    std::vector<unsigned char> someArray;
    someArray.push_back(1);
    someArray.push_back(3);
    someArray.push_back(4);

    ByteBuffer testBuffer([this, &wasCalled]{ wasCalled = true; });
    testBuffer.Write(someArray, 7);
    testBuffer.Purge();

    ASSERT_EQ(3, testBuffer.Size());
    ASSERT_EQ(1, testBuffer.PeekByte());
    ASSERT_TRUE(wasCalled);
}