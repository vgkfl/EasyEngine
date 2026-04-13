#include "Test01PerfScenes.h"

#include <array>
#include <cstdint>
#include <cstring>
#include <random>

#include "BaseProtocol/Animation/AnimatorComponent.h"
#include "BaseProtocol/Camera/CameraComponent.h"
#include "BaseProtocol/Light/LightComponent.h"
#include "BaseProtocol/Mesh/MeshRenderComponent.h"
#include "BaseProtocol/Mesh/SkinnedMeshRendererComponent.h"
#include "BaseProtocol/Script/ScriptComponent.h"

#include "ControlProtocol/CharacterAssetManager/CharacterAssetManager.h"
#include "ControlProtocol/EntityManager/EntityManager.h"
#include "ControlProtocol/TransformManager/TransformManager.h"

#include "DataProtocol/MaterialAsset.h"
#include "DataProtocol/MeshAsset.h"

#include "core/Context/RunTimeContext/WorldContext.h"

namespace
{
	struct DebugVertex
	{
		float px, py, pz;
		float nx, ny, nz;
	};

	struct CrowdBuildConfig
	{
		int rowCount = 1;
		int colCount = 1;
		float spacingX = 2.2f;
		float spacingZ = 2.2f;

		bool randomState = false;
		bool randomNormalizedTime = false;
		bool randomPlayRate = false;
		bool useIdleRunOnly = false;
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

	void CreateCommonEnvironment(EZ::WorldContext& world, std::vector<EZ::Entity>& outSceneRoots)
	{
		auto* entityManager = world.TryGet<ControlProtocol::EntityManager>();
		auto* transformManager = world.TryGet<ControlProtocol::TransformManager>();
		if (!entityManager || !transformManager)
		{
			return;
		}

		// Camera
		{
			EZ::Entity camera = transformManager->CreateEntity({ 0.0f, 1.6f, 4.2f });

			auto& scriptComponent = entityManager->AddComponent<BaseProtocol::ScriptComponent>(camera);
			scriptComponent.AddScript("PlayerController");

			auto& cameraComponent = entityManager->AddComponent<BaseProtocol::CameraComponent>(camera);
			cameraComponent.isMainCamera = true;
			cameraComponent.fovYDegrees = 45.0f;
			cameraComponent.nearClip = 0.1f;
			cameraComponent.farClip = 1000.0f;
			cameraComponent.clearColor = { 0.08f, 0.10f, 0.14f, 1.0f };

			outSceneRoots.push_back(camera);
		}

		// Directional light
		{
			EZ::Entity light = transformManager->CreateEntity({ -3.0f, 6.0f, 4.0f });
			auto& lightComponent = entityManager->AddComponent<BaseProtocol::LightComponent>(light);
			lightComponent.type = BaseProtocol::LightType::Directional;
			lightComponent.color = { 1.0f, 0.97f, 0.92f };
			lightComponent.intensity = 1.2f;
			lightComponent.castShadow = false;

			outSceneRoots.push_back(light);
		}

		// Ground
		{
			EZ::Entity ground = transformManager->CreateEntity({ 0.0f, -0.25f, 0.0f });
			AttachCubeRenderer(*entityManager, ground, &g_GroundMaterial);

			if (auto* local = entityManager->TryGetComponent<BaseProtocol::LocalTransform>(ground))
			{
				auto& t = local->Get();
				t.scale = { 16.0f, 0.5f, 16.0f };
			}

			outSceneRoots.push_back(ground);
		}

		const DataProtocol::Vec3 boxPositions[] =
		{
			{ -2.5f, 0.5f, -1.5f },
			{ -1.0f, 0.5f, -1.8f },
			{  2.2f, 0.5f, -1.0f },
			{  2.2f, 1.5f, -1.0f }
		};

		for (const auto& pos : boxPositions)
		{
			EZ::Entity box = transformManager->CreateEntity(pos);
			AttachCubeRenderer(*entityManager, box, &g_BoxMaterial);
			outSceneRoots.push_back(box);
		}
	}

