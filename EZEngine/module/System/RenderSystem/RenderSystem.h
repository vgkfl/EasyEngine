#pragma once
#ifndef __RENDER_SYSTEM_H__
#define __RENDER_SYSTEM_H__

#include <string>
#include <unordered_map>
#include <vector>

#include "core/System/ISystem.h"
#include "core/Types.h"
#include "core/Render/RenderImageDesc.h"
#include "DataProtocol/MeshAsset.h"
#include "DataProtocol/MaterialAsset.h"
#include "System/RenderSystem/RenderFrameData.h"

namespace EZ
{
	struct WorldContext;
}

namespace ControlProtocol
{
	class EntityManager;
	class RenderDeviceController;
	class RenderImageBuffer;
}

class RenderSystem : public EZ::ISystem
{
public:
	struct FrameStats
	{
		EZ::u32 drawCalls = 0;

		EZ::u32 staticDrawCalls = 0;
		EZ::u32 skinnedDrawCalls = 0;

		EZ::u32 staticInstanceCount = 0;
		EZ::u32 skinnedInstanceCount = 0;

		void Reset()
		{
			drawCalls = 0;
			staticDrawCalls = 0;
			skinnedDrawCalls = 0;
			staticInstanceCount = 0;
			skinnedInstanceCount = 0;
		}
	};

	const FrameStats& GetFrameStats() const { return m_FrameStats; }

	const char* GetName() const override { return "RenderSystem"; }

	int Initialize(EZ::ProjectContext& project, EZ::WorldContext& world) override;
	void Shutdown(EZ::ProjectContext& project, EZ::WorldContext& world) override;
	void BeginFrame(EZ::ProjectContext& project, EZ::WorldContext& world, float deltaTime) override;
	void EndFrame(EZ::ProjectContext& project, EZ::WorldContext& world, float deltaTime) override;

	bool SetupForwardOpaqueTargets(
		EZ::WorldContext& world,
		ControlProtocol::RenderImageBuffer& imageBuffer,
		ControlProtocol::RenderDeviceController& device);

	void ExecuteForwardOpaque(
		EZ::WorldContext& world,
		ControlProtocol::RenderImageBuffer& imageBuffer,
		ControlProtocol::RenderDeviceController& device);

	bool Present(
		EZ::WorldContext& world,
		ControlProtocol::RenderImageBuffer& imageBuffer,
		ControlProtocol::RenderDeviceController& device);

private:
	struct GPUMesh
	{
		EZ::u32 vao = 0;
		EZ::u32 vbo = 0;
		EZ::u32 ebo = 0;
		EZ::u32 indexGLType = 0;
	};

	struct GPUTexture
	{
		EZ::u32 handle = 0;
		EZ::u32 width = 0;
		EZ::u32 height = 0;
	};

	struct ShaderProgram
	{
		EZ::u32 handle = 0;
	};

	struct InstanceData
	{
		float model[16];
		EZ::u32 paletteBaseOffset = 0;
		EZ::u32 padding0 = 0;
		EZ::u32 padding1 = 0;
		EZ::u32 padding2 = 0;
	};

	struct StaticBatchKey
	{
		const DataProtocol::MeshAsset* meshAsset = nullptr;
		const DataProtocol::MaterialAsset* materialAsset = nullptr;
		EZ::u32 shaderProgram = 0;
		EZ::u32 subMeshIndex = 0;

		bool operator==(const StaticBatchKey& rhs) const
		{
			return meshAsset == rhs.meshAsset
				&& materialAsset == rhs.materialAsset
				&& shaderProgram == rhs.shaderProgram
				&& subMeshIndex == rhs.subMeshIndex;
		}
	};

	struct StaticBatchKeyHash
	{
		size_t operator()(const StaticBatchKey& key) const noexcept
		{
			size_t h = 0;
			auto hashCombine = [&](size_t v)
				{
					h ^= v + 0x9e3779b9 + (h << 6) + (h >> 2);
				};

			hashCombine(std::hash<const void*>{}(key.meshAsset));
			hashCombine(std::hash<const void*>{}(key.materialAsset));
			hashCombine(std::hash<EZ::u32>{}(key.shaderProgram));
			hashCombine(std::hash<EZ::u32>{}(key.subMeshIndex));
			return h;
		}
	};

	struct StaticBatch
	{
		const DataProtocol::MeshAsset* meshAsset = nullptr;
		const DataProtocol::MaterialAsset* materialAsset = nullptr;
		EZ::u32 shaderProgram = 0;
		EZ::u32 subMeshIndex = 0;
		std::vector<const DataProtocol::Mat4*> modelMatrices;
	};

