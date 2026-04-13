#pragma once
#ifndef __C_P_SERIALIZATION_ENUMTRAITS_H__
#define __C_P_SERIALIZATION_ENUMTRAITS_H__

#include <type_traits>

namespace ControlProtocol::Serialization
{
	template<typename T>
	struct IsEnumType : std::is_enum<T>
	{
	};

	template<typename T>
	inline constexpr bool IsEnumTypeV = IsEnumType<T>::value;

	template<typename T, typename = std::enable_if_t<IsEnumTypeV<T>>>
	constexpr auto ToUnderlying(T value) noexcept
	{
		return static_cast<std::underlying_type_t<T>>(value);
	}
}

#endif