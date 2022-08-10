//
// Created by Wande on 6/15/2022.
//

#ifndef D3PP_CPPNBT_H
#define D3PP_CPPNBT_H





#include <string>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <vector>
#include <variant>
#include "compression.h"
#include "Utils.h"

namespace Nbt {
    enum CompressionMode {
        NONE = 0,
        DETECT = 1,
        GZip = 2,
        ZLib = 3
    };

    enum TagType {
        TAG_END,
        TAG_BYTE,
        TAG_SHORT,
        TAG_INT,
        TAG_LONG,
        TAG_FLOAT,
        TAG_DOUBLE,
        TAG_BYTE_ARRAY,
        TAG_STRING,
        TAG_LIST,
        TAG_COMPOUND,
        TAG_INT_ARRAY,
        TAG_LONG_ARRAY
    };

    typedef std::nullptr_t TagEnd;


    typedef std::int8_t TagByte;
    typedef std::int16_t TagShort;
    typedef std::int32_t TagInt;
    typedef std::int64_t TagLong;


    typedef float TagFloat;
    typedef double TagDouble;


    typedef std::vector<TagByte> TagByteArray;
    typedef std::vector<TagInt> TagIntArray;
    typedef std::vector<TagLong> TagLongArray;


    typedef std::string TagString;


    struct TagList;
    struct TagCompound;


    typedef std::variant<TagEnd, TagByte, TagShort, TagInt, TagLong, TagFloat,
            TagDouble, TagByteArray, TagString, TagList, TagCompound, TagIntArray,
            TagLongArray>
            Tag;

    struct TagList {
        int size;

        std::variant<std::vector<TagEnd>, std::vector<TagByte>,
                std::vector<TagShort>, std::vector<TagInt>, std::vector<TagLong>,
                std::vector<TagFloat>, std::vector<TagDouble>, std::vector<TagByteArray>,
                std::vector<TagString>, std::vector<TagList>, std::vector<TagCompound>,
                std::vector<TagIntArray>, std::vector<TagLongArray>>
                base;
    };

    struct TagCompound {
        std::string name;
        std::map<std::string, Tag> data;

        Tag& operator[](const std::string& key) {
            return data[key];
        }
        Tag& operator[](const char* key) {
            return data[key];
        }

        Tag& at(const std::string& key) {
            return data.at(key);
        }
        const Tag& at(const std::string& key) const {
            return data.at(key);
        }

        template <typename T> T& at(const std::string& key) {
            return std::get<T>(data.at(key));
        }
    };

    class NbtFile {
    public:
        static Tag Load(const std::string &file, CompressionMode compression = CompressionMode::DETECT) {
            if (!std::filesystem::exists(file)) {
                throw std::invalid_argument("Invalid argument 'file', file does not exist.");
            }

            std::ifstream is(file, std::ios::binary);

            if (!is.is_open()) {
                throw std::runtime_error("Failed to open NBT file");
            }
            int fileSize = Utils::FileSize(file);

            std::vector<unsigned char> data;
            data.resize(fileSize);
            data.assign(std::istreambuf_iterator<char>(is),
                        std::istreambuf_iterator<char>());
            is.close();

            if (compression == CompressionMode::DETECT) {
                compression = DetectCompression(data);
            }

            if (compression == CompressionMode::ZLib || compression == CompressionMode::GZip) {
                data.clear();
                data.resize(67108864);
                int decompSize = GZIP::GZip_DecompressFromFile(reinterpret_cast<unsigned char*>(data.data()), 67108864, file);
                data.resize(decompSize);
            }

            Tag result = Decode(data);
            return result;
        }

        static bool Save(Tag t, const std::string &filename, CompressionMode compression = CompressionMode::ZLib) {
            if (!std::holds_alternative<TagCompound>(t)) {
                throw std::runtime_error("TAG_COMPOUND is not the base");
            }
            TagCompound baseTag = std::get<TagCompound>(t);
            std::vector<unsigned char> encoded = Encode(baseTag);

            if (compression == CompressionMode::ZLib || compression == CompressionMode::GZip) {
                GZIP::GZip_CompressToFile(encoded.data(), encoded.size(), filename);
            } else {
                std::ofstream is(filename, std::ios::binary | std::ios::trunc);
                is.write((char *) &encoded[0], encoded.size());
                is.close();
            }

            return true;
        }

