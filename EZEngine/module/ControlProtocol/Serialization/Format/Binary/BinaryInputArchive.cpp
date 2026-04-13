#include "ControlProtocol/Serialization/Format/Binary/BinaryInputArchive.h"
#include <istream>

namespace ControlProtocol::Serialization
{
    namespace
    {
        template<typename T>
        bool ReadPod(std::istream& stream, T& value)
        {
            stream.read(reinterpret_cast<char*>(&value), sizeof(T));
            return stream.good();
        }
    }

    BinaryInputArchive::BinaryInputArchive(std::istream& stream)
        : m_stream(&stream)
    {
    }

    ArchiveMode BinaryInputArchive::GetMode() const noexcept
    {
        return ArchiveMode::Read;
    }

    bool BinaryInputArchive::BeginObject(const char* typeName, EZ::u32 version)
    {
        std::string savedTypeName;
        EZ::u32 savedVersion = 0;
        if (!ReadString(savedTypeName) || !ReadU32(savedVersion))
        {
            return false;
        }

        return savedTypeName == (typeName != nullptr ? typeName : "") && savedVersion == version;
    }

    void BinaryInputArchive::EndObject()
    {
    }

    bool BinaryInputArchive::BeginField(const char* fieldName)
    {
        std::string savedFieldName;
        if (!ReadString(savedFieldName))
        {
            return false;
        }

        return savedFieldName == (fieldName != nullptr ? fieldName : "");
    }

    void BinaryInputArchive::EndField()
    {
    }

    bool BinaryInputArchive::ReadBool(bool& value)
    {
        EZ::u8 raw = 0;
        if (!ReadU8(raw))
        {
            return false;
        }

        value = (raw != 0);
        return true;
    }

    bool BinaryInputArchive::ReadU8(EZ::u8& value)
    {
        return ReadPod(*m_stream, value);
    }

    bool BinaryInputArchive::ReadU16(EZ::u16& value)
    {
        return ReadPod(*m_stream, value);
    }

    bool BinaryInputArchive::ReadU32(EZ::u32& value)
    {
        return ReadPod(*m_stream, value);
    }

    bool BinaryInputArchive::ReadI32(EZ::i32& value)
    {
        return ReadPod(*m_stream, value);
    }

    bool BinaryInputArchive::ReadF32(EZ::f32& value)
    {
        return ReadPod(*m_stream, value);
    }

    bool BinaryInputArchive::ReadString(std::string& value)
    {
        EZ::u32 size = 0;
        if (!ReadU32(size))
        {
            return false;
        }

        value.resize(size);
        if (size == 0)
        {
            return true;
        }

        m_stream->read(value.data(), static_cast<std::streamsize>(size));
        return m_stream->good();
    }

    bool BinaryInputArchive::BeginArray(EZ::u32& count)
    {
        return ReadU32(count);
    }

    void BinaryInputArchive::EndArray()
    {
    }
}