	bool EnsureAnbiResourcesLoaded(EZ::WorldContext& world)
	{
		auto* characterAssetManager = world.TryGet<ControlProtocol::CharacterAssetManager>();
		if (!characterAssetManager)
		{
			return false;
		}

		if (!characterAssetManager->FindCharacter("Anbi"))
		{
			if (!characterAssetManager->LoadCharacter(
				"Anbi",
				"Project/Test01/res/fbx/Anbi/Anbi.FBX",
				0.01f,
				4))
			{
				return false;
			}
		}

		if (!characterAssetManager->FindClip("Anbi", "Idle"))
		{
			characterAssetManager->LoadAnimationClip("Anbi", "Idle", "Project/Test01/res/fbx/Anbi/Anbi@Idle.FBX", 0.01f, 4);
			characterAssetManager->LoadAnimationClip("Anbi", "Idle_AFK", "Project/Test01/res/fbx/Anbi/Anbi@Idle_AFK.FBX", 0.01f, 4);
			characterAssetManager->LoadAnimationClip("Anbi", "Walk", "Project/Test01/res/fbx/Anbi/Anbi@Walk.FBX", 0.01f, 4);
			characterAssetManager->LoadAnimationClip("Anbi", "Run", "Project/Test01/res/fbx/Anbi/Anbi@Run.FBX", 0.01f, 4);
			characterAssetManager->LoadAnimationClip("Anbi", "Run_End_L", "Project/Test01/res/fbx/Anbi/Anbi@Run_End_L.FBX", 0.01f, 4);
			characterAssetManager->LoadAnimationClip("Anbi", "Run_End_R", "Project/Test01/res/fbx/Anbi/Anbi@Run_End_R.FBX", 0.01f, 4);
			characterAssetManager->LoadAnimationClip("Anbi", "TurnBack", "Project/Test01/res/fbx/Anbi/Anbi@TurnBack.FBX", 0.01f, 4);
			characterAssetManager->LoadAnimationClip("Anbi", "Evade_Front", "Project/Test01/res/fbx/Anbi/Anbi@Evade_Front.FBX", 0.01f, 4);
			characterAssetManager->LoadAnimationClip("Anbi", "Evade_Back", "Project/Test01/res/fbx/Anbi/Anbi@Evade_Back.FBX", 0.01f, 4);
			characterAssetManager->LoadAnimationClip("Anbi", "SwitchIn_Normal", "Project/Test01/res/fbx/Anbi/Anbi@SwitchIn_Normal.FBX", 0.01f, 4);
			characterAssetManager->LoadAnimationClip("Anbi", "SwitchOut_Normal", "Project/Test01/res/fbx/Anbi/Anbi@SwitchOut_Normal.FBX", 0.01f, 4);

			characterAssetManager->LoadAnimationClip("Anbi", "Attack_Normal_1", "Project/Test01/res/fbx/Anbi/Anbi@Attack_Normal_1.FBX", 0.01f, 4);
			characterAssetManager->LoadAnimationClip("Anbi", "Attack_Normal_1_End", "Project/Test01/res/fbx/Anbi/Anbi@Attack_Normal_1_End.FBX", 0.01f, 4);
			characterAssetManager->LoadAnimationClip("Anbi", "Attack_Normal_2", "Project/Test01/res/fbx/Anbi/Anbi@Attack_Normal_2.FBX", 0.01f, 4);
			characterAssetManager->LoadAnimationClip("Anbi", "Attack_Normal_2_End", "Project/Test01/res/fbx/Anbi/Anbi@Attack_Normal_2_End.FBX", 0.01f, 4);
			characterAssetManager->LoadAnimationClip("Anbi", "Attack_Normal_3", "Project/Test01/res/fbx/Anbi/Anbi@Attack_Normal_3.FBX", 0.01f, 4);
			characterAssetManager->LoadAnimationClip("Anbi", "Attack_Normal_3_End", "Project/Test01/res/fbx/Anbi/Anbi@Attack_Normal_3_End.FBX", 0.01f, 4);
			characterAssetManager->LoadAnimationClip("Anbi", "Attack_Normal_4", "Project/Test01/res/fbx/Anbi/Anbi@Attack_Normal_4.FBX", 0.01f, 4);
			characterAssetManager->LoadAnimationClip("Anbi", "Attack_Normal_4_End", "Project/Test01/res/fbx/Anbi/Anbi@Attack_Normal_4_End.FBX", 0.01f, 4);

			characterAssetManager->LoadAnimationClip("Anbi", "BigSkill", "Project/Test01/res/fbx/Anbi/Anbi@BigSkill.FBX", 0.01f, 4);
			characterAssetManager->LoadAnimationClip("Anbi", "BigSkill_Start", "Project/Test01/res/fbx/Anbi/Anbi@BigSkill_Start.FBX", 0.01f, 4);
			characterAssetManager->LoadAnimationClip("Anbi", "BigSkill_End", "Project/Test01/res/fbx/Anbi/Anbi@BigSkill_End.FBX", 0.01f, 4);
		}

		if (!characterAssetManager->FindController("AnbiController"))
		{
			characterAssetManager->CreateController("AnbiController", "Idle");

			{
				DataProtocol::AnimatorParameterDesc p{};
				p.name = "Speed";
				p.type = DataProtocol::AnimatorParameterType::Float;
				p.defaultFloat = 0.0f;
				characterAssetManager->AddControllerParameter("AnbiController", p);
			}

			{
				DataProtocol::AnimatorParameterDesc p{};
				p.name = "Attack";
				p.type = DataProtocol::AnimatorParameterType::Trigger;
				characterAssetManager->AddControllerParameter("AnbiController", p);
			}

			characterAssetManager->AddControllerState("AnbiController", "Idle", "Anbi", "Idle", true, 1.0f, false);
			characterAssetManager->AddControllerState("AnbiController", "Idle_AFK", "Anbi", "Idle_AFK", true, 1.0f, false);
			characterAssetManager->AddControllerState("AnbiController", "Walk", "Anbi", "Walk", true, 1.0f, false);
			characterAssetManager->AddControllerState("AnbiController", "Run", "Anbi", "Run", true, 1.0f, false);

			characterAssetManager->AddControllerState("AnbiController", "Run_End_L", "Anbi", "Run_End_L", false, 1.0f, false);
			characterAssetManager->AddControllerState("AnbiController", "Run_End_R", "Anbi", "Run_End_R", false, 1.0f, false);
			characterAssetManager->AddControllerState("AnbiController", "TurnBack", "Anbi", "TurnBack", false, 1.0f, false);
			characterAssetManager->AddControllerState("AnbiController", "Evade_Front", "Anbi", "Evade_Front", false, 1.0f, false);
			characterAssetManager->AddControllerState("AnbiController", "Evade_Back", "Anbi", "Evade_Back", false, 1.0f, false);
			characterAssetManager->AddControllerState("AnbiController", "SwitchIn_Normal", "Anbi", "SwitchIn_Normal", false, 1.0f, false);
			characterAssetManager->AddControllerState("AnbiController", "SwitchOut_Normal", "Anbi", "SwitchOut_Normal", false, 1.0f, false);

			characterAssetManager->AddControllerState("AnbiController", "Attack_Normal_1", "Anbi", "Attack_Normal_1", false, 1.0f, false);
			characterAssetManager->AddControllerState("AnbiController", "Attack_Normal_1_End", "Anbi", "Attack_Normal_1_End", false, 1.0f, false);
			characterAssetManager->AddControllerState("AnbiController", "Attack_Normal_2", "Anbi", "Attack_Normal_2", false, 1.0f, false);
			characterAssetManager->AddControllerState("AnbiController", "Attack_Normal_2_End", "Anbi", "Attack_Normal_2_End", false, 1.0f, false);
			characterAssetManager->AddControllerState("AnbiController", "Attack_Normal_3", "Anbi", "Attack_Normal_3", false, 1.0f, false);
			characterAssetManager->AddControllerState("AnbiController", "Attack_Normal_3_End", "Anbi", "Attack_Normal_3_End", false, 1.0f, false);
			characterAssetManager->AddControllerState("AnbiController", "Attack_Normal_4", "Anbi", "Attack_Normal_4", false, 1.0f, false);
			characterAssetManager->AddControllerState("AnbiController", "Attack_Normal_4_End", "Anbi", "Attack_Normal_4_End", false, 1.0f, false);

			characterAssetManager->AddControllerState("AnbiController", "BigSkill", "Anbi", "BigSkill", false, 1.0f, false);
			characterAssetManager->AddControllerState("AnbiController", "BigSkill_Start", "Anbi", "BigSkill_Start", false, 1.0f, false);
			characterAssetManager->AddControllerState("AnbiController", "BigSkill_End", "Anbi", "BigSkill_End", false, 1.0f, false);

			{
				DataProtocol::AnimatorTransitionDesc t{};
				t.fromState = "Idle";
				t.toState = "Run";

				DataProtocol::AnimatorConditionDesc c{};
				c.parameterName = "Speed";
				c.mode = DataProtocol::AnimatorConditionMode::Greater;
				c.thresholdFloat = 0.1f;

				t.conditions.push_back(c);
				characterAssetManager->AddControllerTransition("AnbiController", t);
			}

			{
				DataProtocol::AnimatorTransitionDesc t{};
				t.fromState = "Run";
				t.toState = "Idle";

				DataProtocol::AnimatorConditionDesc c{};
				c.parameterName = "Speed";
				c.mode = DataProtocol::AnimatorConditionMode::Less;
				c.thresholdFloat = 0.1f;

				t.conditions.push_back(c);
				characterAssetManager->AddControllerTransition("AnbiController", t);
			}
		}

		// Idle -> Run（先放前面，Shift 直接进跑步）
		{
			DataProtocol::AnimatorTransitionDesc t{};
			t.fromState = "Idle";
			t.toState = "Run";

			DataProtocol::AnimatorConditionDesc c{};
			c.parameterName = "Speed";
			c.mode = DataProtocol::AnimatorConditionMode::Greater;
			c.thresholdFloat = 0.85f;

			t.conditions.push_back(c);
			characterAssetManager->AddControllerTransition("AnbiController", t);
		}

		// Idle -> Walk
		{
			DataProtocol::AnimatorTransitionDesc t{};
			t.fromState = "Idle";
			t.toState = "Walk";

			DataProtocol::AnimatorConditionDesc c{};
			c.parameterName = "Speed";
			c.mode = DataProtocol::AnimatorConditionMode::Greater;
			c.thresholdFloat = 0.08f;

			t.conditions.push_back(c);
			characterAssetManager->AddControllerTransition("AnbiController", t);
		}

		// Walk -> Run
		{
			DataProtocol::AnimatorTransitionDesc t{};
			t.fromState = "Walk";
			t.toState = "Run";

			DataProtocol::AnimatorConditionDesc c{};
			c.parameterName = "Speed";
			c.mode = DataProtocol::AnimatorConditionMode::Greater;
			c.thresholdFloat = 0.75f;

			t.conditions.push_back(c);
			characterAssetManager->AddControllerTransition("AnbiController", t);
		}

		// Run -> Walk
		{
			DataProtocol::AnimatorTransitionDesc t{};
			t.fromState = "Run";
			t.toState = "Walk";

			DataProtocol::AnimatorConditionDesc c{};
			c.parameterName = "Speed";
			c.mode = DataProtocol::AnimatorConditionMode::Less;
			c.thresholdFloat = 0.75f;

			t.conditions.push_back(c);
			characterAssetManager->AddControllerTransition("AnbiController", t);
		}

		// Walk -> Idle
		{
			DataProtocol::AnimatorTransitionDesc t{};
			t.fromState = "Walk";
			t.toState = "Idle";

			DataProtocol::AnimatorConditionDesc c{};
			c.parameterName = "Speed";
			c.mode = DataProtocol::AnimatorConditionMode::Less;
			c.thresholdFloat = 0.08f;

			t.conditions.push_back(c);
			characterAssetManager->AddControllerTransition("AnbiController", t);
		}

		// Run -> Idle（保底）
		{
			DataProtocol::AnimatorTransitionDesc t{};
			t.fromState = "Run";
			t.toState = "Idle";

			DataProtocol::AnimatorConditionDesc c{};
			c.parameterName = "Speed";
			c.mode = DataProtocol::AnimatorConditionMode::Less;
			c.thresholdFloat = 0.08f;

			t.conditions.push_back(c);
			characterAssetManager->AddControllerTransition("AnbiController", t);
		}

		return true;
	}

