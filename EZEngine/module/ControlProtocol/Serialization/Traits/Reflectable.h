#pragma once
#ifndef __C_P_SERIALIZATION_REFLECTABLE_H__
#define __C_P_SERIALIZATION_REFLECTABLE_H__

#include <type_traits>
#include <utility>
#include "core/Types.h"

namespace ControlProtocol::Serialization
{
	struct FieldProbeVisitor
	{
		template<typename TValue>
		void operator()(const char*, TValue&&) const noexcept
		{
		}
	};

	template<typename T, typename = void>
	struct IsReflectable : std::false_type
	{
	};

	template<typename T>
	struct IsReflectable<T, std::void_t<
		decltype(T::TypeName()),
		decltype(T::Version()),
		decltype(std::declval<T&>().Reflect(std::declval<FieldProbeVisitor>()))
		>> : std::true_type
	{
	};

	template<typename T>
	inline constexpr bool IsReflectableV = IsReflectable<T>::value;
}

#endif