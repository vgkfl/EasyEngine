#pragma once
#ifndef __B_P_SKINNING_PALETTE_COMPONENT_H__
#define __B_P_SKINNING_PALETTE_COMPONENT_H__

#include <vector>

#include "DataProtocol/MathTypes.h"

namespace BaseProtocol
{
	struct SkinningPaletteComponent
	{
		// 自有矩阵存储
		std::vector<DataProtocol::Mat4> finalMatrices;

		// 当前帧可指向共享缓存
		const std::vector<DataProtocol::Mat4>* sharedFinalMatrices = nullptr;

		bool dirty = true;

		void Clear()
		{
			finalMatrices.clear();
			sharedFinalMatrices = nullptr;
			dirty = false;
		}

		void UseOwnedStorage()
		{
			sharedFinalMatrices = nullptr;
		}

		void UseSharedMatrices(const std::vector<DataProtocol::Mat4>* shared)
		{
			sharedFinalMatrices = shared;
		}

		const std::vector<DataProtocol::Mat4>& GetFinalMatrices() const
		{
			if (sharedFinalMatrices)
			{
				return *sharedFinalMatrices;
			}
			return finalMatrices;
		}

		bool HasFinalMatrices() const
		{
			return !GetFinalMatrices().empty();
		}
	};
}

#endif