	void ApplyAnbiTextures(ControlProtocol::EntityManager& entityManager, EZ::Entity entity)
	{
		auto* renderer = entityManager.TryGetComponent<BaseProtocol::SkinnedMeshRendererComponent>(entity);
		if (!renderer)
		{
			return;
		}

		auto& materials = renderer->materialAssets;

		if (materials.size() > 0 && materials[0])
		{
			materials[0]->textureSlots[0].imageAssetPath =
				"Project/Test01/res/fbx/Anbi/PNG/Anbi_Body_D.tga.png";
		}
		if (materials.size() > 1 && materials[1])
		{
			materials[1]->textureSlots[0].imageAssetPath =
				"Project/Test01/res/fbx/Anbi/PNG/Anbi_Face_D.tga.png";
		}
		if (materials.size() > 2 && materials[2])
		{
			materials[2]->textureSlots[0].imageAssetPath =
				"Project/Test01/res/fbx/Anbi/PNG/Anbi_Hair_D.tga.png";
		}
		if (materials.size() > 3 && materials[3])
		{
			materials[3]->textureSlots[0].imageAssetPath =
				"Project/Test01/res/fbx/Anbi/PNG/Anbi_Weapon_D.tga.png";
		}
	}

	const char* PickState(std::mt19937& rng, const CrowdBuildConfig& config)
	{
		if (!config.randomState)
		{
			return "Idle";
		}

		if (config.useIdleRunOnly)
		{
			static const std::array<const char*, 2> kStates =
			{
				"Idle",
				"Run"
			};

			std::uniform_int_distribution<int> dist(0, static_cast<int>(kStates.size()) - 1);
			return kStates[dist(rng)];
		}

		static const std::array<const char*, 22> kStates =
		{
			"Idle",
			"Idle_AFK",
			"Walk",
			"Run",
			"Run_End_L",
			"Run_End_R",
			"TurnBack",
			"Evade_Front",
			"Evade_Back",
			"SwitchIn_Normal",
			"SwitchOut_Normal",
			"Attack_Normal_1",
			"Attack_Normal_1_End",
			"Attack_Normal_2",
			"Attack_Normal_2_End",
			"Attack_Normal_3",
			"Attack_Normal_3_End",
			"Attack_Normal_4",
			"Attack_Normal_4_End",
			"BigSkill",
			"BigSkill_Start",
			"BigSkill_End"
		};

		std::uniform_int_distribution<int> dist(0, static_cast<int>(kStates.size()) - 1);
		return kStates[dist(rng)];
	}

