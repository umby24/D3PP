//
// Created by Wande on 7/25/2022.
//
#include "Utils.h"
#include <files/ClassicWorld.h>


D3PP::files::ClassicWorld::ClassicWorld(Common::Vector3S size) {
    BlockData.resize(size.X * size.Y * size.Z);
    Size = size;
    SpawnLook = 0;
    SpawnRotation = 0;
    Spawn = D3PP::Common::Vector3S((short)(Size.X/2), Size.Y/2, Size.Z/2);
    FormatVersion = 1;
    TimeCreated = Utils::CurrentUnixTime();
    LastAccessed = Utils::CurrentUnixTime();
    LastModified = Utils::CurrentUnixTime();

    metaParsers.insert(std::make_pair("CPE", std::make_unique<CPEMetadata>()));
}

D3PP::files::ClassicWorld::ClassicWorld(std::string filepath) {
    auto baseTag = Nbt::NbtFile::Load(filepath);
    m_baseTag = std::get<Nbt::TagCompound>(baseTag);

    if (m_baseTag.name != "ClassicWorld") {
        throw std::runtime_error("Not a valid ClassicWorld file; Basetag name is not 'classicworld'.");
    }

    metaParsers.insert(std::make_pair("CPE", std::make_unique<CPEMetadata>()));
}

void D3PP::files::ClassicWorld::Load() {
    BlockData.clear();
    if (std::holds_alternative<Nbt::TagByte>(m_baseTag["FormatVersion"])) {
        FormatVersion = std::get<Nbt::TagByte>(m_baseTag["FormatVersion"]);
    } else {
        throw std::runtime_error("Not a valid ClassicWorld file; FormatVersion is not a byte.");
    }

    if (FormatVersion != 1) {
        throw std::runtime_error("Unsupported format version.");
    }

    if (m_baseTag.data.contains("Name")) {
        if (std::holds_alternative<Nbt::TagString>(m_baseTag["Name"])) {
            MapName = std::get<Nbt::TagString>(m_baseTag["Name"]);
        }
    }
    if (MapName.empty()) {
        MapName = "ClassicWorldMap";
    }
    Uuid = std::get<Nbt::TagByteArray>(m_baseTag["UUID"]);
    Size = Common::Vector3S {
        std::get<Nbt::TagShort>(m_baseTag["X"]),
        std::get<Nbt::TagShort>(m_baseTag["Y"]),
        std::get<Nbt::TagShort>(m_baseTag["Z"])
    };

    if (m_baseTag.data.contains("CreatedBy")) {
        auto createdBase = std::get<Nbt::TagCompound>(m_baseTag["CreatedBy"]);
        CreatingService = std::get<Nbt::TagString>(createdBase["Service"]);
        CreatingUsername = std::get<Nbt::TagString>(createdBase["Username"]);
    }

    if (m_baseTag.data.contains("MapGenerator")) {
        auto genBase = std::get<Nbt::TagCompound>(m_baseTag["MapGenerator"]);
        GeneratingSoftware = std::get<Nbt::TagString>(genBase["Software"]);
        GeneratorName = std::get<Nbt::TagString>(genBase["MapGeneratorName"]);
    }

    if (m_baseTag.data.contains("TimeCreated"))
        TimeCreated = std::get<Nbt::TagLong>(m_baseTag["TimeCreated"]);
    else
        TimeCreated = Utils::CurrentUnixTime();

    LastAccessed = Utils::CurrentUnixTime();

    if (m_baseTag.data.contains("LastModified"))
        LastModified = std::get<Nbt::TagLong>(m_baseTag["LastModified"]);
    else
        LastModified = Utils::CurrentUnixTime();

    if (!m_baseTag.data.contains("Spawn"))
        throw std::runtime_error("Map is missing spawn compound.");

    auto spawnBase = std::get<Nbt::TagCompound>(m_baseTag["Spawn"]);
    Spawn = Common::Vector3S {
        std::get<Nbt::TagShort>(spawnBase["X"]),
        std::get<Nbt::TagShort>(spawnBase["Y"]),
        std::get<Nbt::TagShort>(spawnBase["Z"])
    };
    SpawnRotation = (unsigned char)std::get<Nbt::TagByte>(spawnBase["H"]);
    SpawnLook = (unsigned char)std::get<Nbt::TagByte>(spawnBase["P"]);

    for (auto q : std::get<Nbt::TagByteArray>(m_baseTag["BlockArray"])) {
        BlockData.push_back(static_cast<unsigned char>(q));
    }

    if (m_baseTag.data.contains("Metadata")) {
        auto metaBase = std::get<Nbt::TagCompound>(m_baseTag["Metadata"]);

        for(const auto& parser : metaParsers) {
            metaBase = parser.second->Read(metaBase);
        }

        foreignMeta.Read(metaBase);
    }

    m_baseTag.data.clear(); // -- Clear mem. Load complete.
}

