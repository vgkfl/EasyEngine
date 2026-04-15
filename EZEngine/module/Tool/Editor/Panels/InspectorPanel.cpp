#include "Tool/Editor/Panels/InspectorPanel.h"

#include <string>

#include "imgui/imgui.h"

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"

#include "BaseProtocol/Animation/AnimatorComponent.h"
#include "BaseProtocol/Animation/BonePoseComponent.h"
#include "BaseProtocol/Animation/SkinningPaletteComponent.h"
#include "BaseProtocol/Camera/CameraComponent.h"
#include "BaseProtocol/Light/LightComponent.h"
#include "BaseProtocol/Mesh/MeshRenderComponent.h"
#include "BaseProtocol/Mesh/SkinnedMeshRendererComponent.h"
#include "BaseProtocol/Script/ScriptComponent.h"
#include "BaseProtocol/Transform/Transform.h"

#include "ControlProtocol/EntityManager/EntityManager.h"
#include "ControlProtocol/TransformManager/TransformManager.h"
#include "Tool/Editor/EditorContext.h"

namespace
{
	static glm::vec3 ToGlm(const DataProtocol::Vec3& v)
	{
		return glm::vec3(v.x, v.y, v.z);
	}

	static DataProtocol::Vec3 ToDataVec3(const glm::vec3& v)
	{
		return DataProtocol::Vec3{ v.x, v.y, v.z };
	}

	static glm::quat ToGlm(const DataProtocol::Quat& q)
	{
		return glm::quat(q.w, q.x, q.y, q.z);
	}

	static DataProtocol::Quat ToDataQuat(const glm::quat& q)
	{
		glm::quat n = glm::normalize(q);
		return DataProtocol::Quat{ n.x, n.y, n.z, n.w };
	}

	static const char* SafeAssetPath(const DataProtocol::MeshAsset* asset)
	{
		if (!asset)
		{
			return "<null>";
		}

		if (!asset->sourcePath.empty())
		{
			return asset->sourcePath.c_str();
		}

		if (!asset->cookedAssetPath.empty())
		{
			return asset->cookedAssetPath.c_str();
		}

		return "<unnamed mesh>";
	}

	static const char* SafeAssetPath(const DataProtocol::MaterialAsset* asset)
	{
		if (!asset)
		{
			return "<null>";
		}

		if (!asset->sourcePath.empty())
		{
			return asset->sourcePath.c_str();
		}

		if (!asset->cookedAssetPath.empty())
		{
			return asset->cookedAssetPath.c_str();
		}

		return "<unnamed material>";
	}

	static const char* SafeAssetPath(const DataProtocol::SkeletonAsset* asset)
	{
		if (!asset)
		{
			return "<null>";
		}

		if (!asset->sourcePath.empty())
		{
			return asset->sourcePath.c_str();
		}

		if (!asset->cookedAssetPath.empty())
		{
			return asset->cookedAssetPath.c_str();
		}

		return "<unnamed skeleton>";
	}

	static const char* SafeAssetPath(const DataProtocol::SkinAsset* asset)
	{
		if (!asset)
		{
			return "<null>";
		}

		if (!asset->sourcePath.empty())
		{
			return asset->sourcePath.c_str();
		}

		if (!asset->cookedAssetPath.empty())
		{
			return asset->cookedAssetPath.c_str();
		}

		return "<unnamed skin>";
	}

	static void DrawLocalTransformSection(
		ControlProtocol::EntityManager& entityManager,
		ControlProtocol::TransformManager& transformManager,
		EZ::Entity entity)
	{
		(void)transformManager;

		auto* localTransform = entityManager.TryGetComponent<BaseProtocol::LocalTransform>(entity);
		if (!localTransform)
		{
			ImGui::TextDisabled("LocalTransform not found.");
			return;
		}

		DataProtocol::Transform value = localTransform->Read();

		float position[3] =
		{
			value.position.x,
			value.position.y,
			value.position.z
		};

		glm::vec3 eulerDegrees = glm::degrees(glm::eulerAngles(ToGlm(value.rotation)));
		float rotation[3] =
		{
			eulerDegrees.x,
			eulerDegrees.y,
			eulerDegrees.z
		};

		float scale[3] =
		{
			value.scale.x,
			value.scale.y,
			value.scale.z
		};

		bool changed = false;
		changed |= ImGui::DragFloat3("Position", position, 0.05f);
		changed |= ImGui::DragFloat3("Rotation", rotation, 0.25f);
		changed |= ImGui::DragFloat3("Scale", scale, 0.05f);

		if (changed)
		{
			value.position = DataProtocol::Vec3{ position[0], position[1], position[2] };
			value.rotation = ToDataQuat(glm::quat(glm::radians(glm::vec3(rotation[0], rotation[1], rotation[2]))));
			value.scale = DataProtocol::Vec3{ scale[0], scale[1], scale[2] };

			localTransform->Set(value);
		}
	}

