#include "files/D3Map.h"

#include "common/Logger.h"
#include "Utils.h"
#include "compression.h"
#include "world/CustomParticle.h"
#include <climits>

namespace D3PP::files {
		D3Map::D3Map(const std::string& folder) :
        MapSize{},
        MapSpawn{},
        m_configFile(D3_MAP_CONFIG_NAME, folder),
        Particles(),
        Teleporter()
		{
            mapPath = folder;

            if (!mapPath.ends_with("/"))
                mapPath += "/";

            BuildRank = 0;
            JoinRank = 0;
            ShowRank = 0;
            OverviewType = D3OverviewType::Iso;
            dataChanged = true;
            portalsChanged = true;
            configChanged = true;
            particleChanged = true;
            rboxChanged = true;
		}

		D3Map::D3Map(const std::string& folder, const std::string& name, const Common::Vector3S& mapSize) :
        m_configFile(D3_MAP_CONFIG_NAME, folder),
        Particles(),
        Teleporter()
		{
            mapPath = folder;
            if (!mapPath.ends_with("/"))
                mapPath += "/";
            SaveInterval = 10;
            ServerVersion = 1004;
            OverviewType = D3OverviewType::Iso;
            Name= name;
            MapSize = mapSize;
            dataChanged = true;
            portalsChanged = true;
            configChanged = true;
            particleChanged = true;
            rboxChanged = true;
            Common::Vector3S defaultSpawn {
                static_cast<short>(MapSize.X/2),
                static_cast<short>(MapSize.Y/2),
                static_cast<short>(MapSize.Z/2)};

            MapSpawn.SetAsBlockCoords(defaultSpawn);
            MapData.resize((mapSize.X * mapSize.Y * mapSize.Z) * 4);
		}

		bool D3Map::Load()
		{
            if (!std::filesystem::exists(mapPath)) {
                return false;
            }

            if (!mapPath.ends_with("/")) {
                mapPath += "/";
            }

            bool loadResult = ReadConfig() && ReadMapData();
            ReadRankBoxes();
            ReadPortals();
            ReadParticles();
            return loadResult;
		}
        bool D3Map::Load(std::string path)
        {
            if (!std::filesystem::exists(path)) {
                return false;
            }

            if (!path.ends_with("/")) {
                path += "/";
            }

            std::string ogMapPath = mapPath;
            mapPath = path;
            bool loadResult = ReadConfig() && ReadMapData();
            ReadRankBoxes();
            ReadPortals();
            ReadParticles();
            mapPath = ogMapPath;
            return loadResult;
        }
		bool D3Map::Save()
		{
            std::filesystem::create_directory(mapPath);
            bool result = false;
            result = SaveConfig();

            if (result)
                result = SaveRankBoxes();

            if (result)
                result = SavePortals();

            if (result)
                result = SaveMapData();

            if (result)
                result = SaveParticles();

			return result;
		}
        bool D3Map::Save(std::string filePath)
        {
            std::filesystem::create_directory(filePath);
            std::string oldPath = mapPath;
            mapPath = filePath;

            bool result = false;
            result = SaveConfig();

            if (result)
                result = SaveRankBoxes();

            if (result)
                result = SavePortals();

            if (result)
                result = SaveMapData();

            if (result)
                result = SaveParticles();

            mapPath = oldPath;
            return result;
        }
        void D3Map::Resize(Common::Vector3S newSize) {
            int newMapSize = (newSize.X * newSize.Y * newSize.Z) * 4;
            // -- Spawn limiting..
            int spawnX, spawnY, spawnZ;
            if (MapSpawn.X() > newSize.X)
                spawnX = newSize.X-1;

            if (MapSpawn.Y() > newSize.Y)
                spawnY = newSize.Y - 1;

            if (MapSpawn.Z() > newSize.Z)
                spawnZ = newSize.Z - 1;

            MapSize = newSize;
            MapData.resize(newMapSize);
            dataChanged = true;
        }

        unsigned char D3Map::GetBlock(Common::Vector3S blockLocation) {
            if (!BlockInBounds(blockLocation))
                return 0;

            int index = GetBlockIndex(blockLocation);
            return MapData[(index*4)];
        }

