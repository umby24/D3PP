#include <fstream>
#include <gtest/gtest.h>
#include "Nbt/cppNbt.h"
#include <variant>

using namespace nbt;

TEST(NBTTest, LoadFile) {
    Nbt::Tag result = Nbt::NbtFile::Load("TestFolder/default.cw");
    Nbt::TagCompound actual = std::get<Nbt::TagCompound>(result);
    ASSERT_EQ(actual.name, "ClassicWorld");
    Nbt::Tag name = actual["Name"];
    Nbt::TagString rn = std::get<Nbt::TagString>(name);
    ASSERT_EQ(rn,"default");
}