	bool IsLoopingAnbiState(const char* stateName)
	{
		return true;

		return
			std::strcmp(stateName, "Idle") == 0 ||
			std::strcmp(stateName, "Idle_AFK") == 0 ||
			std::strcmp(stateName, "Walk") == 0 ||
			std::strcmp(stateName, "Run") == 0;
	}

	void ApplyDirectAnimationState(
		ControlProtocol::CharacterAssetManager& characterAssetManager,
		BaseProtocol::AnimatorComponent& animator,
		const char* characterName,
		const char* stateName,
		float normalizedTime,
		float playRate)
	{
		const DataProtocol::AnimationClipAsset* clip =
			characterAssetManager.FindClip(characterName, stateName);

		if (!clip)
		{
			return;
		}

		animator.controller = nullptr;
		animator.currentStateName = stateName;
		animator.currentClip = clip;
		animator.playing = true;
		animator.playRate = playRate;
		animator.looping = IsLoopingAnbiState(stateName);
		animator.stateSpeed = 1.0f;
		animator.stateElapsedTime = 0.0f;

		float startTime = 0.0f;
		if (clip->duration > 0.0001f)
		{
			startTime = normalizedTime * clip->duration;

			if (animator.looping)
			{
				startTime = std::fmod(startTime, clip->duration);
				if (startTime < 0.0f)
				{
					startTime += clip->duration;
				}
			}
			else
			{
				if (startTime < 0.0f) startTime = 0.0f;
				if (startTime > clip->duration) startTime = clip->duration;
			}
		}

		animator.currentTime = startTime;
		animator.ClearRootMotion();
	}