        unsigned char D3Map::GetBlockMetadata(Common::Vector3S blockLocation) {
            if (!BlockInBounds(blockLocation))
                return 0;

            int index = GetBlockIndex(blockLocation);
            return MapData[(index*4)+1];
        }

        short D3Map::GetBlockLastPlayer(Common::Vector3S blockLocation) {
            if (!BlockInBounds(blockLocation))
                return -1;
            int index = GetBlockIndex(blockLocation);
            short result = 0;
            result |= MapData[(index*4)+2] << 8;
            result |= MapData[(index*4)+3];

            return result;
        }

        void D3Map::SetBlock(Common::Vector3S blockLocation, unsigned char type) {
            if (!BlockInBounds(blockLocation))
                return;

            int index = GetBlockIndex(blockLocation);
            MapData[(index*4)] = type;
            dataChanged = true;
        }

        void D3Map::SetBlockMetadata(Common::Vector3S blockLocation, unsigned char metadata) {
            if (!BlockInBounds(blockLocation))
                return;

            int index = GetBlockIndex(blockLocation);
            MapData[(index*4)+1] = metadata;
            dataChanged = true;
        }

        void D3Map::SetBlockLastPlayer(Common::Vector3S blockLocation, short playerNumber) {
            if (!BlockInBounds(blockLocation))
                return;

            int index = GetBlockIndex(blockLocation);
            MapData[(index*4)+2] = (playerNumber & 0xFF00) >> 8;
            MapData[(index*4)+3] = (playerNumber&0xFF);
            dataChanged = true;

        }

        std::string D3Map::GenerateUuid() {
            std::string result;

            for(auto i = 1; i < 16; i++)
                result += char(65 + Utils::RandomNumber(25));

            return result;
        }

        bool D3Map::SaveConfig() {
            if (!configChanged)
                return true;

            std::string configName = mapPath + D3_MAP_CONFIG_NAME;
            std::ofstream oStream(configName);

            if (oStream.is_open()) {
                oStream << "; Overview_Types: 0=Nothing, 1=2D, 2=Iso(fast)" << "\n";
                oStream << "; Save_Interval: in minutes (0 = Disabled)" << "\n";
                oStream << "; Jumpheight: -1 = Default" << "\n";
                oStream << ";" << "\n";
                oStream << "Server_Version = 1050" << "\n";
                oStream << "Unique_ID = " << UUID << "\n";
                oStream << "Name = " + Name << "\n";
                oStream << "Rank_Build = " << BuildRank << "\n";
                oStream << "Rank_Join = " << JoinRank << "\n";
                oStream << "Rank_Show = " << ShowRank << "\n";
                oStream << "Physic_Stopped = " << PhysicsEnabled << "\n";
                oStream << "MOTD_Override = " << MOTD << "\n";
                oStream << "Save_Intervall = " << SaveInterval << "\n";
                oStream << "Overview_Type = " << OverviewType << "\n";
                oStream << "Size_X = " << MapSize.X << "\n";
                oStream << "Size_Y = " << MapSize.Y << "\n";
                oStream << "Size_Z = " << MapSize.Z << "\n";

                oStream << "Spawn_X = " << MapSpawn.X() / 32.0f  << "\n";
                oStream << "Spawn_Y = " << MapSpawn.Y() / 32.0f << "\n";
                oStream << "Spawn_Z = " << (MapSpawn.Z() -51) / 32.0f << "\n";

                oStream << "Spawn_Rot = " << MapSpawn.Rotation << "\n";
                oStream << "Spawn_Look = " << MapSpawn.Look << "\n";
                oStream << "Colors_Set = " << ColorsSet << "\n";
                oStream << "Sky_Color = " << SkyColor << "\n";
                oStream << "Cloud_Color = " << CloudColor << "\n";
                oStream << "Fog_Color = " << FogColor << "\n";
                oStream << "A_Light = " << alight << "\n";
                oStream << "D_Light = " << dlight << "\n";
                oStream << "Custom_Appearance = " << CustomAppearance << "\n";
                oStream << "Custom_Texture_Url = " << TextureUrl << "\n";
                oStream << "Custom_Side_Block = " << (int)SideBlock << "\n";
                oStream << "Custom_Edge_Block = " << (int)EdgeBlock << "\n";
                oStream << "Custom_Side_Level = " << SideLevel << "\n";
                oStream << "Allow_Flying = " << Flying << "\n";
                oStream << "Allow_Noclip = " << NoClip << "\n";
                oStream << "Allow_Fastwalk = " << Speeding << "\n";
                oStream << "Allow_Respawn = " << SpawnControl << "\n";
                oStream << "Allow_Thirdperson = " << ThirdPerson << "\n";
                oStream << "Allow_Weatherchange = " << Weather << "\n";
                oStream << "Jumpheight = " << JumpHeight << "\n";
                oStream << "cloudHeight = " << cloudHeight << "\n";
                oStream << "maxFogDistance = " << maxFogDistance << "\n";
                oStream << "cloudSpeed = " << cloudSpeed << "\n";
                oStream << "weatherSpeed = " << weatherSpeed << "\n";
                oStream << "weatherFade = " << weatherFade << "\n";
                oStream << "expoFog = " << expoFog << "\n";
                oStream << "mapSideOffset = " << mapSideOffset << "\n";

                oStream.close();
                Logger::LogAdd("D3Map", "File saved [" + mapPath + D3_MAP_CONFIG_NAME + "]", LogType::NORMAL, GLF);
                configChanged = false;
                return true;
            }
            return false;
        }

