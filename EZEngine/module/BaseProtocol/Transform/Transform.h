#pragma once
#ifndef __B_P_TRANSFORM_H__
#define __B_P_TRANSFORM_H__

#include "DataProtocol/MathTypes.h"

namespace BaseProtocol
{
	struct LocalTransform
	{
		enum : EZ::u8
		{
			Flag_LocalDirty = 1 << 0
		};

		const DataProtocol::Transform& Read() const
		{
			return value;
		}

		DataProtocol::Transform& Get()
		{
			flags |= Flag_LocalDirty;
			return value;
		}

		void Set(const DataProtocol::Transform& newValue)
		{
			value = newValue;
			flags |= Flag_LocalDirty;
		}

		bool IsLocalDirty() const
		{
			return (flags & Flag_LocalDirty) != 0;
		}

		void ClearLocalDirty()
		{
			flags &= ~Flag_LocalDirty;
		}

	private:
		// 你自己的字段
		DataProtocol::Transform value{};
		EZ::u8 flags = Flag_LocalDirty;
	};

	//所有骨骼渲染等将直接访问
	struct LocalToWorld
	{
		DataProtocol::Mat4 value;
	};

	struct LocalMatrix
	{
		DataProtocol::Mat4 value;
	};

	struct PreviousLocalToWorld
	{
		DataProtocol::Mat4 value;
	};

	//父子级组件
	//struct HierarchyNode
	//{
	//	EZ::Entity parent = EZ::Entity{};
	//	EZ::Entity firstChild = EZ::Entity{};
	//	EZ::Entity nextSibling = EZ::Entity{};
	//	EZ::Entity prevSibling = EZ::Entity{};

	//	// 可选缓存，方便排序/调试/编辑器
	//	EZ::u32 depth = 0;
	//};
}

#endif