	void SpawnAnbiCrowd(
		EZ::WorldContext& world,
		const CrowdBuildConfig& config,
		std::vector<EZ::Entity>& outSceneRoots)
	{
		auto* entityManager = world.TryGet<ControlProtocol::EntityManager>();
		auto* transformManager = world.TryGet<ControlProtocol::TransformManager>();
		auto* characterAssetManager = world.TryGet<ControlProtocol::CharacterAssetManager>();
		if (!entityManager || !transformManager || !characterAssetManager)
		{
			return;
		}

		BaseProtocol::AnimatorComponent animatorTemplate{};
		animatorTemplate.playing = true;
		animatorTemplate.playRate = 1.0f;
		animatorTemplate.applyBindPoseWhenNoClip = true;

		EZ::Entity anbiRoot = transformManager->CreateEntity({ 0.0f, 0.0f, 0.0f });
		outSceneRoots.push_back(anbiRoot);

		const float originX = -static_cast<float>(config.colCount - 1) * 0.5f * config.spacingX;
		const float originZ = -static_cast<float>(config.rowCount - 1) * 0.5f * config.spacingZ;

		std::mt19937 rng(1337u);
		std::uniform_real_distribution<float> normalizedTimeDist(0.0f, 1.0f);
		std::uniform_real_distribution<float> playRateDist(0.92f, 1.08f);

		for (int row = 0; row < config.rowCount; ++row)
		{
			for (int col = 0; col < config.colCount; ++col)
			{
				const DataProtocol::Vec3 worldPos
				{
					originX + static_cast<float>(col) * config.spacingX,
					1.0f,
					originZ + static_cast<float>(row) * config.spacingZ
				};

				EZ::Entity anbi = characterAssetManager->SpawnCharacter(
					world,
					"Anbi",
					"AnbiController",
					worldPos,
					{ 100.0f, 100.0f, 100.0f },
					animatorTemplate);

				if (anbi == EZ::Entity{})
				{
					continue;
				}

				transformManager->SetParent(anbi, anbiRoot);
				ApplyAnbiTextures(*entityManager, anbi);

				auto& animator = entityManager->GetComponent<BaseProtocol::AnimatorComponent>(anbi);

				const char* state = PickState(rng, config);
				const float normalizedTime = config.randomNormalizedTime
					? normalizedTimeDist(rng)
					: 0.0f;
				const float playRate = config.randomPlayRate
					? playRateDist(rng)
					: 1.0f;

				// 多动画演示场景：直接播 clip，不走状态机
				ApplyDirectAnimationState(
					*characterAssetManager,
					animator,
					"Anbi",
					state,
					normalizedTime,
					playRate);

				// 这里把参数清成中性值，避免后续有人误判
				animator.SetFloat("Speed", 0.0f);
			}
		}
	}

