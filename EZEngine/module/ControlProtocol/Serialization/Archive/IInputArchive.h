#pragma once
#ifndef __C_P_SERIALIZATION_IINPUTARCHIVE_H__
#define __C_P_SERIALIZATION_IINPUTARCHIVE_H__

#include "core/Types.h"
#include "ControlProtocol/Serialization/Archive/ArchiveMode.h"
#include <string>

namespace ControlProtocol::Serialization
{
    class IInputArchive
    {
    public:
        virtual ~IInputArchive() = default;

        virtual ArchiveMode GetMode() const noexcept = 0;

        virtual bool BeginObject(const char* typeName, EZ::u32 version) = 0;
        virtual void EndObject() = 0;

        virtual bool BeginField(const char* fieldName) = 0;
        virtual void EndField() = 0;

        virtual bool ReadBool(bool& value) = 0;
        virtual bool ReadU8(EZ::u8& value) = 0;
        virtual bool ReadU16(EZ::u16& value) = 0;
        virtual bool ReadU32(EZ::u32& value) = 0;
        virtual bool ReadI32(EZ::i32& value) = 0;
        virtual bool ReadF32(EZ::f32& value) = 0;
        virtual bool ReadString(std::string& value) = 0;

        virtual bool BeginArray(EZ::u32& count) = 0;
        virtual void EndArray() = 0;
    };
}

#endif
