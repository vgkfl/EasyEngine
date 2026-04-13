#pragma once
#ifndef __C_P_SERIALIZATION_SERIALIZEVALUE_H__
#define __C_P_SERIALIZATION_SERIALIZEVALUE_H__

#include "ControlProtocol/Serialization/Core/SerializeObject.h"
#include "ControlProtocol/Serialization/Traits/EnumTraits.h"
#include <string>
#include <vector>
#include <array>

namespace ControlProtocol::Serialization
{
	bool SerializeValue(IOutputArchive& archive, bool value);
	bool SerializeValue(IOutputArchive& archive, EZ::u8 value);
	bool SerializeValue(IOutputArchive& archive, EZ::u16 value);
	bool SerializeValue(IOutputArchive& archive, EZ::u32 value);
	bool SerializeValue(IOutputArchive& archive, EZ::i32 value);
	bool SerializeValue(IOutputArchive& archive, EZ::f32 value);
	bool SerializeValue(IOutputArchive& archive, const std::string& value);

	bool DeserializeValue(IInputArchive& archive, bool& value);
	bool DeserializeValue(IInputArchive& archive, EZ::u8& value);
	bool DeserializeValue(IInputArchive& archive, EZ::u16& value);
	bool DeserializeValue(IInputArchive& archive, EZ::u32& value);
	bool DeserializeValue(IInputArchive& archive, EZ::i32& value);
	bool DeserializeValue(IInputArchive& archive, EZ::f32& value);
	bool DeserializeValue(IInputArchive& archive, std::string& value);
}

#include "ControlProtocol/Serialization/Core/SerializePrimitive.inl"
#include "ControlProtocol/Serialization/Core/SerializeEnum.inl"
#include "ControlProtocol/Serialization/Core/SerializeArray.inl"
#include "ControlProtocol/Serialization/Core/SerializeVector.inl"
#include "ControlProtocol/Serialization/Core/SerializeProtocol.inl"

#endif