	void CreateRootMotionRunwayEnvironment(
		EZ::WorldContext& world,
		std::vector<EZ::Entity>& outSceneRoots)
	{
		auto* entityManager = world.TryGet<ControlProtocol::EntityManager>();
		auto* transformManager = world.TryGet<ControlProtocol::TransformManager>();
		if (!entityManager || !transformManager)
		{
			return;
		}

		// 固定观察相机：先不要挂 PlayerController，避免干扰 root motion 观察
		{
			EZ::Entity camera = transformManager->CreateEntity({ 0.0f, 2.2f, 6.5f });

			auto& cameraComponent = entityManager->AddComponent<BaseProtocol::CameraComponent>(camera);
			cameraComponent.isMainCamera = true;
			cameraComponent.fovYDegrees = 45.0f;
			cameraComponent.nearClip = 0.1f;
			cameraComponent.farClip = 1000.0f;
			cameraComponent.clearColor = { 0.08f, 0.10f, 0.14f, 1.0f };

			outSceneRoots.push_back(camera);
		}

		{
			EZ::Entity light = transformManager->CreateEntity({ -3.0f, 6.0f, 4.0f });
			auto& lightComponent = entityManager->AddComponent<BaseProtocol::LightComponent>(light);
			lightComponent.type = BaseProtocol::LightType::Directional;
			lightComponent.color = { 1.0f, 0.97f, 0.92f };
			lightComponent.intensity = 1.2f;
			lightComponent.castShadow = false;

			outSceneRoots.push_back(light);
		}

		// 长跑道地面
		{
			EZ::Entity ground = transformManager->CreateEntity({ 0.0f, -0.25f, 0.0f });
			AttachCubeRenderer(*entityManager, ground, &g_GroundMaterial);

			if (auto* local = entityManager->TryGetComponent<BaseProtocol::LocalTransform>(ground))
			{
				auto& t = local->Get();
				t.scale = { 4.0f, 0.5f, 40.0f };
			}

			outSceneRoots.push_back(ground);
		}

		// 跑道两侧参考块，方便肉眼观察位移
		for (int i = -10; i <= 10; ++i)
		{
			const float z = static_cast<float>(i) * 2.0f;

			EZ::Entity leftMarker = transformManager->CreateEntity({ -1.8f, 0.1f, z });
			AttachCubeRenderer(*entityManager, leftMarker, &g_BoxMaterial);
			if (auto* local = entityManager->TryGetComponent<BaseProtocol::LocalTransform>(leftMarker))
			{
				auto& t = local->Get();
				t.scale = { 0.2f, 0.2f, 0.2f };
			}
			outSceneRoots.push_back(leftMarker);

			EZ::Entity rightMarker = transformManager->CreateEntity({ 1.8f, 0.1f, z });
			AttachCubeRenderer(*entityManager, rightMarker, &g_BoxMaterial);
			if (auto* local = entityManager->TryGetComponent<BaseProtocol::LocalTransform>(rightMarker))
			{
				auto& t = local->Get();
				t.scale = { 0.2f, 0.2f, 0.2f };
			}
			outSceneRoots.push_back(rightMarker);
		}

		// 起点参考块
		{
			EZ::Entity startMarker = transformManager->CreateEntity({ 0.0f, 0.05f, 0.0f });
			AttachCubeRenderer(*entityManager, startMarker, &g_BoxMaterial);
			if (auto* local = entityManager->TryGetComponent<BaseProtocol::LocalTransform>(startMarker))
			{
				auto& t = local->Get();
				t.scale = { 0.8f, 0.1f, 0.3f };
			}
			outSceneRoots.push_back(startMarker);
		}
	}

