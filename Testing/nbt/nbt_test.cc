#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <gtest/gtest.h>
#include "Nbt/cppNbt.h"
#include <variant>

TEST(NBTTest, LoadFile) {
    Nbt::Tag result = Nbt::NbtFile::Load("TestFolder/default.cw");
    Nbt::TagCompound actual = std::get<Nbt::TagCompound>(result);
    ASSERT_EQ(actual.name, "ClassicWorld");
    Nbt::Tag name = actual["Name"];
    Nbt::TagString rn = std::get<Nbt::TagString>(name);
    ASSERT_EQ(rn,"default");
}

TEST(NBTTest, WriteFile) {
    Nbt::TagCompound myNewTag;
    myNewTag.name = "UnitTest";
    myNewTag.data.insert(std::make_pair<std::string, Nbt::TagString>("UnitTestStr", "My test string"));
    myNewTag.data.insert(std::make_pair<std::string, Nbt::TagByte>("UnitTestByte", 0x01));
    myNewTag.data.insert(std::make_pair<std::string, Nbt::TagByte>("", 0x05));

    Nbt::NbtFile::Save(myNewTag, "TestFolder/uncompressed.nbt", Nbt::CompressionMode::NONE);
}

TEST(NBTTest, WriteFileCompressed) {
    Nbt::TagCompound myNewTag;
    Nbt::TagByteArray myByteArray;
    myByteArray.push_back(0);
    myByteArray.push_back(1);
    myByteArray.push_back(2);
    myByteArray.push_back(3);

    myNewTag.name = "UnitTestCompressed";
    myNewTag.data.insert(std::make_pair<std::string, Nbt::TagString>("UnitTestStr", "My test string"));
    myNewTag.data.insert(std::make_pair<std::string, Nbt::TagByte>("UnitTestByte", 0x01));
    myNewTag.data.insert(std::make_pair<std::string, Nbt::TagByte>("", 0x05));
    myNewTag.data.insert(std::make_pair<std::string, Nbt::TagShort>("ShortTest", 200));
    myNewTag.data.insert({"ByteArrayTest", {myByteArray}});

    Nbt::TagCompound secondCompound;
    secondCompound.name = "UnitTest";
    secondCompound.data.insert(std::make_pair<std::string, Nbt::TagString>("UnitTestStr", "My test string"));
    secondCompound.data.insert(std::make_pair<std::string, Nbt::TagByte>("UnitTestByte", 0x01));
    secondCompound.data.insert(std::make_pair<std::string, Nbt::TagByte>("", 0x05));
    myNewTag.data.insert({"compoundTest", {secondCompound}});

    Nbt::NbtFile::Save(myNewTag, "TestFolder/compressed.nbt", Nbt::CompressionMode::NONE);
}

TEST(NBTTest, RewriteCw) {
    Nbt::Tag result = Nbt::NbtFile::Load("TestFolder/default.cw");
    Nbt::TagCompound actual = std::get<Nbt::TagCompound>(result);

    Nbt::NbtFile::Save(actual, "TestFolder/rewrite.cw");
}