        bool D3Map::SavePortals() {
            if (!portalsChanged)
                return true;
            PreferenceLoader pLoader(D3_MAP_PORTALS_NAME, mapPath, true);
            for (auto const &tp : Teleporter) {
                pLoader.SelectGroup(tp.first);
                pLoader.Write("X_0", tp.second.X0);
                pLoader.Write("Y_0", tp.second.Y0);
                pLoader.Write("Z_0", tp.second.Z0);
                pLoader.Write("X_1", tp.second.X1);
                pLoader.Write("Y_1", tp.second.Y1);
                pLoader.Write("Z_1", tp.second.Z1);
                pLoader.Write("Dest_Map_Unique_ID", tp.second.DestMapUniqueId);
                pLoader.Write("Dest_Map_ID", tp.second.DestMapId);
                pLoader.Write("Dest_X", tp.second.DestX);
                pLoader.Write("Dest_Y", tp.second.DestY);
                pLoader.Write("Dest_Z", tp.second.DestZ);
                pLoader.Write("Dest_Rot", tp.second.DestRot);
                pLoader.Write("Dest_Look", tp.second.DestLook);
            }
            pLoader.SaveFile();
            Logger::LogAdd("D3Map", "File saved [" + mapPath + D3_MAP_PORTALS_NAME + "]", LogType::NORMAL, GLF);
            portalsChanged = false;
            return true;
        }

        bool D3Map::SaveRankBoxes() {
            if (!rboxChanged)
                return true;

            PreferenceLoader pLoader(D3_MAP_RBOX_NAME, mapPath, true);
            int rBoxNumber = 0;
            for(auto const &rb : RankBoxes) {
                pLoader.SelectGroup(stringulate(rBoxNumber));
                pLoader.Write("X_0", rb.X0);
                pLoader.Write("Y_0", rb.Y0);
                pLoader.Write("Z_0", rb.Z0);
                pLoader.Write("X_1", rb.X1);
                pLoader.Write("Y_1", rb.Y1);
                pLoader.Write("Z_1", rb.Z1);
                pLoader.Write("Rank", rb.Rank);
            }
            pLoader.SaveFile();
            Logger::LogAdd("D3Map", "File saved [" + mapPath + D3_MAP_RBOX_NAME + "]", LogType::NORMAL, GLF);
            rboxChanged = false;
            return true;
        }

        bool D3Map::SaveMapData() {
            if (!dataChanged)
                return true;
            int mapDataSize = MapSize.X * MapSize.Y * MapSize.Z * 4;

            if (!GZIP::GZip_CompressToFile(MapData.data(), mapDataSize, "temp.gz"))
                return false;

            try {
                if (std::filesystem::exists(mapPath + D3_MAP_BLOCKS_NAME))
                    std::filesystem::remove(mapPath + D3_MAP_BLOCKS_NAME);

                std::filesystem::copy("temp.gz", mapPath + D3_MAP_BLOCKS_NAME, std::filesystem::copy_options::overwrite_existing);
            } catch (std::filesystem::filesystem_error& e) {
                Logger::LogAdd("D3Map", "Could not copy mapfile: " + stringulate(e.what()), LogType::L_ERROR, GLF);
                return false;
            }

            Logger::LogAdd("D3Map", "File saved [" + mapPath + D3_MAP_BLOCKS_NAME + "]", LogType::NORMAL, GLF);
            dataChanged = false;
            return true;
        }