	void SpawnRootMotionRunner(
		EZ::WorldContext& world,
		std::vector<EZ::Entity>& outSceneRoots)
	{
		auto* entityManager = world.TryGet<ControlProtocol::EntityManager>();
		auto* transformManager = world.TryGet<ControlProtocol::TransformManager>();
		auto* characterAssetManager = world.TryGet<ControlProtocol::CharacterAssetManager>();
		if (!entityManager || !transformManager || !characterAssetManager)
		{
			return;
		}

		BaseProtocol::AnimatorComponent animatorTemplate{};
		animatorTemplate.playing = true;
		animatorTemplate.playRate = 1.0f;
		animatorTemplate.applyBindPoseWhenNoClip = true;

		// 关键：打开 root motion 提取
		animatorTemplate.applyRootMotion = true;
		animatorTemplate.rootMotionBoneIndex = 2;
		animatorTemplate.rootMotionXZOnly = true;
		animatorTemplate.rootMotionIgnoreY = true;

		EZ::Entity anbi = characterAssetManager->SpawnCharacter(
			world,
			"Anbi",
			"AnbiController",
			{ 0.0f, 1.0f, 0.0f },
			{ 100.0f, 100.0f, 100.0f },
			animatorTemplate);

		if (anbi == EZ::Entity{})
		{
			return;
		}

		auto& scriptComponent = entityManager->AddComponent<BaseProtocol::ScriptComponent>(anbi);
		scriptComponent.AddScript("RootMotionDebug");
		scriptComponent.AddScript("RootMotionConsume");

		ApplyAnbiTextures(*entityManager, anbi);

		auto& animator = entityManager->GetComponent<BaseProtocol::AnimatorComponent>(anbi);
		animator.applyRootMotion = true;
		animator.rootMotionBoneIndex = 2;
		animator.rootMotionXZOnly = true;
		animator.rootMotionIgnoreY = true;
		animator.playRate = 1.0f;
		animator.Play("Run", 0.0f);
		animator.SetFloat("Speed", 1.0f);

		outSceneRoots.push_back(anbi);
	}

	void CreateThirdPersonEnvironment(EZ::WorldContext& world, std::vector<EZ::Entity>& outSceneRoots)
	{
		auto* entityManager = world.TryGet<ControlProtocol::EntityManager>();
		auto* transformManager = world.TryGet<ControlProtocol::TransformManager>();
		if (!entityManager || !transformManager)
		{
			return;
		}

		{
			EZ::Entity light = transformManager->CreateEntity({ -3.0f, 6.0f, 4.0f });
			auto& lightComponent = entityManager->AddComponent<BaseProtocol::LightComponent>(light);
			lightComponent.type = BaseProtocol::LightType::Directional;
			lightComponent.color = { 1.0f, 0.97f, 0.92f };
			lightComponent.intensity = 1.2f;
			lightComponent.castShadow = false;
			outSceneRoots.push_back(light);
		}

		{
			EZ::Entity ground = transformManager->CreateEntity({ 0.0f, -0.25f, 0.0f });
			AttachCubeRenderer(*entityManager, ground, &g_GroundMaterial);

			if (auto* local = entityManager->TryGetComponent<BaseProtocol::LocalTransform>(ground))
			{
				auto& t = local->Get();
				t.scale = { 24.0f, 0.5f, 24.0f };
			}

			outSceneRoots.push_back(ground);
		}

		const DataProtocol::Vec3 boxPositions[] =
		{
			{ -3.0f, 0.5f,  2.0f },
			{ -1.5f, 0.5f,  4.0f },
			{  2.0f, 0.5f,  3.0f },
			{  3.5f, 0.5f, -2.0f },
			{ -2.5f, 0.5f, -4.5f }
		};

		for (const auto& pos : boxPositions)
		{
			EZ::Entity box = transformManager->CreateEntity(pos);
			AttachCubeRenderer(*entityManager, box, &g_BoxMaterial);
			outSceneRoots.push_back(box);
		}
	}

