#pragma once

namespace ControlProtocol::Serialization
{
    template<typename T>
    bool SerializeValue(IOutputArchive& archive, std::vector<T>& values)
    {
        EZ::u32 count = static_cast<EZ::u32>(values.size());
        if (!archive.BeginArray(count))
        {
            return false;
        }

        for (auto& value : values)
        {
            if (!SerializeValue(archive, value))
            {
                archive.EndArray();
                return false;
            }
        }

        archive.EndArray();
        return true;
    }

    template<typename T>
    bool DeserializeValue(IInputArchive& archive, std::vector<T>& values)
    {
        EZ::u32 count = 0;
        if (!archive.BeginArray(count))
        {
            return false;
        }

        values.clear();
        values.resize(count);
        for (EZ::u32 index = 0; index < count; ++index)
        {
            if (!DeserializeValue(archive, values[index]))
            {
                archive.EndArray();
                return false;
            }
        }

        archive.EndArray();
        return true;
    }
}