        bool D3Map::ReadConfig() {
            PreferenceLoader pLoader(D3_MAP_CONFIG_NAME, mapPath);
            pLoader.LoadFile();
            
            int sizeX = pLoader.Read("Size_X", 0);
            int sizeY = pLoader.Read("Size_Y", 0);
            int sizeZ = pLoader.Read("Size_Z", 0);
            int mapSize = sizeX * sizeY * sizeZ;
            
            if (mapSize == 0)
                return false;

            Common::Vector3S newSize {
                static_cast<short>(sizeX),
                static_cast<short>(sizeY),
                static_cast<short>(sizeZ) 
            };
            Resize(newSize);

            UUID = pLoader.Read("Unique_ID", GenerateUuid());
            Name = pLoader.Read("Name", "D3Map");
            BuildRank = pLoader.Read("Rank_Build", 0);
            JoinRank = pLoader.Read("Rank_Join", 0);
            ShowRank = pLoader.Read("Rank_Show", 0);
            PhysicsEnabled = !pLoader.Read("Physic_Stopped", 0);
            MOTD = pLoader.Read("MOTD_Override", "");
            SaveInterval = pLoader.Read("Save_Intervall", 10);
            OverviewType = static_cast<D3OverviewType>(pLoader.Read("Overview_Type", 2));
            float SpawnX = stof(pLoader.Read("Spawn_X", "1"));
            float SpawnY = stof(pLoader.Read("Spawn_Y", "1"));
            float SpawnZ = stof(pLoader.Read("Spawn_Z", "0"));
            MapSpawn.SetAsPlayerCoords(Common::Vector3F{SpawnX, SpawnY, SpawnZ});
            MapSpawn.Rotation = stof(pLoader.Read("Spawn_Rot", "0"));
            MapSpawn.Look=stof(pLoader.Read("Spawn_Look", "0"));
            ColorsSet = (pLoader.Read("Colors_Set", 0) > 0);
            SkyColor = pLoader.Read("Sky_Color", -1);
            CloudColor = pLoader.Read("Cloud_Color", -1);
            FogColor = pLoader.Read("Fog_Color", -1);
            alight = pLoader.Read("A_Light", -1);
            dlight = pLoader.Read("D_Light", -1);
            CustomAppearance = (pLoader.Read("Custom_Appearance", 0) > 0);
            TextureUrl = pLoader.Read("Custom_Texture_Url", "");
            SideLevel = static_cast<short>(pLoader.Read("Custom_Side_Level", -1));
            EdgeBlock = pLoader.Read("Custom_Edge_Block", -1);
            SideBlock = pLoader.Read("Custom_Side_Block", -1);
            Flying = (pLoader.Read("Allow_Flying", 1) > 0);
            NoClip = (pLoader.Read("Allow_Noclip", 1) > 0);
            Speeding = (pLoader.Read("Allow_Fastwalk", 1) > 0);
            SpawnControl = (pLoader.Read("Allow_Respawn", 1) > 0);
            ThirdPerson = (pLoader.Read("Allow_Thirdperson", 1) > 0);
            Weather = (pLoader.Read("Allow_Weatherchange", 1) > 0);
            JumpHeight = pLoader.Read("Jumpheight", -1);

            cloudHeight = pLoader.Read("cloudHeight", sizeZ+2);
            maxFogDistance = pLoader.Read("maxFogDistance", 0);
            cloudSpeed = pLoader.Read("cloudSpeed", 256);
            weatherSpeed = pLoader.Read("weatherSpeed", 256);
            weatherFade = pLoader.Read("weatherFade", 128);
            expoFog = pLoader.Read("expoFog", 0);
            mapSideOffset = pLoader.Read("mapSideOffset", -2);

            // -- Sanity checks
            if (JumpHeight > 1000 || JumpHeight < -1)
                JumpHeight = -1;

            if (BuildRank > SHRT_MAX || BuildRank < 0)
                BuildRank = 0;

            if (JoinRank > SHRT_MAX || JoinRank < 0)
                JoinRank = 0;

            if (ShowRank > SHRT_MAX || ShowRank < 0)
                ShowRank = 0;

            if (SaveInterval > 1000 || SaveInterval < 0)
                SaveInterval = 10;

            if (SkyColor < -1)
                SkyColor = -1;

            if (FogColor < -1)
                FogColor = -1;

            if (CloudColor < -1)
                CloudColor = -1;

            if (alight < -1)
                alight = -1;

            if (dlight < -1)
                dlight = -1;
            
            if (expoFog > 1)
                expoFog = 1;

            return true;
        }

