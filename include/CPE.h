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
#define PLAYER_CLICK_EXT_NAME "PlayerClick"
#define TWOWAY_PING_EXT_NAME "TwoWayPing"
#define BLOCK_DEFS_EXT_NAME "BlockDefinitions"
#define BLOCK_DEFS_EXTENDED_EXT_NAME "BlockDefinitionsExt"
#define EXTENDED_TEXTURES_EXT_NAME "ExtendedTextures"
#define FULL_CODEPAGE_EXT_NAME "FullCP437"
#define CUSTOM_PARTICLES_EXT_NAME "CustomParticles"
#define CUSTOM_MODELS_EXT_NAME "CustomModels"
#define TEXT_COLORS_EXT_NAME "TextColors"
#define MAP_ASPECT_EXT_NAME "EnvMapAspect"
#define ENTITY_PROPERTIES_EXT_NAME "EntityProperty"
#define INVENTORY_ORDER_EXT_NAME "InventoryOrder"
#define SET_HOTBAR_EXT_NAME "SetHotbar"


#include <memory>
#include <string>

class IMinecraftClient;
class Entity;

class CPE {
    public:
    static int GetClientExtVersion(const std::shared_ptr<IMinecraftClient>& client, const std::string& extension);
    static void PreLoginExtensions(const std::shared_ptr<IMinecraftClient>& client);
    static void AfterMapActions(const std::shared_ptr<IMinecraftClient>& client);
    static void AfterLoginActions(const std::shared_ptr<IMinecraftClient>& client);
    static void PreEntityActions();
    static void PostEntityActions(const std::shared_ptr<IMinecraftClient>& client, const std::shared_ptr<Entity>& postEntity);
    static void DuringMapActions(const std::shared_ptr<IMinecraftClient>& client);
};
#endif