        static std::string Serialize(Tag t, std::string name) {
            if (std::holds_alternative<TagByte>(t))
                return Serialize(std::get<TagByte>(t), name);
            if (std::holds_alternative<TagShort>(t))
                return Serialize(std::get<TagShort>(t), name);
            if (std::holds_alternative<TagInt>(t))
                return Serialize(std::get<TagInt>(t), name);
            if (std::holds_alternative<TagLong>(t))
                return Serialize(std::get<TagLong>(t), name);
            if (std::holds_alternative<TagFloat>(t))
                return Serialize(std::get<TagFloat>(t), name);
            if (std::holds_alternative<TagDouble>(t))
                return Serialize(std::get<TagDouble>(t), name);
            if (std::holds_alternative<TagString>(t))
                return Serialize(std::get<TagString>(t), name);
            if (std::holds_alternative<TagByteArray>(t))
                return Serialize(std::get<TagByteArray>(t), name);
//            if (std::holds_alternative<TagList>(t))
//                return Serialize(std::get<TagList>(t), name);
//            if (std::holds_alternative<TagIntArray>(t))
//                return Serialize(std::get<TagIntArray>(t), name);
//            if (std::holds_alternative<TagLongArray>(t))
//                return Serialize(std::get<TagLongArray>(t), name);
            if (std::holds_alternative<TagCompound>(t))
                return Serialize(std::get<TagCompound>(t), name);

            return "";
        }
        static std::string Serialize(TagByte t, std::string name) {
            std::stringstream output;
            output << "TAG_Byte('";
            output << name;
            output << "'): ";
            output << (int)t << std::endl;
            return output.str();
        }
        static std::string Serialize(TagShort t, std::string name) {
            std::stringstream output;
            output << "TAG_Short('";
            output << name;
            output << "'): ";
            output << (int)t << std::endl;
            return output.str();
        }
        static std::string Serialize(TagInt t, std::string name) {
            std::stringstream output;
            output << "TAG_Int('";
            output << name;
            output << "'): ";
            output << t << std::endl;
            return output.str();
        }
        static std::string Serialize(TagLong t, std::string name) {
            std::stringstream output;
            output << "TAG_Long('";
            output << name;
            output << "'): ";
            output << t << std::endl;
            return output.str();
        }
        static std::string Serialize(TagFloat t, std::string name) {
            std::stringstream output;
            output << std::setprecision(20);
            output << "TAG_Float('";
            output << name;
            output << "'): ";
            output << t << std::endl;
            return output.str();
        }
        static std::string Serialize(TagDouble t, std::string name) {
            std::stringstream output;
            output << std::setprecision(20);
            output << "TAG_Double('";
            output << name;
            output << "'): ";
            output << t << std::endl;
            return output.str();
        }

        static std::string Serialize(TagString t, std::string name) {
            std::stringstream output;
            output << "TAG_String('";
            output << name;
            output << "'): ";
            output << t << std::endl;
            return output.str();
        }
        static std::string Serialize(TagByteArray t, std::string name) {
            std::stringstream output;
            output << "TAG_Byte_Array('";
            output << name;
            output << "'): Elements: ";
            output << t.size() << std::endl;
            return output.str();
        }
        static std::string Serialize(TagCompound t, std::string name) {
            std::stringstream output;
            output << "TAG_Compound(";
            output << "'" << name << "'";
            output << "): Elements: ";
            output << t.data.size();
            output << " {" << std::endl;

            for(auto const &p: t.data) {
                std::string sOut = Serialize(p.second, p.first);
                Utils::replaceAll(sOut, "\n", "\n\t");
                if (!sOut.empty()) {
                    if (sOut.at(sOut.size() - 1) == '\t')
                        sOut = sOut.substr(0, sOut.size() - 1);
                }

                output << "\t" << sOut;
            }

            output << "}" << std::endl;
            return output.str();
        }
        static std::string Serialize(TagCompound t) {
            std::stringstream output;
            output << "TAG_Compound(";
            t.name == "" ? output << "None" : output << "'" << t.name << "'";
            output << "): Elements: ";
            output << t.data.size();
            output << " {" << std::endl;
            for(auto const &p: t.data) {
                std::string sOut = Serialize(p.second, p.first);
                Utils::replaceAll(sOut, "\n", "\n\t");
                if (!sOut.empty()) {
                    if (sOut.at(sOut.size() - 1) == '\t')
                        sOut = sOut.substr(0, sOut.size() - 1);
                }

                output << "\t" << sOut;
            }
            output << "}";
            return output.str();
        }

