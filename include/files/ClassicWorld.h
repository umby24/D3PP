//
// Created by Wande on 7/25/2022.
//

#ifndef D3PP_CLASSICWORLD_H
#define D3PP_CLASSICWORLD_H
#include <vector>
#include <map>
#include "common/Vectors.h"
#include "Nbt/cppNbt.h"

namespace D3PP::files {
    class IMetadataStructure {
    public:
        IMetadataStructure() = default;
        virtual ~IMetadataStructure() = default;

        virtual Nbt::TagCompound Read(Nbt::TagCompound metadata) = 0;
        virtual Nbt::TagCompound Write() = 0;
    };

    class ForeignMeta : public IMetadataStructure {
    public:
        Nbt::TagCompound Read(Nbt::TagCompound metadata) override {
            std::vector<std::string> toRemove;
            for (auto const &t : metadata.data) {
                if (!std::holds_alternative<Nbt::TagCompound>(t.second))
                    continue;

                Nbt::TagCompound foreign = std::get<Nbt::TagCompound>(t.second);
                foreign.name = t.first;
                m_tags.push_back(foreign);
                toRemove.push_back(t.first);
            }
            for(const auto &t : toRemove) {
                metadata.data.erase(t);
            }

            return metadata;
        }

        Nbt::TagCompound Write() override {
            Nbt::TagCompound result;
            result.name = "Metadata";

            if (m_tags.empty())
                return result;

            for(const auto &t : m_tags) {
                if (std::holds_alternative<Nbt::TagCompound>(t)) {
                    Nbt::TagCompound foreign = std::get<Nbt::TagCompound>(t);
                    result.data.insert({foreign.name, {foreign}});
                }
            }

            return result;
        }
    private:
        std::vector<Nbt::Tag> m_tags;
    };

    class CPEMetadata : public IMetadataStructure {
    public:
        int ClickDistanceVersion;
        short ClickDistance;

        int CustomBlocksVersion;
        short CustomBlocksLevel;
        std::vector<unsigned char> CustomBlocksFallback;

        int EnvColorsVersion;
        Common::Vector3S SkyColor;
        Common::Vector3S CloudColor;
        Common::Vector3S FogColor;
        Common::Vector3S AmbientColor;
        Common::Vector3S SunlightColor;

        int EnvMapAppearanceVersion;
        std::string TextureUrl;
        unsigned char SideBlock;
        unsigned char EdgeBlock;
        short SideLevel;
        unsigned char Weather;

        int EnvMapAspectVersion;
        std::map<unsigned char, int> MapEnvProperties;

        int HackControlVersion;
        bool CanFly, CanClip, CanSpeed, CanRespawn, CanThirdPerson, CanSetWeather;
        short JumpHeight;

