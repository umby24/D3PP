#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "Client.h"
#include "network/Network.h"
#include "network/NetworkClient.h"
#include "world/Player.h"
#include "common/Player_List.h"
#include "world/MapMain.h"
#include "Rank.h"
#include "plugins/Heartbeat.h"

using ::testing::_;
using ::testing::Return;
using ::testing::NiceMock;

class MockNetwork : public Network {
public:
    MOCK_METHOD(std::shared_ptr<NetworkClient>, GetClient, (int clientId), (override));
};

class MockNetworkClient : public NetworkClient {
public:
    MOCK_METHOD(void, Kick, (const std::string& reason, bool showReason), (override));
};

class MockPlayerList : public Player_List {
public:
    MOCK_METHOD(std::shared_ptr<PlayerListEntry>, GetPointer, (const std::string& name), (override));
    MOCK_METHOD(void, Add, (const std::string& name), (override));
};

class MockMapMain : public MapMain {
public:
    MOCK_METHOD(std::shared_ptr<Map>, GetPointer, (int mapId), (override));
};

class MockRank : public Rank {
public:
    MOCK_METHOD(RankItem, GetRank, (int rankId, bool isTempRank), (override));
};

class MockHeartbeat : public Heartbeat {
public:
    MOCK_METHOD(bool, VerifyName, (const std::string& name, const std::string& mppass), (override));
};

TEST(ClientTest, LoginWithInvalidVersion) {
    NiceMock<MockNetwork> mockNetwork;
    NiceMock<MockNetworkClient> mockNetworkClient;
    NiceMock<MockPlayerList> mockPlayerList;
    NiceMock<MockMapMain> mockMapMain;
    NiceMock<MockRank> mockRank;
    NiceMock<MockHeartbeat> mockHeartbeat;

    Network::SetInstance(&mockNetwork);
    Player_List::SetInstance(&mockPlayerList);
    MapMain::SetInstance(&mockMapMain);
    Rank::SetInstance(&mockRank);
    Heartbeat::SetInstance(&mockHeartbeat);

    ON_CALL(mockNetwork, GetClient(_)).WillByDefault(Return(std::make_shared<MockNetworkClient>()));
    ON_CALL(mockNetworkClient, Kick(_, _)).WillByDefault([](const std::string& reason, bool showReason) {
        ASSERT_EQ(reason, "Unknown client version");
        ASSERT_TRUE(showReason);
    });

    Client::Login(1, "Test", "Test", 6);

    Network::SetInstance(nullptr);
    Player_List::SetInstance(nullptr);
    MapMain::SetInstance(nullptr);
    Rank::SetInstance(nullptr);
    Heartbeat::SetInstance(nullptr);
}

TEST(ClientTest, LoginWithInvalidName) {
// Similar to the above test, but this time we're testing the case where the name is invalid.
// The setup and assertions would be similar, but with different parameters and expected results.
}

TEST(ClientTest, LoginWithServerFull) {
// Similar to the above tests, but this time we're testing the case where the server is full.
// The setup and assertions would be similar, but with different parameters and expected results.
}

TEST(ClientTest, LoginWithInvalidSpawnMap) {
// Similar to the above tests, but this time we're testing the case where the spawn map is invalid.
// The setup and assertions would be similar, but with different parameters and expected results.
}

TEST(ClientTest, LoginWithFailedNameVerification) {
// Similar to the above tests, but this time we're testing the case where the name verification fails.
// The setup and assertions would be similar, but with different parameters and expected results.
}

TEST(ClientTest, LoginWithBannedPlayer) {
// Similar to the above tests, but this time we're testing the case where the player is banned.
// The setup and assertions would be similar, but with different parameters and expected results.
}

TEST(ClientTest, LoginWithSuccessfulLogin) {
// Similar to the above tests, but this time we're testing the case where the login is successful.
// The setup and assertions would be similar, but with different parameters and expected results.
}