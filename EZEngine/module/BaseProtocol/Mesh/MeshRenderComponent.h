#pragma once
#ifndef __B_P_MESH_RENDERER_COMPONENT_H__
#define __B_P_MESH_RENDERER_COMPONENT_H__

#include <vector>

#include "DataProtocol/MeshAsset.h"
#include "DataProtocol/MaterialAsset.h"

namespace BaseProtocol
{
	struct MeshRendererComponent
	{
		const DataProtocol::MeshAsset* meshAsset = nullptr;
		std::vector<const DataProtocol::MaterialAsset*> materialAssets;

		bool visible = true;
		bool castShadow = true;
		bool receiveShadow = true;
	};
}

#endif