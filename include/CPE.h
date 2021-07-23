#ifndef D3PP_CPE_H
#define D3PP_CPE_H

#define CLICK_DISTANCE_EXT_NAME "ClickDistance"
#define CUSTOM_BLOCKS_EXT_NAME "CustomBlocks"
#define ENV_COLORS_EXT_NAME "EnvColors"
#define ENV_APPEARANCE_EXT_NAME "EnvMapAppearance"
#define HACKCONTROL_EXT_NAME "HackControl"
#define EXT_PLAYER_LIST_EXT_NAME "ExtPlayerList"
#define EXT_WEATHER_CONTROL_EXT_NAME "EnvWeatherType"
#define BLOCK_PERMISSIONS_EXT_NAME "BlockPermissions"
#define HOTKEY_EXT_NAME "TextHotKey"
#define HELDBLOCK_EXT_NAME "HeldBlock"
#define CHANGE_MODEL_EXT_NAME "ChangeModel"
#define MESSAGE_TYPES_EXT_NAME "MessageTypes"
#define SELECTION_CUBOID_EXT_NAME "SelectionCuboid"
#define LONG_MESSAGES_EXT_NAME "LongerMessages"

#include <memory>

class NetworkClient;
class Entity;

class CPE {
    public:
    static int GetClientExtVersion(std::shared_ptr<NetworkClient> client, std::string extension);
    static void PreLoginExtensions(std::shared_ptr<NetworkClient> client);
    static void AfterMapActions(std::shared_ptr<NetworkClient> client);
    static void AfterLoginActions(std::shared_ptr<NetworkClient> client);
    static void PreEntityActions();
    static void PostEntityActions(std::shared_ptr<NetworkClient> client, std::shared_ptr<Entity> postEntity);
    //static std::map<std::string, int> SupportedExtensions { std::pair<std::string, int>("CustomBlocks", 1)};
};
#endif