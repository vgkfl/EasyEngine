#include "Test01.h"
#include <DataProtocol/MeshAsset.h>
#include <DataProtocol/MaterialAsset.h>
#include <System/RenderSystem/RenderSystem.h>
#include <core/Entity/Entity.h>
#include <core/ScriptManager/ScriptRegistry.h>
#include "Scripts/PlayerControllerScript.h"
#include <Launcher/GameLauncher/Scene/SceneManager.h>
#include "Scenes/Test01PerfScenes.h"
#include <BaseProtocol/Mesh/MeshRenderComponent.h>
#include "Scripts/RootMotionDebugScript.h"
#include "Scripts/RootMotionConsumeScript.h"
#include "Scripts/ThirdPersonControllerScript.h"




namespace
{
	struct DebugVertex
	{
		float px, py, pz;
		float nx, ny, nz;
	};

	DataProtocol::MeshAsset BuildCubeMeshAsset()
	{
		DataProtocol::MeshAsset mesh{};
		mesh.sourcePath = "builtin://cube";
		mesh.cookedAssetPath = "builtin://cube";
		mesh.isImported = false;

		mesh.vertexLayout.stride = sizeof(DebugVertex);
		mesh.vertexLayout.attributes = {
			{ DataProtocol::VertexSemantic::Position, 0, 3, DataProtocol::VertexScalarType::Float32, false, 0 },
			{ DataProtocol::VertexSemantic::Normal,   0, 3, DataProtocol::VertexScalarType::Float32, false, 12 }
		};

		const std::array<DebugVertex, 24> vertices =
		{ {
				{ 0.5f,-0.5f,-0.5f, 1,0,0 }, { 0.5f,-0.5f, 0.5f, 1,0,0 }, { 0.5f, 0.5f, 0.5f, 1,0,0 }, { 0.5f, 0.5f,-0.5f, 1,0,0 },
				{-0.5f,-0.5f, 0.5f,-1,0,0 }, {-0.5f,-0.5f,-0.5f,-1,0,0 }, {-0.5f, 0.5f,-0.5f,-1,0,0 }, {-0.5f, 0.5f, 0.5f,-1,0,0 },
				{-0.5f, 0.5f,-0.5f, 0,1,0 }, { 0.5f, 0.5f,-0.5f, 0,1,0 }, { 0.5f, 0.5f, 0.5f, 0,1,0 }, {-0.5f, 0.5f, 0.5f, 0,1,0 },
				{-0.5f,-0.5f, 0.5f, 0,-1,0 }, { 0.5f,-0.5f, 0.5f, 0,-1,0 }, { 0.5f,-0.5f,-0.5f, 0,-1,0 }, {-0.5f,-0.5f,-0.5f, 0,-1,0 },
				{-0.5f,-0.5f, 0.5f, 0,0,1 }, {-0.5f, 0.5f, 0.5f, 0,0,1 }, { 0.5f, 0.5f, 0.5f, 0,0,1 }, { 0.5f,-0.5f, 0.5f, 0,0,1 },
				{ 0.5f,-0.5f,-0.5f, 0,0,-1 }, { 0.5f, 0.5f,-0.5f, 0,0,-1 }, {-0.5f, 0.5f,-0.5f, 0,0,-1 }, {-0.5f,-0.5f,-0.5f, 0,0,-1 }
			} };

		const std::array<EZ::u32, 36> indices =
		{ {
			0,2,1, 0,3,2,
			4,6,5, 4,7,6,
			8,10,9, 8,11,10,
			12,14,13, 12,15,14,
			16,18,17, 16,19,18,
			20,22,21, 20,23,22
		} };

		mesh.vertexCount = static_cast<EZ::u32>(vertices.size());
		mesh.indexCount = static_cast<EZ::u32>(indices.size());
		mesh.indexType = DataProtocol::IndexScalarType::UInt32;

		mesh.vertexRawData.resize(sizeof(vertices));
		std::memcpy(mesh.vertexRawData.data(), vertices.data(), sizeof(vertices));

		mesh.indexRawData.resize(sizeof(indices));
		std::memcpy(mesh.indexRawData.data(), indices.data(), sizeof(indices));

		mesh.subMeshes.push_back({ 0, mesh.indexCount, 0 });
		mesh.bounds.min = { -0.5f, -0.5f, -0.5f };
		mesh.bounds.max = { 0.5f,  0.5f,  0.5f };
		mesh.hasSkinning = false;
		return mesh;
	}

	DataProtocol::MaterialAsset BuildMaterial(const char* name, float r, float g, float b)
	{
		DataProtocol::MaterialAsset material{};
		material.sourcePath = name;
		material.cookedAssetPath = name;
		material.isImported = false;
		material.shadingModel = DataProtocol::MaterialShadingModel::Lit;
		material.baseColor = { r, g, b, 1.0f };
		return material;
	}

	DataProtocol::MeshAsset g_CubeMesh = BuildCubeMeshAsset();
	DataProtocol::MaterialAsset g_GroundMaterial = BuildMaterial("builtin://ground", 0.25f, 0.35f, 0.25f);
	DataProtocol::MaterialAsset g_BoxMaterial = BuildMaterial("builtin://box", 0.65f, 0.55f, 0.42f);

	void AttachCubeRenderer(
		ControlProtocol::EntityManager& entityManager,
		EZ::Entity entity,
		const DataProtocol::MaterialAsset* material)
	{
		auto& renderer = entityManager.AddComponent<BaseProtocol::MeshRendererComponent>(entity);
		renderer.meshAsset = &g_CubeMesh;
		if (material)
		{
			renderer.materialAssets.push_back(material);
		}
		renderer.visible = true;
		renderer.castShadow = true;
		renderer.receiveShadow = true;
	}
}

void Test01Project::RegisterScripts(EZ::WorldContext& world)
{
	auto* registry = world.TryGet<EZ::ScriptRegistry>();
	if (!registry)
	{
		return;
	}

	registry->Register<PlayerControllerScript>("PlayerController");
	registry->Register<RootMotionDebugScript>("RootMotionDebug");
	registry->Register<RootMotionConsumeScript>("RootMotionConsume");
	registry->Register<ThirdPersonControllerScript>("ThirdPersonController");
}

void Test01Project::RegisterStartupScene(EZ::WorldContext& world)
{
	auto* sceneManager = world.TryGet<GameScene::SceneManager>();
	if (!sceneManager)
	{
		return;
	}

	sceneManager->RegisterScene(std::make_unique<Test01Scenes::BaselineScene>());
	sceneManager->RegisterScene(std::make_unique<Test01Scenes::CrowdSyncIdleScene>());
	sceneManager->RegisterScene(std::make_unique<Test01Scenes::CrowdIdlePhaseOffsetScene>());
	sceneManager->RegisterScene(std::make_unique<Test01Scenes::CrowdMixedStatesScene>());
	sceneManager->RegisterScene(std::make_unique<Test01Scenes::RootMotionRunwayScene>());
	sceneManager->RegisterScene(std::make_unique<Test01Scenes::ThirdPersonControllerScene>());

	// Ä¬ÈÏÆô¶¯³¡¾°
	sceneManager->LoadScene("CrowdMixedStates", world);
}