    protected:
        static Tag Decode(std::vector<unsigned char> data) {
            if (data.at(0) != TAG_COMPOUND) {
                throw std::runtime_error("TAG_COMPOUND is not the base");
            }

            TagCompound base;
            int offset = 1;
            base.name = ReadString(data, offset);
            base.data = ReadCompound(data, offset).data;

            return base;
        }

        static std::vector<unsigned char> Encode(TagCompound base) {
            std::vector<unsigned char> result;
            result.push_back(static_cast<unsigned char>(TAG_COMPOUND));
            WriteString(base.name, result);
            WriteCompound(base, result);
            return result;
        }

        static TagCompound ReadCompound(std::vector<unsigned char> data, int& offset) {
            TagType nextType = TAG_END;
            TagCompound baseTag;
            std::string nextName = "";
            do {
                nextType = static_cast<TagType>(data.at(offset++));
                if (nextType != TAG_END) {
                    nextName = ReadString(data, offset);
                    Tag nextTag = ReadOnType(data, offset, nextType);
                    baseTag.data.insert(std::make_pair(nextName, nextTag));
                }
            } while (nextType != TAG_END);

            return baseTag;
        }
        static void WriteCompound(TagCompound tag, std::vector<unsigned char>& data) {
            for(const auto &t : tag.data) {
                WriteTagType(t.second, data);
                WriteString(t.first, data);
                WriteOnType(t.second, data);
            }

            data.push_back(static_cast<unsigned char>(TAG_END));
        }
        static void WriteTagType(const Tag& t, std::vector<unsigned char>& data) {
            if (std::holds_alternative<TagByte>(t))
                data.push_back(static_cast<unsigned char>(TAG_BYTE));
            if (std::holds_alternative<TagShort>(t))
                data.push_back(static_cast<unsigned char>(TAG_SHORT));
            if (std::holds_alternative<TagInt>(t))
                data.push_back(static_cast<unsigned char>(TAG_INT));
            if (std::holds_alternative<TagLong>(t))
               data.push_back(static_cast<unsigned char>(TAG_LONG));
            if (std::holds_alternative<TagFloat>(t))
                data.push_back(static_cast<unsigned char>(TAG_FLOAT));
            if (std::holds_alternative<TagDouble>(t))
                data.push_back(static_cast<unsigned char>(TAG_DOUBLE));
            if (std::holds_alternative<TagString>(t))
                data.push_back(static_cast<unsigned char>(TAG_STRING));
            if (std::holds_alternative<TagList>(t))
                data.push_back(static_cast<unsigned char>(TAG_LIST));
            if (std::holds_alternative<TagByteArray>(t))
                data.push_back(static_cast<unsigned char>(TAG_BYTE_ARRAY));
            if (std::holds_alternative<TagIntArray>(t))
                data.push_back(static_cast<unsigned char>(TAG_INT_ARRAY));
            if (std::holds_alternative<TagLongArray>(t))
                data.push_back(static_cast<unsigned char>(TAG_LONG_ARRAY));
            if (std::holds_alternative<TagCompound>(t))
                data.push_back(static_cast<unsigned char>(TAG_COMPOUND));
        }

