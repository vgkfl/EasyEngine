#pragma once
#ifndef __B_P_SKINNED_MESH_RENDERER_COMPONENT_H__
#define __B_P_SKINNED_MESH_RENDERER_COMPONENT_H__

#include <vector>

#include "DataProtocol/MeshAsset.h"
#include "DataProtocol/SkeletonAsset.h"
#include "DataProtocol/SkinAsset.h"
#include "DataProtocol/MaterialAsset.h"

namespace BaseProtocol
{
	struct SkinnedMeshRendererComponent
	{
		const DataProtocol::MeshAsset* meshAsset = nullptr;
		const DataProtocol::SkeletonAsset* skeletonAsset = nullptr;
		const DataProtocol::SkinAsset* skinAsset = nullptr;

		std::vector<DataProtocol::MaterialAsset*> materialAssets;

		bool visible = true;
		bool castShadow = true;
		bool receiveShadow = true;
	};
}

#endif