        bool D3Map::ReadMapData() {
            int mapSize = MapSize.X* MapSize.Y*MapSize.Z;
            int dSize = GZIP::GZip_DecompressFromFile(reinterpret_cast<unsigned char*>(MapData.data()), mapSize * 4, mapPath + D3_MAP_BLOCKS_NAME);

            if (dSize == (mapSize * 4)) {
                Logger::LogAdd("D3Map", "Map Loaded [" + mapPath + D3_MAP_BLOCKS_NAME + "] (" + stringulate(MapSize.X) + "x" + stringulate(MapSize.Y) + "x" + stringulate(MapSize.Z) + ")", LogType::NORMAL, GLF);
                return true;
            }

            Logger::LogAdd("D3Map", "Error loading map [" + mapPath + D3_MAP_BLOCKS_NAME + "]!", L_ERROR, GLF);
            return false;
            
        }

        void D3Map::ReadPortals() {
            PreferenceLoader tpLoader(D3_MAP_PORTALS_NAME, mapPath);
            tpLoader.LoadFile();

            for (auto const &tp : tpLoader.SettingsDictionary) {
                tpLoader.SelectGroup(tp.first);
                MapTeleporterElement tpe{};
                tpe.Id = tp.first;
                tpe.X0 = tpLoader.Read("X_0", 0);
                tpe.Y0 = tpLoader.Read("Y_0", 0);
                tpe.Z0 = tpLoader.Read("Z_0", 0);
                tpe.X1 = tpLoader.Read("X_1", 0);
                tpe.Y1 = tpLoader.Read("Y_1", 0);
                tpe.Z1 = tpLoader.Read("Z_1", 0);

                tpe.DestMapUniqueId = tpLoader.Read("Dest_Map_Unique_ID", "");
                tpe.DestMapId = tpLoader.Read("Dest_Map_ID", -1);
                tpe.DestX = tpLoader.Read("Dest_X", -1);
                tpe.DestY = tpLoader.Read("Dest_Y", -1);
                tpe.DestZ = tpLoader.Read("Dest_Z", -1);
                tpe.DestRot = tpLoader.Read("Dest_Rot", 0);
                tpe.DestLook = tpLoader.Read("Dest_Look", 0);
                Teleporter.insert(std::make_pair(tpe.Id, tpe));
            }
        }

        void D3Map::ReadRankBoxes() {
            PreferenceLoader rBoxLoader(D3_MAP_RBOX_NAME, mapPath);
            rBoxLoader.LoadFile();
            for (auto const &fi : rBoxLoader.SettingsDictionary) {
                rBoxLoader.SelectGroup(fi.first);
                MapRankElement mre{};
                mre.X0 = rBoxLoader.Read("X_0", 0);
                mre.Y0 = rBoxLoader.Read("Y_0", 0);
                mre.Z0 = rBoxLoader.Read("Z_0", 0);
                mre.X1 = rBoxLoader.Read("X_1", 0);
                mre.Y1 = rBoxLoader.Read("Y_1", 0);
                mre.Z1 = rBoxLoader.Read("Z_1", 0);
                mre.Rank = rBoxLoader.Read("Rank", 0);
                RankBoxes.push_back(mre);
            }
        }

        bool D3Map::BlockInBounds(Common::Vector3S loc) {
            return (loc.X >= 0 && loc.X < MapSize.X && loc.Y >= 0 && loc.Y < MapSize.Y && loc.Z >= 0 && loc.Z < MapSize.Z);
        }

        int D3Map::GetBlockIndex(Common::Vector3S loc) {
            return GetBlockIndex(loc, MapSize.X, MapSize.Y);
        }

