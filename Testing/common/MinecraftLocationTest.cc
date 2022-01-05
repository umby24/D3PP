#include <string>
#include <gtest/gtest.h>
#include "common/MinecraftLocation.h"

TEST(MinecraftLocationTest, EqualityTest) {
    MinecraftLocation givenFirst {0, 0, { 0, 0, 0} };
    MinecraftLocation givenSecond {0, 0, { 0, 0, 0} };
    bool actualResult = givenFirst == givenSecond;
    assert(actualResult);
}

TEST(MinecraftLocationTest, EqualityNegativeTest) {
    MinecraftLocation givenFirst{0, 0, { 0, 0, 0} };
    MinecraftLocation givenSecond{0, 0, { 3, 0, 0} };
    bool actualResult = givenFirst == givenSecond;
    assert(!actualResult);
}

TEST(MinecraftLocationTest, SetAsBlockCoordsTest) {
    Vector3S givenBlockCoords {20, 22, 24};
    Vector3S expectedPlayerCoords {640, 704, 819};
    MinecraftLocation givenLocation;

    givenLocation.SetAsBlockCoords(givenBlockCoords);
    assert(givenLocation.Location.isEqual(expectedPlayerCoords));
}

TEST(MinecraftLocationTest, SetAsPlayerCoordsTest) {
    Vector3S givenPlayerCoords {640, 704, 819};
    MinecraftLocation givenLocation;

    givenLocation.SetAsPlayerCoords(givenPlayerCoords);
    assert(givenLocation.Location.isEqual(givenPlayerCoords));
}

TEST(MinecraftLocationTest, SetAsPlayerCoordsFloatTest) {
    Vector3F givenPlayerCoords {20.25, 22.25, 24.25};
    Vector3S expectedPlayerCoords {648, 712, 827};
    MinecraftLocation givenLocation;

    givenLocation.SetAsPlayerCoords(givenPlayerCoords);
    assert(givenLocation.Location.isEqual(expectedPlayerCoords));
}