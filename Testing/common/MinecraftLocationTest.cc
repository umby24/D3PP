#include <string>
#include <gtest/gtest.h>
#include "common/MinecraftLocation.h"

TEST(MinecraftLocationTest, EqualityTest) {
    D3PP::Common::Vector3S blankLoc((short)0, (short)0, (short)0);
    MinecraftLocation givenFirst {0, 0, blankLoc };
    MinecraftLocation givenSecond {0, 0, blankLoc };
    bool actualResult = givenFirst == givenSecond;
    assert(actualResult);
}

TEST(MinecraftLocationTest, EqualityNegativeTest) {
    D3PP::Common::Vector3S blankLoc((short)0, (short)0, (short)0);
    D3PP::Common::Vector3S otherLoc((short)3, (short)0, (short)0);
    MinecraftLocation givenFirst{0, 0, blankLoc };
    MinecraftLocation givenSecond{0, 0, otherLoc };
    bool actualResult = givenFirst == givenSecond;
    assert(!actualResult);
}

TEST(MinecraftLocationTest, SetAsBlockCoordsTest) {
    D3PP::Common::Vector3S givenBlockCoords {(short)20, (short)22, (short)24};
    D3PP::Common::Vector3S expectedPlayerCoords {(short)640, 704, 819};
    MinecraftLocation givenLocation{};

    givenLocation.SetAsBlockCoords(givenBlockCoords);
    assert(givenLocation.Location.isEqual(expectedPlayerCoords));
}

TEST(MinecraftLocationTest, SetAsPlayerCoordsTest) {
    D3PP::Common::Vector3S givenPlayerCoords {(short)640, 704, 819};
    MinecraftLocation givenLocation{};

    givenLocation.SetAsPlayerCoords(givenPlayerCoords);
    assert(givenLocation.Location.isEqual(givenPlayerCoords));
}

TEST(MinecraftLocationTest, SetAsPlayerCoordsFloatTest) {
    D3PP::Common::Vector3F givenPlayerCoords {20.25, 22.25, 24.25};
    D3PP::Common::Vector3S expectedPlayerCoords {(short)648, 712, 827};
    MinecraftLocation givenLocation{};

    givenLocation.SetAsPlayerCoords(givenPlayerCoords);
    assert(givenLocation.Location.isEqual(expectedPlayerCoords));
}