	void SpawnThirdPersonPlayer(EZ::WorldContext& world, std::vector<EZ::Entity>& outSceneRoots)
	{
		auto* entityManager = world.TryGet<ControlProtocol::EntityManager>();
		auto* transformManager = world.TryGet<ControlProtocol::TransformManager>();
		auto* characterAssetManager = world.TryGet<ControlProtocol::CharacterAssetManager>();
		if (!entityManager || !transformManager || !characterAssetManager)
		{
			return;
		}

		// 玩家根实体：真正移动的是它
		EZ::Entity playerRoot = transformManager->CreateEntity({ 0.0f, 0.0f, 0.0f });
		outSceneRoots.push_back(playerRoot);

		{
			auto& scripts = entityManager->AddComponent<BaseProtocol::ScriptComponent>(playerRoot);
			scripts.AddScript("ThirdPersonController");
		}

		// 角色可视子节点：Animator 在这里
		BaseProtocol::AnimatorComponent animatorTemplate{};
		animatorTemplate.playing = true;
		animatorTemplate.playRate = 1.0f;
		animatorTemplate.applyBindPoseWhenNoClip = true;
		animatorTemplate.applyRootMotion = true;
		animatorTemplate.rootMotionBoneIndex = 2;
		animatorTemplate.rootMotionXZOnly = true;
		animatorTemplate.rootMotionIgnoreY = true;

		EZ::Entity anbi = characterAssetManager->SpawnCharacter(
			world,
			"Anbi",
			"AnbiController",
			{ 0.0f, 0.0f, 0.0f },
			{ 100.0f, 100.0f, 100.0f },
			animatorTemplate);

		if (anbi != EZ::Entity{})
		{
			transformManager->SetParent(anbi, playerRoot);
			ApplyAnbiTextures(*entityManager, anbi);

			auto& animator = entityManager->GetComponent<BaseProtocol::AnimatorComponent>(anbi);
			animator.applyRootMotion = true;
			animator.rootMotionBoneIndex = 2;
			animator.rootMotionXZOnly = true;
			animator.rootMotionIgnoreY = true;
			animator.Play("Idle", 0.0f);
			animator.SetFloat("Speed", 0.0f);
		}

		// 相机子节点：由 ThirdPersonControllerScript 统一控制
		EZ::Entity camera = transformManager->CreateEntity({ 0.0f, 2.5f, -5.5f });
		{
			auto& cameraComponent = entityManager->AddComponent<BaseProtocol::CameraComponent>(camera);
			cameraComponent.isMainCamera = true;
			cameraComponent.fovYDegrees = 45.0f;
			cameraComponent.nearClip = 0.1f;
			cameraComponent.farClip = 1000.0f;
			cameraComponent.clearColor = { 0.08f, 0.10f, 0.14f, 1.0f };
		}
		transformManager->SetParent(camera, playerRoot);
	}

	
}

namespace Test01Scenes
{
	void BaselineScene::Build(EZ::WorldContext& world, std::vector<EZ::Entity>& outSceneRoots)
	{
		CreateCommonEnvironment(world, outSceneRoots);
		if (!EnsureAnbiResourcesLoaded(world))
		{
			return;
		}

		CrowdBuildConfig config{};
		config.rowCount = 1;
		config.colCount = 1;
		config.randomState = false;
		config.randomNormalizedTime = false;
		config.randomPlayRate = false;

		SpawnAnbiCrowd(world, config, outSceneRoots);
	}

	void CrowdSyncIdleScene::Build(EZ::WorldContext& world, std::vector<EZ::Entity>& outSceneRoots)
	{
		CreateCommonEnvironment(world, outSceneRoots);
		if (!EnsureAnbiResourcesLoaded(world))
		{
			return;
		}

		CrowdBuildConfig config{};
		config.rowCount = 10;
		config.colCount = 10;
		config.randomState = false;
		config.randomNormalizedTime = false;
		config.randomPlayRate = false;

		SpawnAnbiCrowd(world, config, outSceneRoots);
	}

	void CrowdIdlePhaseOffsetScene::Build(EZ::WorldContext& world, std::vector<EZ::Entity>& outSceneRoots)
	{
		CreateCommonEnvironment(world, outSceneRoots);
		if (!EnsureAnbiResourcesLoaded(world))
		{
			return;
		}

		CrowdBuildConfig config{};
		config.rowCount = 10;
		config.colCount = 10;
		config.randomState = false;
		config.randomNormalizedTime = true;
		config.randomPlayRate = false;

		SpawnAnbiCrowd(world, config, outSceneRoots);
	}

	void CrowdMixedStatesScene::Build(EZ::WorldContext& world, std::vector<EZ::Entity>& outSceneRoots)
	{
		CreateCommonEnvironment(world, outSceneRoots);
		if (!EnsureAnbiResourcesLoaded(world))
		{
			return;
		}

		CrowdBuildConfig config{};
		config.rowCount = 10;
		config.colCount = 10;
		config.randomState = true;
		config.randomNormalizedTime = true;
		config.randomPlayRate = false; // 先固定 1.0，便于分析
		config.useIdleRunOnly = false;

		SpawnAnbiCrowd(world, config, outSceneRoots);
	}

	void RootMotionRunwayScene::Build(EZ::WorldContext& world, std::vector<EZ::Entity>& outSceneRoots)
	{
		CreateRootMotionRunwayEnvironment(world, outSceneRoots);

		if (!EnsureAnbiResourcesLoaded(world))
		{
			return;
		}

		SpawnRootMotionRunner(world, outSceneRoots);
	}

	void ThirdPersonControllerScene::Build(EZ::WorldContext& world, std::vector<EZ::Entity>& outSceneRoots)
	{
		CreateThirdPersonEnvironment(world, outSceneRoots);

		if (!EnsureAnbiResourcesLoaded(world))
		{
			return;
		}

		SpawnThirdPersonPlayer(world, outSceneRoots);
	}
}