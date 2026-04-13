#pragma once
#ifndef __C_P_SERIALIZATION_PROTOCOLTRAITS_H__
#define __C_P_SERIALIZATION_PROTOCOLTRAITS_H__

#include "Reflectable.h"
#include "core/Types.h"

namespace ControlProtocol::Serialization
{
	template<typename T, typename Enable = void>
	struct ProtocolTraits
	{
	};

	template<typename T>
	struct ProtocolTraits<T, std::enable_if_t<IsReflectableV<T>>>
	{
		static constexpr const char* TypeName() noexcept
		{
			return T::TypeName();
		}

		static constexpr EZ::u32 Version() noexcept
		{
			return T::Version();
		}
	};
}

#endif