	static void DrawMeshRendererSection(BaseProtocol::MeshRendererComponent& renderer)
	{
		ImGui::Checkbox("Visible", &renderer.visible);
		ImGui::Checkbox("Cast Shadow", &renderer.castShadow);
		ImGui::Checkbox("Receive Shadow", &renderer.receiveShadow);

		ImGui::Separator();
		ImGui::Text("Mesh: %s", SafeAssetPath(renderer.meshAsset));

		if (renderer.meshAsset)
		{
			ImGui::Text("Vertex Count: %u", renderer.meshAsset->vertexCount);
			ImGui::Text("Index Count: %u", renderer.meshAsset->indexCount);
			ImGui::Text("SubMesh Count: %u", static_cast<EZ::u32>(renderer.meshAsset->subMeshes.size()));
		}

		ImGui::Separator();
		ImGui::Text("Material Count: %u", static_cast<EZ::u32>(renderer.materialAssets.size()));

		for (EZ::u32 i = 0; i < static_cast<EZ::u32>(renderer.materialAssets.size()); ++i)
		{
			ImGui::BulletText("Material[%u]: %s", i, SafeAssetPath(renderer.materialAssets[i]));
		}
	}

	static void DrawSkinnedMeshRendererSection(BaseProtocol::SkinnedMeshRendererComponent& renderer)
	{
		ImGui::Checkbox("Visible", &renderer.visible);
		ImGui::Checkbox("Cast Shadow", &renderer.castShadow);
		ImGui::Checkbox("Receive Shadow", &renderer.receiveShadow);

		ImGui::Separator();
		ImGui::Text("Mesh: %s", SafeAssetPath(renderer.meshAsset));
		ImGui::Text("Skeleton: %s", SafeAssetPath(renderer.skeletonAsset));
		ImGui::Text("Skin: %s", SafeAssetPath(renderer.skinAsset));

		if (renderer.meshAsset)
		{
			ImGui::Text("Vertex Count: %u", renderer.meshAsset->vertexCount);
			ImGui::Text("Index Count: %u", renderer.meshAsset->indexCount);
			ImGui::Text("SubMesh Count: %u", static_cast<EZ::u32>(renderer.meshAsset->subMeshes.size()));
		}

		ImGui::Separator();
		ImGui::Text("Material Count: %u", static_cast<EZ::u32>(renderer.materialAssets.size()));

		for (EZ::u32 i = 0; i < static_cast<EZ::u32>(renderer.materialAssets.size()); ++i)
		{
			ImGui::BulletText("Material[%u]: %s", i, SafeAssetPath(renderer.materialAssets[i]));
		}
	}

	static void DrawScriptSection(BaseProtocol::ScriptComponent& scripts)
	{
		if (scripts.scripts.empty())
		{
			ImGui::TextDisabled("No scripts.");
			return;
		}

		for (EZ::u32 i = 0; i < static_cast<EZ::u32>(scripts.scripts.size()); ++i)
		{
			auto& script = scripts.scripts[i];

			ImGui::PushID(static_cast<int>(i));

			const bool open = ImGui::TreeNodeEx(
				"ScriptEntry",
				ImGuiTreeNodeFlags_DefaultOpen,
				"%s",
				script.scriptName.empty() ? "<unnamed script>" : script.scriptName.c_str());

			if (open)
			{
				ImGui::Checkbox("Enabled", &script.enabled);
				ImGui::Text("Awoken: %s", script.awoken ? "true" : "false");
				ImGui::Text("Started: %s", script.started ? "true" : "false");
				ImGui::Text("Was Enabled: %s", script.wasEnabled ? "true" : "false");
				ImGui::Text("Destroy Requested: %s", script.destroyRequested ? "true" : "false");

				if (!script.destroyRequested)
				{
					if (ImGui::Button("Request Destroy"))
					{
						script.destroyRequested = true;
					}
				}
				else
				{
					ImGui::TextDisabled("Pending destroy.");
				}

				ImGui::TreePop();
			}

			ImGui::PopID();
		}
	}

