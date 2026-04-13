#pragma once
#ifndef __C_P_SERIALIZATION_BINARYOUTPUTARCHIVE_H__
#define __C_P_SERIALIZATION_BINARYOUTPUTARCHIVE_H__

#include "ControlProtocol/Serialization/Archive/IOutputArchive.h"
#include <iosfwd>

namespace ControlProtocol::Serialization
{
    class BinaryOutputArchive final : public IOutputArchive
    {
    public:
        explicit BinaryOutputArchive(std::ostream& stream);

        ArchiveMode GetMode() const noexcept override;

        bool BeginObject(const char* typeName, EZ::u32 version) override;
        void EndObject() override;

        bool BeginField(const char* fieldName) override;
        void EndField() override;

        bool WriteBool(bool value) override;
        bool WriteU8(EZ::u8 value) override;
        bool WriteU16(EZ::u16 value) override;
        bool WriteU32(EZ::u32 value) override;
        bool WriteI32(EZ::i32 value) override;
        bool WriteF32(EZ::f32 value) override;
        bool WriteString(const std::string& value) override;

        bool BeginArray(EZ::u32 count) override;
        void EndArray() override;

    private:
        std::ostream* m_stream = nullptr;
    };
}

#endif
