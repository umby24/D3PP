#include <string>
#include <gtest/gtest.h>
#include "files/D3Map.h"
using namespace D3PP::files;

TEST(D3MapTest, LoadExistingMap) {
	D3Map mapTest("TestFolder/goodMap");
	bool result = mapTest.Load();
	ASSERT_TRUE(result);
}

TEST(D3MapTest, LoadNonExistingMap) {
	D3Map mapTest("TestFolder/TestFolder");
	bool result = mapTest.Load();
	ASSERT_FALSE(result);
}

TEST(D3MapTest, LoadMapBadConfig) {
	D3Map mapTest("TestFolder/badConfig");
	bool result = mapTest.Load();
	ASSERT_FALSE(result);
}

TEST(D3MapTest, LoadMapBadBlockData) {
	D3Map mapTest("TestFolder/badBlocks");
	bool result = mapTest.Load();
	ASSERT_FALSE(result);
}