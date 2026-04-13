#include "ControlProtocol/Serialization/Format/Binary/BinaryOutputArchive.h"
#include <ostream>

namespace ControlProtocol::Serialization
{
    namespace
    {
        template<typename T>
        bool WritePod(std::ostream& stream, const T& value)
        {
            stream.write(reinterpret_cast<const char*>(&value), sizeof(T));
            return stream.good();
        }
    }

    BinaryOutputArchive::BinaryOutputArchive(std::ostream& stream)
        : m_stream(&stream)
    {
    }

    ArchiveMode BinaryOutputArchive::GetMode() const noexcept
    {
        return ArchiveMode::Write;
    }

    bool BinaryOutputArchive::BeginObject(const char* typeName, EZ::u32 version)
    {
        std::string name = typeName != nullptr ? typeName : "";
        return WriteString(name) && WriteU32(version);
    }

    void BinaryOutputArchive::EndObject()
    {
    }

    bool BinaryOutputArchive::BeginField(const char* fieldName)
    {
        std::string name = fieldName != nullptr ? fieldName : "";
        return WriteString(name);
    }

    void BinaryOutputArchive::EndField()
    {
    }

    bool BinaryOutputArchive::WriteBool(bool value)
    {
        EZ::u8 raw = value ? 1u : 0u;
        return WriteU8(raw);
    }

    bool BinaryOutputArchive::WriteU8(EZ::u8 value)
    {
        return WritePod(*m_stream, value);
    }

    bool BinaryOutputArchive::WriteU16(EZ::u16 value)
    {
        return WritePod(*m_stream, value);
    }

    bool BinaryOutputArchive::WriteU32(EZ::u32 value)
    {
        return WritePod(*m_stream, value);
    }

    bool BinaryOutputArchive::WriteI32(EZ::i32 value)
    {
        return WritePod(*m_stream, value);
    }

    bool BinaryOutputArchive::WriteF32(EZ::f32 value)
    {
        return WritePod(*m_stream, value);
    }

    bool BinaryOutputArchive::WriteString(const std::string& value)
    {
        EZ::u32 size = static_cast<EZ::u32>(value.size());
        if (!WriteU32(size))
        {
            return false;
        }

        m_stream->write(value.data(), static_cast<std::streamsize>(value.size()));
        return m_stream->good();
    }

    bool BinaryOutputArchive::BeginArray(EZ::u32 count)
    {
        return WriteU32(count);
    }

    void BinaryOutputArchive::EndArray()
    {
    }
}
