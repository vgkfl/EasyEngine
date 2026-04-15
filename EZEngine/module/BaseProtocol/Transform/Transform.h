#pragma once
#ifndef __B_P_TRANSFORM_H__
#define __B_P_TRANSFORM_H__

#include "DataProtocol/MathTypes.h"
#include "core/Types.h"

namespace BaseProtocol
{
	struct LocalTransform
	{
		enum : EZ::u8
		{
			Flag_LocalDirty = 1 << 0,
			Flag_WorldDirty = 1 << 1
		};

		const DataProtocol::Transform& Read() const
		{
			return value;
		}

		DataProtocol::Transform& Get()
		{
			flags |= Flag_LocalDirty;
			flags |= Flag_WorldDirty;
			return value;
		}

		void Set(const DataProtocol::Transform& newValue)
		{
			value = newValue;
			flags |= Flag_LocalDirty;
			flags |= Flag_WorldDirty;
		}

		void MarkLocalDirty()
		{
			flags |= Flag_LocalDirty;
			flags |= Flag_WorldDirty;
		}

		void MarkWorldDirty()
		{
			flags |= Flag_WorldDirty;
		}

		bool IsLocalDirty() const
		{
			return (flags & Flag_LocalDirty) != 0;
		}

		bool IsWorldDirty() const
		{
			return (flags & Flag_WorldDirty) != 0;
		}

		void ClearLocalDirty()
		{
			flags &= ~Flag_LocalDirty;
		}

		void ClearWorldDirty()
		{
			flags &= ~Flag_WorldDirty;
		}

	public:
		DataProtocol::Transform value{};
		EZ::u8 flags = Flag_LocalDirty | Flag_WorldDirty;
	};

	struct LocalMatrix
	{
		DataProtocol::Mat4 value{};
	};

	struct LocalToWorld
	{
		DataProtocol::Mat4 value{};
	};

	struct PreviousLocalToWorld
	{
		DataProtocol::Mat4 value{};
	};
}

#endif