        static Tag ReadOnType(std::vector<unsigned char> data, int& offset, TagType nextType) {
            Tag nextTag;
            switch (nextType) {
                case TAG_BYTE: {
                    auto val = ReadByte(data, offset);
                    nextTag = val;
                    break;
                    }
                case TAG_SHORT: {
                    auto val2 = ReadShort(data, offset);
                    nextTag = val2;
                    break;
                }
                case TAG_INT: {
                    auto val3 = ReadInt(data, offset);
                    nextTag = val3;
                    break;
                }
                case TAG_LONG: {
                    auto val4 = ReadLong(data, offset);
                    nextTag = val4;
                    break;
                }
                case TAG_FLOAT: {
                    auto va5l = ReadFloat(data, offset);
                    nextTag = va5l;
                    break;
                }
                case TAG_DOUBLE: {
                    auto va6l = ReadDouble(data, offset);
                    nextTag = va6l;
                    break;
                }
                case TAG_BYTE_ARRAY: {
                    auto val7 = ReadByteArray(data, offset);
                    nextTag = val7;
                    break;
                }
                case TAG_STRING: {
                    TagString val8 = ReadString(data, offset);
                    nextTag = val8;
                    break;
                }
                case TAG_LIST: {
                    TagList val9 = ReadList(data, offset);
                    nextTag = val9;
                    break;
                }
                case TAG_COMPOUND: {
                    TagCompound val99 = ReadCompound(data, offset);
                    nextTag = val99;
                    break;
                }
                case TAG_INT_ARRAY: {
                    TagIntArray val88 = ReadIntArray(data, offset);
                    nextTag = val88;
                    break;
                }
                case TAG_LONG_ARRAY: {
                    TagLongArray val77 = ReadLongArray(data, offset);
                    nextTag = val77;
                    break;
                }
            }

            return nextTag;
        }
        static void WriteOnType(Tag t, std::vector<unsigned char>& data) {
            if (std::holds_alternative<TagByte>(t))
                WriteByte(std::get<TagByte>(t), data);
            if (std::holds_alternative<TagShort>(t))
                WriteShort(std::get<TagShort>(t), data);
            if (std::holds_alternative<TagInt>(t))
                WriteInt(std::get<TagInt>(t), data);
            if (std::holds_alternative<TagLong>(t))
                WriteLong(std::get<TagLong>(t), data);
            if (std::holds_alternative<TagFloat>(t))
                WriteFloat(std::get<TagFloat>(t), data);
            if (std::holds_alternative<TagDouble>(t))
                WriteDouble(std::get<TagDouble>(t), data);
            if (std::holds_alternative<TagString>(t))
                WriteString(std::get<TagString>(t), data);
            if (std::holds_alternative<TagList>(t))
                WriteList(std::get<TagList>(t), data);
            if (std::holds_alternative<TagByteArray>(t))
                WriteByteArray(std::get<TagByteArray>(t), data);
            if (std::holds_alternative<TagIntArray>(t))
                WriteIntArray(std::get<TagIntArray>(t), data);
            if (std::holds_alternative<TagLongArray>(t))
                WriteLongArray(std::get<TagLongArray>(t), data);
            if (std::holds_alternative<TagCompound>(t))
                WriteCompound(std::get<TagCompound>(t), data);
        }
        static TagList ReadList(std::vector<unsigned char> data, int& offset) {
            auto listType = static_cast<TagType>(data.at(offset++));
            TagInt listLength = ReadInt(data, offset);
            TagList result;
            if (listLength <= 0)
                return result;

            for(auto i = 0; i < listLength; i++) {
            switch (listType) {
                case TAG_BYTE: {
                    std::vector<TagByte> vec;
                    vec.push_back(std::get<TagByte>(ReadOnType(data, offset, listType)));
                    result.base = vec;
                    break;
                }
                case TAG_SHORT:{
                    std::vector<TagShort> vec;
                    vec.push_back(std::get<TagShort>(ReadOnType(data, offset, listType)));
                    result.base = vec;
                    break;
                }
                case TAG_INT:{
                    std::vector<TagInt> vec;
                    vec.push_back(std::get<TagInt>(ReadOnType(data, offset, listType)));
                    result.base = vec;
                    break;
                }
                case TAG_LONG:{
                    std::vector<TagLong> vec;
                    vec.push_back(std::get<TagLong>(ReadOnType(data, offset, listType)));
                    result.base = vec;
                    break;
                }
                case TAG_FLOAT:{
                    std::vector<TagFloat> vec;
                    vec.push_back(std::get<TagFloat>(ReadOnType(data, offset, listType)));
                    result.base = vec;
                    break;
                }
                case TAG_DOUBLE:{
                    std::vector<TagDouble> vec;
                    vec.push_back(std::get<TagDouble>(ReadOnType(data, offset, listType)));
                    result.base = vec;
                    break;
                }
                case TAG_BYTE_ARRAY:{
                    std::vector<TagByteArray> vec;
                    vec.push_back(std::get<TagByteArray>(ReadOnType(data, offset, listType)));
                    result.base = vec;
                    break;
                }
                case TAG_STRING:{
                    std::vector<TagString> vec;
                    vec.push_back(std::get<TagString>(ReadOnType(data, offset, listType)));
                    result.base = vec;
                    break;
                }
                case TAG_LIST:{
                    std::vector<TagList> vec;
                    vec.push_back(std::get<TagList>(ReadOnType(data, offset, listType)));
                    result.base = vec;
                    break;
                }
                case TAG_COMPOUND:{
                    std::vector<TagCompound> vec;
                    vec.push_back(std::get<TagCompound>(ReadOnType(data, offset, listType)));
                    result.base = vec;
                    break;
                }
                case TAG_INT_ARRAY:{
                    std::vector<TagIntArray> vec;
                    vec.push_back(std::get<TagIntArray>(ReadOnType(data, offset, listType)));
                    result.base = vec;
                    break;
                }
                case TAG_LONG_ARRAY:{
                    std::vector<TagLongArray> vec;
                    vec.push_back(std::get<TagLongArray>(ReadOnType(data, offset, listType)));
                    result.base = vec;
                    break;
                }
            }


            }

            return result;
        }
        static void WriteList(TagList tag, std::vector<unsigned char>& data) {
            throw std::runtime_error("Not implementing at this time :)");
//            std::get(tag.base)
//            WriteTagType(tag.base, data);
//            WriteInt(tag.size, data);

        }
        static TagByte ReadByte(std::vector<unsigned char> data, int& offset) {
            return data.at(offset++);
        }
        static void WriteByte(TagByte tag, std::vector<unsigned char>& data) {
            data.push_back(static_cast<unsigned char>(tag));
        }
        static TagShort ReadShort(std::vector<unsigned char> data, int& offset) {
            short val = 0;
            val |= data.at(offset++) << 8;
            val |= data.at(offset++);
            return val;
        }
        static void WriteShort(TagShort tag, std::vector<unsigned char>& data) {
            data.push_back(static_cast<unsigned char>(tag >> 8));
            data.push_back(static_cast<unsigned char>(tag));
        }
        static TagInt ReadInt(std::vector<unsigned char> data, int& offset) {
            int result = 0;
            result |= data.at(offset++) << 24;
            result |= data.at(offset++) << 16;
            result |= data.at(offset++) << 8;
            result |= data.at(offset++);
            return result;
        }
        static void WriteInt(TagInt tag, std::vector<unsigned char>& data) {
            data.push_back(static_cast<unsigned char>(tag >> 24));
            data.push_back(static_cast<unsigned char>(tag >> 16));
            data.push_back(static_cast<unsigned char>(tag >> 8));
            data.push_back(static_cast<unsigned char>(tag));
        }
        static TagLong ReadLong(std::vector<unsigned char> data, int& offset) {
            TagLong result = 0;
            char* resultD = (char*)&result;
            resultD[7] = data.at(offset++);
            resultD[6] = data.at(offset++);
            resultD[5] = data.at(offset++);
            resultD[4] = data.at(offset++);
            resultD[3] = data.at(offset++);
            resultD[2] = data.at(offset++);
            resultD[1] = data.at(offset++);
            resultD[0] = data.at(offset++);
            return result;
        }
        static void WriteLong(TagLong tag, std::vector<unsigned char>& data) {
            data.push_back(static_cast<unsigned char>(tag >> 56));
            data.push_back(static_cast<unsigned char>(tag >> 48));
            data.push_back(static_cast<unsigned char>(tag >> 40));
            data.push_back(static_cast<unsigned char>(tag >> 32));
            data.push_back(static_cast<unsigned char>(tag >> 24));
            data.push_back(static_cast<unsigned char>(tag >> 16));
            data.push_back(static_cast<unsigned char>(tag >> 8));
            data.push_back(static_cast<unsigned char>(tag));
        }
        static TagFloat ReadFloat(std::vector<unsigned char> data, int& offset) {
            TagFloat result = 0;
            char* resultD = (char*)&result;
            resultD[3] = data.at(offset++);
            resultD[2] = data.at(offset++);
            resultD[1] = data.at(offset++);
            resultD[0] = data.at(offset++);

            return result;
        }

