#pragma once
#ifndef __C_P_SERIALIZATION_JSONOUTPUTARCHIVE_H__
#define __C_P_SERIALIZATION_JSONOUTPUTARCHIVE_H__

#include "ControlProtocol/Serialization/Archive/IOutputArchive.h"
#include <memory>
#include <iosfwd>

namespace ControlProtocol::Serialization
{
    class JsonOutputArchive final : public IOutputArchive
    {
    public:
        explicit JsonOutputArchive(std::ostream& stream);
        ~JsonOutputArchive() override;

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
        class Impl;
        std::unique_ptr<Impl> m_impl;
    };
}

#endif