        Nbt::TagCompound Read(Nbt::TagCompound metadata) override {
            if (!metadata.data.contains("CPE"))
                return metadata;

            Nbt::TagCompound cpeBase = std::get<Nbt::TagCompound>(metadata["CPE"]);
            if (cpeBase.data.contains("ClickDistance")) {
                auto cdBase = std::get<Nbt::TagCompound>(cpeBase["ClickDistance"]);
                if (cdBase.data.contains("ExtensionVersion")) {
                    auto extVersion = std::get<Nbt::TagInt>(cdBase["ExtensionVersion"]);
                    auto distance = std::get<Nbt::TagShort>(cdBase["Distance"]);
                    ClickDistanceVersion = extVersion;
                    ClickDistance = distance;
                }
            }
            if (cpeBase.data.contains("CustomBlocks")) {
                auto cdBase = std::get<Nbt::TagCompound>(cpeBase["CustomBlocks"]);
                if (cdBase.data.contains("ExtensionVersion")) {
                    auto extVersion = std::get<Nbt::TagInt>(cdBase["ExtensionVersion"]);
                    auto level = std::get<Nbt::TagShort>(cdBase["SupportLevel"]);
                    auto fallback = std::get<Nbt::TagByteArray>(cdBase["Fallback"]);
                    CustomBlocksVersion = extVersion;
                    CustomBlocksLevel = level;

                    for(auto q : fallback) // -- Converting signed -> unsigned
                        CustomBlocksFallback.push_back(static_cast<unsigned char>(q));
                }
            }
            if (cpeBase.data.contains("EnvColors")) {
                auto cdBase = std::get<Nbt::TagCompound>(cpeBase["EnvColors"]);
                if (cdBase.data.contains("ExtensionVersion")) {
                    auto extVersion = std::get<Nbt::TagInt>(cdBase["ExtensionVersion"]);
                    auto skyBase = std::get<Nbt::TagCompound>(cdBase["Sky"]);
                    auto cloudBase = std::get<Nbt::TagCompound>(cdBase["Cloud"]);
                    auto fogBase = std::get<Nbt::TagCompound>(cdBase["Fog"]);
                    auto ambientBase = std::get<Nbt::TagCompound>(cdBase["Ambient"]);
                    auto sunlightBase = std::get<Nbt::TagCompound>(cdBase["Sunlight"]);

                    EnvColorsVersion = extVersion;
                    SkyColor = ReadColor(skyBase);
                    CloudColor = ReadColor(cloudBase);
                    FogColor = ReadColor(fogBase);
                    AmbientColor = ReadColor(ambientBase);
                    SunlightColor = ReadColor(sunlightBase);
                }
            }
            if (cpeBase.data.contains("EnvMapAppearance")) {
                auto cdBase = std::get<Nbt::TagCompound>(cpeBase["EnvMapAppearance"]);
                if (cdBase.data.contains("ExtensionVersion")) {
                    auto extVersion = std::get<Nbt::TagInt>(cdBase["ExtensionVersion"]);
                    auto textUrl = std::get<Nbt::TagString>(cdBase["TextureURL"]);
                    auto sideBlock = std::get<Nbt::TagByte>(cdBase["SideBlock"]);
                    auto edgeBlock = std::get<Nbt::TagByte>(cdBase["EdgeBlock"]);
                    auto sideLevel = std::get<Nbt::TagShort>(cdBase["SideLevel"]);

                    EnvMapAppearanceVersion = extVersion;
                    TextureUrl = textUrl;
                    SideBlock = sideBlock;
                    SideLevel = sideLevel;
                    EdgeBlock = edgeBlock;
                }
            }
            if (cpeBase.data.contains("EnvWeatherType")) {
                auto cdBase = std::get<Nbt::TagCompound>(cpeBase["EnvWeatherType"]);
                if (cdBase.data.contains("ExtensionVersion")) {
                    auto extVersion = std::get<Nbt::TagInt>(cdBase["ExtensionVersion"]);
                    auto wType = std::get<Nbt::TagByte>(cdBase["WeatherType"]);
                    Weather = wType;
                }
            }
            if (cpeBase.data.contains("EnvMapAspect")) {
                auto cdBase = std::get<Nbt::TagCompound>(cpeBase["EnvMapAspect"]);
                if (cdBase.data.contains("ExtensionVersion")) {
                    auto extVersion = std::get<Nbt::TagInt>(cdBase["ExtensionVersion"]);
                    auto propsList = std::get<Nbt::TagList>(cdBase["Properties"]);
                    bool isIntList = std::holds_alternative<std::vector<Nbt::TagInt>>(propsList.base);
                    if (isIntList) {
                        EnvMapAspectVersion = extVersion;
                        MapEnvProperties = std::map<unsigned char, int>();
                        auto propInts = std::get<std::vector<Nbt::TagInt>>(propsList.base);

                        for(unsigned char i = 0; i < propInts.size(); i++) {
                            MapEnvProperties.insert(std::make_pair(i, propInts.at(i)));
                        }
                    }
                }
            }
            if (cpeBase.data.contains("HackControl")) {
                auto cdBase = std::get<Nbt::TagCompound>(cpeBase["HackControl"]);
                if (cdBase.data.contains("ExtensionVersion")) {
                    auto extVersion = std::get<Nbt::TagInt>(cdBase["ExtensionVersion"]);
                    auto flying = std::get<Nbt::TagByte>(cdBase["CanFly"]);
                    auto noclip = std::get<Nbt::TagByte>(cdBase["CanClip"]);
                    auto speeding = std::get<Nbt::TagByte>(cdBase["CanSpeed"]);
                    auto spawncontrol = std::get<Nbt::TagByte>(cdBase["CanRespawn"]);
                    auto thirdperson = std::get<Nbt::TagByte>(cdBase["CanThirdPerson"]);
                    auto weather = std::get<Nbt::TagByte>(cdBase["CanSetWeather"]);
                    auto jumpheight = std::get<Nbt::TagShort>(cdBase["JumpHeight"]);

                    HackControlVersion = extVersion;
                    CanFly = flying;
                    CanClip = noclip;
                    CanSpeed = speeding;
                    CanRespawn = spawncontrol;
                    CanThirdPerson = thirdperson;
                    CanSetWeather = weather;
                    JumpHeight = jumpheight;
                }
            }

            metadata.data.erase("CPE");
            return metadata;
        }

        Common::Vector3S ReadColor(Nbt::TagCompound base) {
            Common::Vector3S result;
            short red = std::get<Nbt::TagShort>(base["R"]);
            short green = std::get<Nbt::TagShort>(base["G"]);
            short blue = std::get<Nbt::TagShort>(base["B"]);

            result.X = red;
            result.Y = green;
            result.Z = blue;

            return result;
        }

        void WriteColor(Nbt::TagCompound& base, Common::Vector3S color) {
            base.data.insert( { "R", { Nbt::TagShort { color.X} }});
            base.data.insert( { "G", { Nbt::TagShort { color.Y} }});
            base.data.insert( { "B", { Nbt::TagShort { color.Z} }});
        }