void D3PP::files::ClassicWorld::Save(std::string filepath) {
    auto nbtMetadata = foreignMeta.Write();

    for (auto &p : metaParsers) {
        Nbt::TagCompound nbtVal = p.second->Write();
        nbtMetadata.data.insert({nbtVal.name, {nbtVal}});
    }

    Nbt::TagCompound baseCompound;
    baseCompound.name = "ClassicWorld";
    baseCompound.data.insert({"FormatVersion", { Nbt::TagByte {1} }});
    baseCompound.data.insert({"Name", { Nbt::TagString {MapName} }});
    baseCompound.data.insert({"UUID", { Nbt::TagByteArray(Uuid) }});
    baseCompound.data.insert({"X", { Nbt::TagShort {Size.X} }});
    baseCompound.data.insert({"Y", { Nbt::TagShort {Size.Y} }});
    baseCompound.data.insert({"Z", { Nbt::TagShort {Size.Z} }});

    Nbt::TagCompound spawnCompound;
    spawnCompound.name = "Spawn";
    spawnCompound.data.insert({"X", { Nbt::TagShort {Spawn.X} }});
    spawnCompound.data.insert({"Y", { Nbt::TagShort {Spawn.Y} }});
    spawnCompound.data.insert({"Z", { Nbt::TagShort {Spawn.Z} }});
    spawnCompound.data.insert({"H", { Nbt::TagByte {(char)SpawnRotation} }});
    spawnCompound.data.insert({"P", { Nbt::TagByte {(char)SpawnLook} }});

    baseCompound.data.insert({"Spawn", {spawnCompound}});
    std::vector<signed char> tempArr(BlockData.begin(), BlockData.end());

    baseCompound.data.insert({"BlockArray", { Nbt::TagByteArray(tempArr) }});
    baseCompound.data.insert({"Metadata", {nbtMetadata}});

    if (!CreatingService.empty() && !CreatingUsername.empty()) {
        Nbt::TagCompound creator;
        creator.name = "CreatedBy";
        creator.data.insert({"Service", { Nbt::TagString {CreatingService} }});
        creator.data.insert({"Username", { Nbt::TagString {CreatingUsername} }});
        baseCompound.data.insert({"CreatedBy", {creator}});
    }

    if (!GeneratingSoftware.empty() && !GeneratorName.empty()) {
        Nbt::TagCompound creator;
        creator.name = "MapGenerator";
        creator.data.insert({"Software", { Nbt::TagString {GeneratingSoftware} }});
        creator.data.insert({"MapGeneratorName", { Nbt::TagString {GeneratorName} }});
        baseCompound.data.insert({"MapGenerator", {creator}});
    }

    baseCompound.data.insert({"TimeCreated", { Nbt::TagLong {TimeCreated} }});
    baseCompound.data.insert({"LastAccessed", { Nbt::TagLong {LastAccessed} }});
    baseCompound.data.insert({"LastModified", { Nbt::TagLong {LastModified} }});

    Nbt::NbtFile::Save(baseCompound, filepath, Nbt::CompressionMode::GZip);
}