	static void DrawAnimatorSection(BaseProtocol::AnimatorComponent& animator)
	{
		ImGui::Checkbox("Playing", &animator.playing);
		ImGui::Checkbox("Looping", &animator.looping);
		ImGui::Checkbox("Apply Scale Keys", &animator.applyScaleKeys);
		ImGui::Checkbox("Apply Bind Pose When No Clip", &animator.applyBindPoseWhenNoClip);

		ImGui::Separator();

		ImGui::InputFloat("Play Rate", &animator.playRate, 0.01f, 0.1f, "%.3f");
		ImGui::InputFloat("State Speed", &animator.stateSpeed, 0.01f, 0.1f, "%.3f");
		ImGui::InputFloat("Current Time", &animator.currentTime, 0.01f, 0.1f, "%.3f");
		ImGui::InputFloat("State Elapsed Time", &animator.stateElapsedTime, 0.01f, 0.1f, "%.3f");

		ImGui::Separator();

		ImGui::Text("Current State: %s",
			animator.currentStateName.empty() ? "<none>" : animator.currentStateName.c_str());

		ImGui::Text("Current Clip: %s",
			(animator.currentClip && !animator.currentClip->sourcePath.empty())
			? animator.currentClip->sourcePath.c_str()
			: (animator.currentClip ? "<clip loaded>" : "<none>"));

		ImGui::Text("Controller: %s",
			(animator.controller && !animator.controller->name.empty())
			? animator.controller->name.c_str()
			: (animator.controller ? "<controller loaded>" : "<none>"));

		ImGui::Separator();

		ImGui::Checkbox("Apply Root Motion", &animator.applyRootMotion);
		ImGui::Checkbox("Root Motion XZ Only", &animator.rootMotionXZOnly);
		ImGui::Checkbox("Root Motion Ignore Y", &animator.rootMotionIgnoreY);
		ImGui::Checkbox("Root Motion Yaw Only", &animator.rootMotionYawOnly);

		ImGui::Text("Has Root Motion: %s", animator.hasRootMotion ? "true" : "false");
		ImGui::InputScalar("Root Motion Bone Index", ImGuiDataType_U32, &animator.rootMotionBoneIndex);

		if (animator.hasRootMotion)
		{
			ImGui::Separator();
			ImGui::Text("Extracted Root Motion");

			float deltaPos[3] =
			{
				animator.extractedRootMotion.deltaPosition.x,
				animator.extractedRootMotion.deltaPosition.y,
				animator.extractedRootMotion.deltaPosition.z
			};

			float deltaRot[4] =
			{
				animator.extractedRootMotion.deltaRotation.x,
				animator.extractedRootMotion.deltaRotation.y,
				animator.extractedRootMotion.deltaRotation.z,
				animator.extractedRootMotion.deltaRotation.w
			};

			ImGui::InputFloat3("Delta Position", deltaPos, "%.4f", ImGuiInputTextFlags_ReadOnly);
			ImGui::InputFloat4("Delta Rotation", deltaRot, "%.4f", ImGuiInputTextFlags_ReadOnly);
		}

		if (animator.controller)
		{
			ImGui::Separator();
			ImGui::Text("Parameters");

			for (const auto& desc : animator.controller->parameters)
			{
				ImGui::PushID(desc.name.c_str());

				switch (desc.type)
				{
				case DataProtocol::AnimatorParameterType::Float:
				{
					float value = animator.GetFloat(desc.name, desc.defaultFloat);
					if (ImGui::InputFloat(desc.name.c_str(), &value, 0.01f, 0.1f, "%.3f"))
					{
						animator.SetFloat(desc.name, value);
					}
					break;
				}
				case DataProtocol::AnimatorParameterType::Int:
				{
					int value = animator.GetInteger(desc.name, desc.defaultInt);
					if (ImGui::InputInt(desc.name.c_str(), &value))
					{
						animator.SetInteger(desc.name, value);
					}
					break;
				}
				case DataProtocol::AnimatorParameterType::Bool:
				{
					bool value = animator.GetBool(desc.name, desc.defaultBool);
					if (ImGui::Checkbox(desc.name.c_str(), &value))
					{
						animator.SetBool(desc.name, value);
					}
					break;
				}
				case DataProtocol::AnimatorParameterType::Trigger:
				{
					const bool pending = animator.HasTrigger(desc.name);
					ImGui::Text("%s", desc.name.c_str());
					ImGui::SameLine();

					if (ImGui::Button("Set Trigger"))
					{
						animator.SetTrigger(desc.name);
					}

					ImGui::SameLine();
					ImGui::TextDisabled("%s", pending ? "(pending)" : "(idle)");
					break;
				}
				default:
					ImGui::TextDisabled("%s <unknown>", desc.name.c_str());
					break;
				}

				ImGui::PopID();
			}
		}
	}

