#pragma once
#include <type_traits>

namespace ControlProtocol::Serialization
{

	template<typename T, std::enable_if_t<IsEnumTypeV<T>, int> = 0>
	bool SerializeValue(IOutputArchive& archive, T& value)
	{
		using Underlying = std::underlying_type_t<T>;
		Underlying temp = static_cast<Underlying>(value);
		return SerializeValue(archive, temp);
	}

	template<typename T, std::enable_if_t<IsEnumTypeV<T>, int> = 0>
	bool DeserializeValue(IInputArchive& archive, T& value)
	{
		using Underlying = std::underlying_type_t<T>;
		Underlying temp{};
		if (!DeserializeValue(archive, temp))
		{
			return false;
		}

		value = static_cast<T>(temp);
		return true;
	}
}