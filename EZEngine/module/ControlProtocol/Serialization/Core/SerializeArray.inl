#pragma once

#include <array>

namespace ControlProtocol::Serialization
{
	template<typename T, std::size_t N>
	bool SerializeValue(IOutputArchive& archive, std::array<T, N>& values)
	{
		if (!archive.BeginArray(static_cast<EZ::u32>(N)))
		{
			return false;
		}

		for (std::size_t i = 0; i < N; ++i)
		{
			if (!SerializeValue(archive, values[i]))
			{
				archive.EndArray();
				return false;
			}
		}

		archive.EndArray();
		return true;
	}

	template<typename T, std::size_t N>
	bool DeserializeValue(IInputArchive& archive, std::array<T, N>& values)
	{
		EZ::u32 count = 0;
		if (!archive.BeginArray(count))
		{
			return false;
		}

		if (count != static_cast<EZ::u32>(N))
		{
			archive.EndArray();
			return false;
		}

		for (std::size_t i = 0; i < N; ++i)
		{
			if (!DeserializeValue(archive, values[i]))
			{
				archive.EndArray();
				return false;
			}
		}

		archive.EndArray();
		return true;
	}
}