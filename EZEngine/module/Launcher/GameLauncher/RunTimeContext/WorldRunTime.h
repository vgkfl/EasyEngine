#pragma once
#ifndef __WORLD_RUNTIME_H__
#define __WORLD_RUNTIME_H__

#include "core/Context/RunTimeContext/ProjectContext.h"
#include "core/Context/RunTimeContext/WorldContext.h"
#include "core/System/SystemManager.h"
#include "core/ScriptManager/ScriptRegistry.h"

#include "ControlProtocol/AnimationManager/AnimationManager.h"
#include "ControlProtocol/CharacterAssetManager/CharacterAssetManager.h"
#include "ControlProtocol/EntityManager/EntityManager.h"
#include "ControlProtocol/InputController/InputController.h"
#include "ControlProtocol/RenderDeviceController/OpenGL/GLRenderDeviceController.h"
#include "ControlProtocol/RenderPipelineController/RenderImageBuffer.h"
#include "ControlProtocol/RenderPipelineController/RenderPipelineController.h"
#include "ControlProtocol/TransformManager/TransformManager.h"

#include "System/RenderSystem/RenderFeatures/ForwardOpaqueRenderFeature.h"
#include "System/RenderSystem/RenderFeatures/PresentRenderFeature.h"
#include "System/TransformSystem/TransformSystem.h"

#include "Tool/Editor/EditorContext.h"
#include "Tool/Editor/ImGui/ImGuiLayer.h"
#include "Tool/Editor/Panels/InspectorPanel.h"
#include "Tool/Editor/Panels/WorldHierarchyPanel.h"
#include "Tool/Editor/RenderFeature/EditorOverlayRenderFeature.h"
#include "Launcher/GameLauncher/Scene/SceneManager.h"

class RenderSystem;

class WorldRuntime
{
public:
	WorldRuntime();
	~WorldRuntime();

	int Initialize(EZ::ProjectContext& project);
	void Shutdown();

	void Tick(EZ::ProjectContext& project);

	EZ::WorldContext& GetWorldContext() { return m_World; }
	const EZ::WorldContext& GetWorldContext() const { return m_World; }

private:
	EZ::ProjectContext* m_Project = nullptr;

	ControlProtocol::EntityManager m_EntityManager;
	ControlProtocol::TransformManager m_TransformManager;
	ControlProtocol::InputController m_InputController;
	ControlProtocol::AnimationManager m_AnimationManager;
	ControlProtocol::CharacterAssetManager m_CharacterAssetManager;
	ControlProtocol::GLRenderDeviceController m_GLRenderDeviceController;

	ControlProtocol::RenderPipelineController m_RenderPipelineController;
	ControlProtocol::RenderImageBuffer m_RenderImageBuffer;

	EZ::SystemManager m_SystemManager;
	TransformSystem* m_TransformSystem = nullptr;
	RenderSystem* m_RenderSystem = nullptr;

	EZ::ScriptRegistry m_ScriptRegistry;
	float m_FixedTimeAccumulator = 0.0f;

	ForwardOpaqueRenderFeature m_ForwardOpaqueFeature;
	PresentRenderFeature m_PresentFeature;

	Tool::EditorContext m_EditorContext;
	Tool::ImGuiLayer m_ImGuiLayer;
	Tool::WorldHierarchyPanel m_WorldHierarchyPanel;
	Tool::InspectorPanel m_InspectorPanel;

	Tool::EditorOverlayRenderFeature m_EditorOverlayFeature;

	EZ::WorldContext m_World;

	GameScene::SceneManager m_SceneManager;
};

#endif