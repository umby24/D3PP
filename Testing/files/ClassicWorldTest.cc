#include <string>
#include <gtest/gtest.h>

#include "files/ClassicWorld.h"
using namespace D3PP::files;

TEST(ClassicWorldTest, CreateNewTest) {
    ClassicWorld myNew(D3PP::Common::Vector3S((short)64, 64, 64));
    myNew.MapName = "UnitTest";

    myNew.Save("somenewfile.cw");
}

TEST(ClassicWorldTest, LoadTest) {
    ClassicWorld myExisting("TestFolder/default.cw");
    myExisting.Load();
    myExisting.Save("TestFolder/resave.cw");
    ASSERT_EQ(myExisting.MapName, "default");
}