        Nbt::TagCompound Write() override {
            Nbt::TagCompound cpeBase;
            cpeBase.name = "CPE";

            if (ClickDistanceVersion > 0) {
                Nbt::TagCompound cdBase;
                cdBase.name = "ClickDistance";
                cdBase.data.insert({"ExtensionVersion", {Nbt::TagInt {ClickDistanceVersion} }});
                cdBase.data.insert({"Distance", { Nbt::TagShort { ClickDistance} }});

                cpeBase.data.insert({"ClickDistance", { cdBase }});
            }
            if (CustomBlocksVersion > 0) {
                Nbt::TagCompound eBase;
                eBase.name = "CustomBlocks";
                eBase.data.insert({"ExtensionVersion", {Nbt::TagInt {CustomBlocksVersion} }});
                eBase.data.insert({"SupportLevel", { Nbt::TagShort{CustomBlocksLevel} } });

                cpeBase.data.insert({ "CustomBlocks", { eBase } });
            }
            if (EnvColorsVersion > 0) {
                Nbt::TagCompound eBase;
                eBase.name = "EnvColors";
                eBase.data.insert({"ExtensionVersion", {Nbt::TagInt {EnvColorsVersion} }});
                Nbt::TagCompound skyColor;
                WriteColor(skyColor, SkyColor);
                eBase.data.insert({"Sky", { skyColor }});

                Nbt::TagCompound cloudColor;
                WriteColor(cloudColor, CloudColor);
                eBase.data.insert({"Cloud", { cloudColor }});

                Nbt::TagCompound fogColor;
                WriteColor(fogColor, FogColor);
                eBase.data.insert({"Fog", { fogColor }});

                Nbt::TagCompound ambientColor;
                WriteColor(ambientColor, AmbientColor);
                eBase.data.insert({"Ambient", { ambientColor }});

                Nbt::TagCompound sunColor;
                WriteColor(sunColor, SunlightColor);
                eBase.data.insert({"Sunlight", { sunColor }});

                cpeBase.data.insert( { "EnvColors", { eBase }});
            }

            if (EnvMapAppearanceVersion > 0) {
                Nbt::TagCompound eBase;
                eBase.name = "EnvMapAppearance";
                eBase.data.insert({"ExtensionVersion", {Nbt::TagInt {EnvMapAppearanceVersion} }});
                eBase.data.insert({"TextureURL", {Nbt::TagString {TextureUrl} }});
                eBase.data.insert({"SideBlock", {Nbt::TagByte {static_cast<char>(SideBlock)} }});
                eBase.data.insert({"EdgeBlock", {Nbt::TagByte {static_cast<char>(EdgeBlock)} }});
                eBase.data.insert({"SideLevel", {Nbt::TagShort {SideLevel} }});

                cpeBase.data.insert( { "EnvMapAppearance", { eBase }});
            }

            if (HackControlVersion > 0) {
                Nbt::TagCompound eBase;
                eBase.name = "HackControl";
                eBase.data.insert({"ExtensionVersion", {Nbt::TagInt {HackControlVersion} }});
                eBase.data.insert({"CanFly", {Nbt::TagByte {static_cast<char>(CanFly) }}});
                eBase.data.insert({"CanClip", {Nbt::TagByte {static_cast<char>(CanClip) }}});
                eBase.data.insert({"CanSpeed", {Nbt::TagByte {static_cast<char>(CanSpeed) }}});
                eBase.data.insert({"CanRespawn", {Nbt::TagByte {static_cast<char>(CanRespawn) }}});
                eBase.data.insert({"CanThirdPerson", {Nbt::TagByte {static_cast<char>(CanThirdPerson) }}});
                eBase.data.insert({"CanSetWeather", {Nbt::TagByte {static_cast<char>(CanSetWeather) }}});
                eBase.data.insert({"JumpHeight", {Nbt::TagShort {JumpHeight }}});

                cpeBase.data.insert( { "HackControl", { eBase }});
            }

            if (EnvMapAspectVersion > 0) {
                Nbt::TagCompound eBase;
                eBase.name = "EnvMapAspect";
                eBase.data.insert({"ExtensionVersion", {Nbt::TagInt {EnvMapAspectVersion} }});

                cpeBase.data.insert( { "EnvMapAspect", { eBase }});
            }

            return cpeBase;
        }
    };

    class ClassicWorld {
    public:
        unsigned char FormatVersion;
        std::string MapName, CreatingService, CreatingUsername, GeneratingSoftware, GeneratorName;
        std::vector<signed char> Uuid;
        Common::Vector3S Size;
        Common::Vector3S Spawn;
        long TimeCreated, LastAccessed, LastModified;
        unsigned char SpawnRotation, SpawnLook;
        std::vector<unsigned char> BlockData;
        ForeignMeta foreignMeta;
        std::map<std::string, std::shared_ptr<IMetadataStructure>> metaParsers;

        // -- Create a new map
        explicit ClassicWorld(Common::Vector3S size);
        // -- Load an existing file
        explicit ClassicWorld(std::string filepath);

        void Load();
        void Save(std::string filepath);
    private:
        Nbt::TagCompound m_baseTag;
        std::string m_filePath;
    };
}
#endif //D3PP_CLASSICWORLD_H
