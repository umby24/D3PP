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

}

TEST(ByteBuffer, WriteString) {

}

TEST(ByteBuffer, WriteByteArray) {

}

TEST(ByteBuffer, WriteIncorrectArray) {

}

TEST(ByteBuffer, Shift) {

}

TEST(ByteBuffer, ShiftZero) {

}

TEST(ByteBuffer, ShiftNegative) {

}

TEST(ByteBuffer, Purge) {

}