#pragma once
#ifndef __C_P_SERIALIZATION_BINARYINPUTARCHIVE_H__
#define __C_P_SERIALIZATION_BINARYINPUTARCHIVE_H__

#include "ControlProtocol/Serialization/Archive/IInputArchive.h"
#include <iosfwd>

namespace ControlProtocol::Serialization
{
    class BinaryInputArchive final : public IInputArchive
    {
    public:
        explicit BinaryInputArchive(std::istream& stream);

        ArchiveMode GetMode() const noexcept override;

        bool BeginObject(const char* typeName, EZ::u32 version) override;
        void EndObject() override;

        bool BeginField(const char* fieldName) override;
        void EndField() override;

        bool ReadBool(bool& value) override;
        bool ReadU8(EZ::u8& value) override;
        bool ReadU16(EZ::u16& value) override;
        bool ReadU32(EZ::u32& value) override;
        bool ReadI32(EZ::i32& value) override;
        bool ReadF32(EZ::f32& value) override;
        bool ReadString(std::string& value) override;

        bool BeginArray(EZ::u32& count) override;
        void EndArray() override;

    private:
        std::istream* m_stream = nullptr;
    };
}

#endif