	struct SkinnedBatchKey
	{
		const DataProtocol::MeshAsset* meshAsset = nullptr;
		const DataProtocol::MaterialAsset* materialAsset = nullptr;
		const DataProtocol::SkeletonAsset* skeletonAsset = nullptr;
		const DataProtocol::SkinAsset* skinAsset = nullptr;
		EZ::u32 shaderProgram = 0;
		EZ::u32 subMeshIndex = 0;

		bool operator==(const SkinnedBatchKey& rhs) const
		{
			return meshAsset == rhs.meshAsset
				&& materialAsset == rhs.materialAsset
				&& skeletonAsset == rhs.skeletonAsset
				&& skinAsset == rhs.skinAsset
				&& shaderProgram == rhs.shaderProgram
				&& subMeshIndex == rhs.subMeshIndex;
		}
	};

	struct SkinnedBatchKeyHash
	{
		size_t operator()(const SkinnedBatchKey& key) const noexcept
		{
			size_t h = 0;
			auto hashCombine = [&](size_t v)
				{
					h ^= v + 0x9e3779b9 + (h << 6) + (h >> 2);
				};

			hashCombine(std::hash<const void*>{}(key.meshAsset));
			hashCombine(std::hash<const void*>{}(key.materialAsset));
			hashCombine(std::hash<const void*>{}(key.skeletonAsset));
			hashCombine(std::hash<const void*>{}(key.skinAsset));
			hashCombine(std::hash<EZ::u32>{}(key.shaderProgram));
			hashCombine(std::hash<EZ::u32>{}(key.subMeshIndex));
			return h;
		}
	};

	struct SkinnedBatchInstance
	{
		const DataProtocol::Mat4* modelMatrix = nullptr;
		const std::vector<DataProtocol::Mat4>* finalMatrices = nullptr;
	};

	struct SkinnedBatch
	{
		const DataProtocol::MeshAsset* meshAsset = nullptr;
		const DataProtocol::MaterialAsset* materialAsset = nullptr;
		const DataProtocol::SkeletonAsset* skeletonAsset = nullptr;
		const DataProtocol::SkinAsset* skinAsset = nullptr;
		EZ::u32 shaderProgram = 0;
		EZ::u32 subMeshIndex = 0;
		std::vector<SkinnedBatchInstance> instances;
	};

private:
	bool EnsureMeshUploaded(const DataProtocol::MeshAsset* meshAsset);
	bool EnsureShaderLoaded(const std::string& shaderPath, EZ::u32& outProgram);
	bool EnsureTextureLoaded(const std::string& imagePath, EZ::u32& outTexture);

	void DestroyAllMeshes();
	void DestroyAllShaders();
	void DestroyAllTextures();

	bool PrepareFrameData(EZ::WorldContext& world);
	bool BuildFrameData(EZ::WorldContext& world);

	bool BuildCameraFrameData(EZ::WorldContext& world, RenderCameraFrameData& outCamera) const;
	void BuildMainLightFrameData(EZ::WorldContext& world, RenderMainLightFrameData& outLight) const;
	void CollectStaticRenderables(EZ::WorldContext& world, RenderFrameData& frameData) const;
	void CollectSkinnedRenderables(EZ::WorldContext& world, RenderFrameData& frameData) const;

	bool SetupPersistentFrameTarget(
		EZ::RenderImageTag tag,
		const EZ::RenderImageDesc& desc,
		ControlProtocol::RenderImageBuffer& imageBuffer,
		ControlProtocol::RenderDeviceController& device);

	void RenderStaticPass(
		const std::vector<StaticRenderableFrameItem>& renderables,
		const RenderCameraFrameData& camera,
		const RenderMainLightFrameData& light);

	void RenderSkinnedPass(
		const std::vector<SkinnedRenderableFrameItem>& renderables,
		const RenderCameraFrameData& camera,
		const RenderMainLightFrameData& light);

private:
	std::unordered_map<const DataProtocol::MeshAsset*, GPUMesh> m_MeshCache;
	std::unordered_map<std::string, ShaderProgram> m_ShaderCache;
	std::unordered_map<std::string, GPUTexture> m_TextureCache;

	EZ::u32 m_InstanceSSBO = 0;
	EZ::u32 m_BoneSSBO = 0;

	std::vector<InstanceData> m_InstanceUploadCache;
	std::vector<DataProtocol::Mat4> m_BonePaletteUploadCache;

	mutable FrameStats m_FrameStats;
	RenderFrameData m_FrameData;
	bool m_FrameDataValid = false;
};

#endif