        static void WriteFloat(TagFloat tag, std::vector<unsigned char>& data) {
            char* tagD = (char*)(&tag);
            data.push_back(static_cast<unsigned char>(tagD[3]));
            data.push_back(static_cast<unsigned char>(tagD[2]));
            data.push_back(static_cast<unsigned char>(tagD[1]));
            data.push_back(static_cast<unsigned char>(tagD[0]));
        }
        static TagDouble ReadDouble(std::vector<unsigned char> data, int& offset) {
            TagDouble result = 0;
            char* resultD = (char*)&result;
            resultD[7] = data.at(offset++);
            resultD[6] = data.at(offset++);
            resultD[5] = data.at(offset++);
            resultD[4] = data.at(offset++);
            resultD[3] = data.at(offset++);
            resultD[2] = data.at(offset++);
            resultD[1] = data.at(offset++);
            resultD[0] = data.at(offset++);
            return result;
        }
        static void WriteDouble(TagDouble tag, std::vector<unsigned char>& data) {
            char* tagD = (char*)(&tag);

            data.push_back(static_cast<unsigned char>(tagD[7]));
            data.push_back(static_cast<unsigned char>(tagD[6]));
            data.push_back(static_cast<unsigned char>(tagD[5]));
            data.push_back(static_cast<unsigned char>(tagD[4]));
            data.push_back(static_cast<unsigned char>(tagD[3]));
            data.push_back(static_cast<unsigned char>(tagD[2]));
            data.push_back(static_cast<unsigned char>(tagD[1]));
            data.push_back(static_cast<unsigned char>(tagD[0]));
        }
        static TagByteArray ReadByteArray(std::vector<unsigned char> data, int& offset) {
            TagInt arraySize = ReadInt(data, offset);
            TagByteArray result;
            result.resize(arraySize);
            for (auto& el : result) {
                el = data.at(offset++);
            }
            return result;
        }
        static void WriteByteArray(TagByteArray tag, std::vector<unsigned char>& data) {
            auto longSize = static_cast<TagInt>(tag.size());
            WriteInt(longSize, data);
            for(const auto &i : tag) {
                WriteByte(i, data);
            }
        }
        static TagString ReadString(std::vector<unsigned char> data, int& offset) {
            short strLen = 0;
            strLen |= data.at(offset++) << 8;
            strLen |= data.at(offset++);
            std::string str(data.begin() + offset, data.begin()+offset+strLen);
            offset += strLen;
            return str;
        }
        static void WriteString(TagString tag, std::vector<unsigned char>& data) {
            short strLen = tag.size();
            data.push_back(static_cast<unsigned char>(strLen >> 8));
            data.push_back(static_cast<unsigned char>(strLen));
            for(auto i = 0; i < strLen; i++) {
                data.push_back(tag.at(i));
            }
        }
        static TagIntArray ReadIntArray(std::vector<unsigned char> data, int& offset) {
            TagInt arraySize = ReadInt(data, offset);
            TagIntArray result;
            for(int i = 0; i < arraySize; i++) {
                result.push_back(ReadInt(data, offset));
            }
            return result;
        }
        static void WriteIntArray(TagIntArray tag, std::vector<unsigned char>& data) {
            auto longSize = static_cast<TagInt>(tag.size());
            WriteInt(longSize, data);
            for(const auto &i : tag) {
                WriteInt(i, data);
            }
        }
        static TagLongArray ReadLongArray(std::vector<unsigned char> data, int& offset) {
            TagInt arraySize = ReadInt(data, offset);
            TagLongArray result;
            for(int i = 0; i < arraySize; i++) {
                result.push_back(ReadLong(data, offset));
            }
            return result;
        }
        static void WriteLongArray(TagLongArray tag, std::vector<unsigned char>& data) {
            auto longSize = static_cast<TagInt>(tag.size());
            WriteInt(longSize, data);
            for(const auto &i : tag) {
                WriteLong(i, data);
            }
        }
        static CompressionMode DetectCompression(std::vector<unsigned char> buf) {
            if (buf.size() < 2) {
                throw std::runtime_error("File is too small to determine compression");
            }

            if (buf.at(0) == 0x1f && buf.at(1) == 0x8B)
                return CompressionMode::ZLib;

            return CompressionMode::NONE;
        }
    };
}
#endif //D3PP_CPPNBT_H
