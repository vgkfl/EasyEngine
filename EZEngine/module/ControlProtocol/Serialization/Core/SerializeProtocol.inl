#pragma once
#include <type_traits>

namespace ControlProtocol::Serialization
{
	template<typename T, std::enable_if_t<IsReflectableV<T>, int> = 0>
	bool SerializeValue(IOutputArchive& archive, T& value)
	{
		return SerializeObject(archive, value);
	}

	template<typename T, std::enable_if_t<IsReflectableV<T>, int> = 0>
	bool DeserializeValue(IInputArchive& archive, T& value)
	{
		return DeserializeObject(archive, value);
	}
}