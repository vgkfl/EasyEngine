#pragma once
#ifndef __RENDER_FRAME_DATA_H__
#define __RENDER_FRAME_DATA_H__

#include <vector>

#include "core/Types.h"
#include "DataProtocol/MathTypes.h"
#include "DataProtocol/MeshAsset.h"
#include "DataProtocol/MaterialAsset.h"
#include "DataProtocol/SkeletonAsset.h"
#include "DataProtocol/SkinAsset.h"
#include "BaseProtocol/Animation/SkinningPaletteComponent.h"

struct RenderCameraFrameData
{
	float view[16]{};
	float proj[16]{};
	float viewProj[16]{};

	float cameraPos[3]{ 0.0f, 0.0f, 0.0f };
	DataProtocol::Vec4 clearColor{ 0.08f, 0.10f, 0.14f, 1.0f };

	bool valid = false;
};

struct RenderMainLightFrameData
{
	bool valid = false;

	float lightDir[3]{ -0.6f, -1.0f, -0.3f };
	float lightColor[3]{ 1.0f, 0.97f, 0.92f };
};

struct StaticRenderableFrameItem
{
	const DataProtocol::MeshAsset* meshAsset = nullptr;
	const DataProtocol::MaterialAsset* materialAsset = nullptr;
	const DataProtocol::Mat4* localToWorld = nullptr;

	EZ::u32 subMeshIndex = 0;

	bool castShadow = true;
	bool receiveShadow = true;
};

struct SkinnedRenderableFrameItem
{
	const DataProtocol::MeshAsset* meshAsset = nullptr;
	const DataProtocol::SkeletonAsset* skeletonAsset = nullptr;
	const DataProtocol::SkinAsset* skinAsset = nullptr;

	const std::vector<DataProtocol::Mat4>* finalMatrices = nullptr;
	const DataProtocol::MaterialAsset* materialAsset = nullptr;
	const DataProtocol::Mat4* localToWorld = nullptr;

	EZ::u32 subMeshIndex = 0;

	bool castShadow = true;
	bool receiveShadow = true;
};

struct RenderFrameData
{
	DataProtocol::UVec2 drawableSize{};

	RenderCameraFrameData camera;
	RenderMainLightFrameData mainLight;

	std::vector<StaticRenderableFrameItem> staticRenderables;
	std::vector<SkinnedRenderableFrameItem> skinnedRenderables;

	void Clear()
	{
		drawableSize = {};
		camera = {};
		mainLight = {};

		staticRenderables.clear();
		skinnedRenderables.clear();
	}
};

#endif