#pragma once

namespace ControlProtocol::Serialization
{
	inline bool SerializeValue(IOutputArchive& archive, bool value)
	{
		return archive.WriteBool(value);
	}

	inline bool SerializeValue(IOutputArchive& archive, EZ::u8 value)
	{
		return archive.WriteU8(value);
	}

	inline bool SerializeValue(IOutputArchive& archive, EZ::u16 value)
	{
		return archive.WriteU16(value);
	}

	inline bool SerializeValue(IOutputArchive& archive, EZ::u32 value)
	{
		return archive.WriteU32(value);
	}

	inline bool SerializeValue(IOutputArchive& archive, EZ::i32 value)
	{
		return archive.WriteI32(value);
	}

	inline bool SerializeValue(IOutputArchive& archive, EZ::f32 value)
	{
		return archive.WriteF32(value);
	}

	inline bool SerializeValue(IOutputArchive& archive, const std::string& value)
	{
		return archive.WriteString(value);
	}

	inline bool DeserializeValue(IInputArchive& archive, bool& value)
	{
		return archive.ReadBool(value);
	}

	inline bool DeserializeValue(IInputArchive& archive, EZ::u8& value)
	{
		return archive.ReadU8(value);
	}

	inline bool DeserializeValue(IInputArchive& archive, EZ::u16& value)
	{
		return archive.ReadU16(value);
	}

	inline bool DeserializeValue(IInputArchive& archive, EZ::u32& value)
	{
		return archive.ReadU32(value);
	}

	inline bool DeserializeValue(IInputArchive& archive, EZ::i32& value)
	{
		return archive.ReadI32(value);
	}

	inline bool DeserializeValue(IInputArchive& archive, EZ::f32& value)
	{
		return archive.ReadF32(value);
	}

	inline bool DeserializeValue(IInputArchive& archive, std::string& value)
	{
		return archive.ReadString(value);
	}
}