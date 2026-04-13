#pragma once
#ifndef __C_P_SERIALIZATION_FACADE_H__
#define __C_P_SERIALIZATION_FACADE_H__

#include "ControlProtocol/Serialization/Core/SerializeValue.h"
#include "ControlProtocol/Serialization/Format/Binary/BinaryInputArchive.h"
#include "ControlProtocol/Serialization/Format/Binary/BinaryOutputArchive.h"
#include "ControlProtocol/Serialization/Format/Json/JsonInputArchive.h"
#include "ControlProtocol/Serialization/Format/Json/JsonOutputArchive.h"
#include <fstream>
#include <string>

/// <summary>
/// 该模块主要功能是为了序列化和反序列化
/// </summary>
namespace ControlProtocol::Serialization
{
    template<typename T>
    bool SaveToBinaryFile(const std::string& path, T& object)
    {
        std::ofstream stream(path, std::ios::binary);
        if (!stream.is_open())
        {
            return false;
        }

        BinaryOutputArchive archive(stream);
        return SerializeObject(archive, object);
    }

    template<typename T>
    bool LoadFromBinaryFile(const std::string& path, T& object)
    {
        std::ifstream stream(path, std::ios::binary);
        if (!stream.is_open())
        {
            return false;
        }

        BinaryInputArchive archive(stream);
        return DeserializeObject(archive, object);
    }

    template<typename T>
    bool SaveToJsonFile(const std::string& path, T& object)
    {
        std::ofstream stream(path);
        if (!stream.is_open())
        {
            return false;
        }

        JsonOutputArchive archive(stream);
        return SerializeObject(archive, object);
    }

    template<typename T>
    bool LoadFromJsonFile(const std::string& path, T& object)
    {
        std::ifstream stream(path);
        if (!stream.is_open())
        {
            return false;
        }

        JsonInputArchive archive(stream);
        return DeserializeObject(archive, object);
    }
}

#endif
