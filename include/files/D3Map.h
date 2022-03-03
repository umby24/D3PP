#pragma once
#include <string>
#include <vector>
#include "common/Vectors.h"
#include "common/MinecraftLocation.h"
#include "common/PreferenceLoader.h"

namespace D3PP::files {
#define D3_MAP_CONFIG_NAME "Config.txt"
#define D3_MAP_BLOCKS_NAME "Data-Layer.gz"
#define D3_MAP_RBOX_NAME "Rank-Layer.txt" // -- Aka regions
#define D3_MAP_PORTALS_NAME "Teleporter.txt"
#define D3_MAP_FILE_VERSION 1050

		enum D3OverviewType {
			None,
			TwoDee,
			Iso
		};

        struct MapTeleporterElement {
            std::string Id;
            short X0;
            short Y0;
            short Z0;
            short X1;
            short Y1;
            short Z1;
            std::string DestMapUniqueId;
            int DestMapId;
            float DestX;
            float DestY;
            float DestZ;
            float DestRot;
            float DestLook;

			MapTeleporterElement() {
				Id = "";
				X0 = 0;
				Y0 = 0;
				Z0 = 0;
				X1 = 0;
				Y1 = 0;
				Z1 = 0;
				DestX = 0;
				DestY = 0;
				DestZ = 0;
				DestRot = 0;
				DestLook = 0;
				DestMapId = 0;
				DestMapUniqueId = "";
			}
        };

        struct MapRankElement {
            unsigned short X0;
            unsigned short Y0;
            unsigned short Z0;
            unsigned short X1;
            unsigned short Y1;
            unsigned short Z1;
            short Rank;
        };

		class D3Map {
		private:
			PreferenceLoader m_configFile;
			std::string mapPath;
			
		public:
			Common::Vector3S MapSize;
			MinecraftLocation MapSpawn;
			
			std::string UUID, Name, MOTD, TextureUrl;
			int BuildRank{}, JoinRank{}, ShowRank{}, SaveInterval{}, ServerVersion{}, JumpHeight{};
			bool PhysicsEnabled{}, ColorsSet{}, CustomAppearance{};
            unsigned char SideBlock{}, EdgeBlock{};
            short SideLevel{};
            int SkyColor{}, CloudColor{}, FogColor{}, alight{}, dlight{};
            bool Flying{}, NoClip{}, Speeding{}, SpawnControl{}, ThirdPerson{}, Weather{};

			D3OverviewType OverviewType;
			std::vector<unsigned char> MapData;
            std::map<std::string, MapTeleporterElement> Teleporter;
            std::vector<MapRankElement> RankBoxes;
			// -----------------------
			D3Map(const std::string& folder);
			D3Map(const std::string& folder, const std::string& name, const Common::Vector3S& mapSize);

			bool Load();
			bool Load(std::string path);
			bool Save();
			bool Save(std::string path);
			void Resize(Common::Vector3S newSize);

			unsigned char GetBlock(Common::Vector3S blockLocation);
			unsigned char GetBlockMetadata(Common::Vector3S blockLocation);
			short GetBlockLastPlayer(Common::Vector3S blockLocation);

			void SetBlock(Common::Vector3S blockLocation, unsigned char type);
			void SetBlockMetadata(Common::Vector3S blockLocation, unsigned char metadata);
			void SetBlockLastPlayer(Common::Vector3S blockLocation, short playerNumber);
		private:
			std::string GenerateUuid();
			bool SaveConfig();
			bool SavePortals();
			bool SaveRankBoxes();
			bool SaveMapData();
			bool ReadConfig();
			bool ReadMapData();
			void ReadPortals();
			void ReadRankBoxes();

			bool BlockInBounds(Common::Vector3S loc);
			int GetBlockIndex(Common::Vector3S loc);
			int GetBlockIndex(Common::Vector3S loc, int sizeX, int sizeY);
		};
	}