        int D3Map::GetBlockIndex(Common::Vector3S loc, int sizeX, int sizeY) {
            return (loc.X + loc.Y * sizeX + loc.Z * sizeX * sizeY) * 1;
        }

    std::vector<MapTeleporterElement> D3Map::getPortals() {
        std::vector<MapTeleporterElement> result;

        for(const auto &kvp : Teleporter) {
            result.push_back(kvp.second);
        }

        return result;
    }

    void D3Map::SetPortals(std::vector<MapTeleporterElement> portals) {
        Teleporter.clear();

        for (const auto item : portals) {
            Teleporter.insert(std::make_pair(item.Id, item));
        }
    }

    bool D3Map::SaveParticles() {
        if (!particleChanged)
            return true;
        PreferenceLoader pLoader(D3_MAP_PARTICLES_NAME, mapPath, true);
        int rBoxNumber = 0;
        for(auto const &rb : Particles) {
            pLoader.SelectGroup(stringulate(rb.effectId));
            pLoader.Write("U1", rb.U1);
            pLoader.Write("V1", rb.V1);
            pLoader.Write("U2", rb.U2);
            pLoader.Write("V2", rb.V2);
            pLoader.Write("Tint", Utils::Rgb(rb.redTint, rb.greenTint, rb.blueTint));
            pLoader.Write("Frame_Count", rb.frameCount);
            pLoader.Write("Particle_Count", rb.particleCount);
            pLoader.Write("Size", rb.size);
            pLoader.Write("Size_Variation", rb.sizeVariation);
            pLoader.Write("Spread", rb.spread);
            pLoader.Write("Speed", rb.speed);
            pLoader.Write("Gravity", rb.gravity);
            pLoader.Write("Base_Life_Time", rb.baseLifetime);
            pLoader.Write("Lifetime_Variation", rb.lifetimeVariation);
            pLoader.Write("Collide_Flags", rb.collideFlags);
            pLoader.Write("Full_Bright", rb.fullBright);
        }

        pLoader.SaveFile();
        Logger::LogAdd("D3Map", "File saved [" + mapPath + D3_MAP_PARTICLES_NAME + "]", LogType::NORMAL, GLF);
        particleChanged = false;
        return true;
    }

    void D3Map::ReadParticles() {
        PreferenceLoader rBoxLoader(D3_MAP_PARTICLES_NAME, mapPath);
        rBoxLoader.LoadFile();

        for (auto const &fi : rBoxLoader.SettingsDictionary) {
            rBoxLoader.SelectGroup(fi.first);
            D3PP::world::CustomParticle particle;
            particle.U1 = rBoxLoader.Read("U1", 0);
            particle.V1 = rBoxLoader.Read("V1", 0);
            particle.U2 = rBoxLoader.Read("U2", 0);
            particle.V2 = rBoxLoader.Read("V2", 0);
            particle.redTint = Utils::RedVal(rBoxLoader.Read("Tint", 0));
            particle.greenTint = Utils::GreenVal(rBoxLoader.Read("Tint", 0));
            particle.blueTint = Utils::BlueVal(rBoxLoader.Read("Tint", 0));
            particle.frameCount = rBoxLoader.Read("Frame_Count", 0);
            particle.particleCount = rBoxLoader.Read("Particle_Count", 0);
            particle.size = rBoxLoader.Read("Size", 0);
            particle.sizeVariation = rBoxLoader.Read("Size_Variation", 0);
            particle.spread = rBoxLoader.Read("Spread", 0);
            particle.speed = rBoxLoader.Read("Speed", 0);
            particle.gravity = rBoxLoader.Read("Gravity", 0);
            particle.baseLifetime = rBoxLoader.Read("Base_Life_Time", 0);
            particle.lifetimeVariation = rBoxLoader.Read("Lifetime_Variation", 0);
            particle.collideFlags = rBoxLoader.Read("Collide_Flags", 0);
            particle.fullBright = rBoxLoader.Read("Full_Bright", 0);
            Particles.push_back(particle);
        }
    }

    std::vector<D3PP::world::CustomParticle> D3Map::GetParticles() {
        return Particles;
    }

    void D3Map::SetParticles(std::vector<D3PP::world::CustomParticle> inParts) {
        Particles = inParts;
        particleChanged = true;
    }
}