	static void DrawLightSection(BaseProtocol::LightComponent& light)
	{
		const char* lightTypeNames[] =
		{
			"Directional",
			"Point",
			"Spot"
		};

		int currentType = static_cast<int>(light.type);
		if (currentType < 0) currentType = 0;
		if (currentType > 2) currentType = 2;

		if (ImGui::Combo("Type", &currentType, lightTypeNames, IM_ARRAYSIZE(lightTypeNames)))
		{
			light.type = static_cast<BaseProtocol::LightType>(currentType);
		}

		float color[3] =
		{
			light.color.x,
			light.color.y,
			light.color.z
		};
		if (ImGui::ColorEdit3("Color", color))
		{
			light.color = { color[0], color[1], color[2] };
		}

		ImGui::InputFloat("Intensity", &light.intensity, 0.05f, 0.2f, "%.3f");
		ImGui::Checkbox("Cast Shadow", &light.castShadow);

		if (light.type == BaseProtocol::LightType::Point ||
			light.type == BaseProtocol::LightType::Spot)
		{
			ImGui::InputFloat("Range", &light.range, 0.1f, 1.0f, "%.3f");
		}

		if (light.type == BaseProtocol::LightType::Spot)
		{
			ImGui::InputFloat("Spot Angle", &light.spotAngleDegrees, 0.5f, 2.0f, "%.3f");
		}

		if (light.castShadow)
		{
			ImGui::Separator();
			ImGui::Text("Shadow Bias");

			ImGui::InputFloat("Depth Bias", &light.depthBias, 0.01f, 0.1f, "%.3f");
			ImGui::InputFloat("Normal Bias", &light.normalBias, 0.01f, 0.1f, "%.3f");
			ImGui::InputFloat("Slope Scale Depth Bias", &light.slopeScaleDepthBias, 0.01f, 0.1f, "%.3f");
		}
	}

	static void DrawOtherComponentsSection(
		ControlProtocol::EntityManager& entityManager,
		EZ::Entity entity)
	{
		bool any = false;

		if (entityManager.HasComponent<BaseProtocol::CameraComponent>(entity))
		{
			ImGui::BulletText("CameraComponent");
			any = true;
		}

		if (entityManager.HasComponent<BaseProtocol::BonePoseComponent>(entity))
		{
			ImGui::BulletText("BonePoseComponent");
			any = true;
		}

		if (entityManager.HasComponent<BaseProtocol::SkinningPaletteComponent>(entity))
		{
			ImGui::BulletText("SkinningPaletteComponent");
			any = true;
		}

		if (!any)
		{
			ImGui::TextDisabled("No extra visible components.");
		}
	}
}

namespace Tool
{
	void InspectorPanel::Draw(EZ::WorldContext& world)
	{
		auto* editor = world.TryGet<Tool::EditorContext>();
		auto* entityManager = world.TryGet<ControlProtocol::EntityManager>();
		auto* transformManager = world.TryGet<ControlProtocol::TransformManager>();

		if (!editor || !entityManager || !transformManager)
		{
			return;
		}

		if (!ImGui::Begin("Inspector", &editor->showInspector))
		{
			ImGui::End();
			return;
		}

		if (!editor->hasSelection || !entityManager->IsValid(editor->selectedEntity))
		{
			ImGui::TextDisabled("No entity selected.");
			ImGui::End();
			return;
		}

		const EZ::Entity entity = editor->selectedEntity;
		ImGui::Text("Entity: %u", static_cast<EZ::u32>(entity));
		ImGui::Separator();

		if (ImGui::CollapsingHeader("LocalTransform", ImGuiTreeNodeFlags_DefaultOpen))
		{
			DrawLocalTransformSection(*entityManager, *transformManager, entity);
		}

		if (auto* renderer = entityManager->TryGetComponent<BaseProtocol::MeshRendererComponent>(entity))
		{
			if (ImGui::CollapsingHeader("Render Component", ImGuiTreeNodeFlags_DefaultOpen))
			{
				DrawMeshRendererSection(*renderer);
			}
		}

		if (auto* skinnedRenderer = entityManager->TryGetComponent<BaseProtocol::SkinnedMeshRendererComponent>(entity))
		{
			if (ImGui::CollapsingHeader("Render Component (Skinned)", ImGuiTreeNodeFlags_DefaultOpen))
			{
				DrawSkinnedMeshRendererSection(*skinnedRenderer);
			}
		}

		if (auto* animator = entityManager->TryGetComponent<BaseProtocol::AnimatorComponent>(entity))
		{
			if (ImGui::CollapsingHeader("Animator Component", ImGuiTreeNodeFlags_DefaultOpen))
			{
				DrawAnimatorSection(*animator);
			}
		}

		if (auto* light = entityManager->TryGetComponent<BaseProtocol::LightComponent>(entity))
		{
			if (ImGui::CollapsingHeader("Light Component", ImGuiTreeNodeFlags_DefaultOpen))
			{
				DrawLightSection(*light);
			}
		}

		if (auto* scripts = entityManager->TryGetComponent<BaseProtocol::ScriptComponent>(entity))
		{
			if (ImGui::CollapsingHeader("Script Component", ImGuiTreeNodeFlags_DefaultOpen))
			{
				DrawScriptSection(*scripts);
			}
		}

		if (ImGui::CollapsingHeader("Other Components"))
		{
			DrawOtherComponentsSection(*entityManager, entity);
		}

		ImGui::End();
	}
}