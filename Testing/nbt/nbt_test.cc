#include <string>
#include <iostream>
#include <fstream>
#include <gtest/gtest.h>
#include "nbt-cpp/include/nbt.hpp"

using namespace nbt;

TEST(NBTTest, LoadFile) {
    std::ifstream is("TestFolder/default.cwd");
    tags::compound_tag root;
    is >> contexts::java >> root;

    std::ofstream output("mojangson.txt");
    output << contexts::mojangson << root;
    is.close();
    output.close();
// Optionally, use the constructor
// nbt::NBT root {ifs};

//    std::stringstream ss;
//    ss << root;
//    printf("%s", ss.str().c_str());
}
