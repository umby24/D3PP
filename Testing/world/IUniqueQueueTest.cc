//
// Created by Wande on 1/25/2022.
//

#include <gtest/gtest.h>
#include "common/Vectors.h"
#include "world/IUniqueQueue.h"
#include <cmath>

TEST(IUniqueQueue, OffsetIsCorrect) {
    D3PP::Common::Vector3S givenSize {10, 10, (short)10};
    D3PP::world::IUniqueQueue underTest(givenSize);
    // --
    D3PP::Common::Vector3S loc1 {0, 0, (short)0};
    D3PP::Common::Vector3S loc2 {1, 5, (short)0};
    D3PP::Common::Vector3S loc3 {2, 5, (short)0};
    D3PP::Common::Vector3S loc4 {3, 5, (short)0};
    D3PP::Common::Vector3S loc5 {4, 5, (short)0};
    D3PP::Common::Vector3S loc6 {5, 5, (short)0};
    D3PP::Common::Vector3S loc7 {6, 5, (short)0};
    D3PP::Common::Vector3S loc8 {7, 5, (short)0};

    int actualOffset = std::ceil(underTest.GetOffset(loc1) / 8);
    int actualOffset2 = std::ceil(underTest.GetOffset(loc2) / 8);
    int actualOffset3 = std::ceil(underTest.GetOffset(loc3) / 8);
    int actualOffset4 = std::ceil(underTest.GetOffset(loc4) / 8);
    int actualOffset5 = std::ceil(underTest.GetOffset(loc5) / 8);
    int actualOffset6 = std::ceil(underTest.GetOffset(loc6) / 8);
    int actualOffset7 = std::ceil(underTest.GetOffset(loc7) / 8);
    int actualOffset8 = std::ceil(underTest.GetOffset(loc8) / 8);

    ASSERT_EQ(actualOffset, 0);
    ASSERT_EQ(actualOffset2, 6);
    ASSERT_EQ(actualOffset3, 6);
    ASSERT_EQ(actualOffset4, 6);
    ASSERT_EQ(actualOffset5, 6);
    ASSERT_EQ(actualOffset6, 6);
    ASSERT_EQ(actualOffset7, 7);
    ASSERT_EQ(actualOffset8, 7);
}

TEST(IUniqueQueue, Queue) {
    D3PP::Common::Vector3S givenSize {64, 64, (short)64};
    D3PP::world::IUniqueQueue underTest(givenSize);

    for(short x = 0; x < 64; x++) {
        for (short y = 0; y < 64; y++) {
            D3PP::Common::Vector3S givenLoc {x, y, (short)1};
            underTest.Queue(givenLoc);
            ASSERT_TRUE(underTest.IsQueued(givenLoc));
            for(short z = 2; z < 64; z++) {
                D3PP::Common::Vector3S givenLoc2 {x, y, z};
                ASSERT_FALSE(underTest.IsQueued(givenLoc2));
            }
        }
    }
}

TEST(IUniqueQueue, Dequeue) {
    D3PP::Common::Vector3S givenSize {64, 64, (short)64};
    D3PP::world::IUniqueQueue underTest(givenSize);

    for(short x = 0; x < 64; x++) {
        for (short y = 0; y < 64; y++) {
            D3PP::Common::Vector3S givenLoc {x, y, (short)1};
            underTest.Queue(givenLoc);
            ASSERT_TRUE(underTest.IsQueued(givenLoc));
            for(short z = 2; z < 64; z++) {
                D3PP::Common::Vector3S givenLoc2 {x, y, z};
                ASSERT_FALSE(underTest.IsQueued(givenLoc2));
            }
            underTest.Dequeue(givenLoc);
            ASSERT_FALSE(underTest.IsQueued(givenLoc));
        }
    }
}