#pragma once
#ifndef __B_P_TRANSFORM_STATE_H__
#define __B_P_TRANSFORM_STATE_H__

#include "core/Types.h"

namespace BaseProtocol
{
	enum class TransformFlags : EZ::u32
	{
		None = 0,
		WorldDirty = 1 << 0,
		HierarchyDirty = 1 << 1,
		InheritPosition = 1 << 2,
		InheritRotation = 1 << 3,
		InheritScale = 1 << 4
	};

	inline constexpr TransformFlags operator|(TransformFlags a, TransformFlags b)
	{
		return static_cast<TransformFlags>(
			static_cast<EZ::u32>(a) | static_cast<EZ::u32>(b)
			);
	}

	inline constexpr TransformFlags operator&(TransformFlags a, TransformFlags b)
	{
		return static_cast<TransformFlags>(
			static_cast<EZ::u32>(a) & static_cast<EZ::u32>(b)
			);
	}

	inline constexpr TransformFlags& operator|=(TransformFlags& a, TransformFlags b)
	{
		a = a | b;
		return a;
	}

	inline constexpr bool HasFlag(TransformFlags value, TransformFlags flag)
	{
		return (static_cast<EZ::u32>(value) & static_cast<EZ::u32>(flag)) != 0;
	}

	struct TransformState
	{
		TransformFlags flags =
			TransformFlags::WorldDirty |
			TransformFlags::HierarchyDirty |
			TransformFlags::InheritPosition |
			TransformFlags::InheritRotation |
			TransformFlags::InheritScale;
	};
}

#endif