#pragma once
#ifndef __Tool_FBXIMPORTER_H__
#define __Tool_FBXIMPORTER_H__

#include <string>
#include <vector>
#include <memory>

#include "DataProtocol/MeshAsset.h"
#include "DataProtocol/SkeletonAsset.h"
#include "DataProtocol/SkinAsset.h"
#include "DataProtocol/MaterialAsset.h"
#include "DataProtocol/ImageAsset.h"
#include "DataProtocol/AnimationClipAsset.h"

namespace Tool
{
	struct FBXImportResult
	{
		std::unique_ptr<DataProtocol::MeshAsset> meshAsset;
		std::unique_ptr<DataProtocol::SkeletonAsset> skeletonAsset;
		std::unique_ptr<DataProtocol::SkinAsset> skinAsset;

		std::vector<std::unique_ptr<DataProtocol::MaterialAsset>> materialAssets;
		std::vector<std::unique_ptr<DataProtocol::ImageAsset>> imageAssets;
		std::vector<std::unique_ptr<DataProtocol::AnimationClipAsset>> animationAssets;
	};

	struct FBXImportOptions
	{
		bool importMesh = true;
		bool importSkeleton = true;
		bool importSkin = true;
		bool importMaterials = true;
		bool importImages = true;
		bool importAnimations = true;

		bool triangulate = true;
		bool generateMissingNormals = true;
		bool cleanSkinWeights = true;

		bool preferTextureSpecifiedUVSet = true;
		bool applyTextureUVTransform = true;
		bool flipVForOpenGL = true;

		EZ::f32 globalScale = 0.00001f;
		EZ::u32 maxBonesPerVertex = 4;
	};

	class FBXImporter
	{
	public:
		static FBXImportResult Import(const std::string& fbxPath, const FBXImportOptions& options = {});
	};
}

#endif