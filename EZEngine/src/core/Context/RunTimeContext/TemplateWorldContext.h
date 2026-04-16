#pragma once
#ifndef __TEMPLATE_WORLD_CONTEXT_H__
#define __TEMPLATE_WORLD_CONTEXT_H__

namespace ControlProtocol
{
	class EntityManager;
	class TransformManager;
	class InputController;
	class AnimationManager;
	class CharacterAssetManager;
	class WindowController;
	class RenderDeviceController;
	class RenderPipelineController;
	class RenderImageBuffer;
}

namespace Tool
{
	struct EditorContext;
	class ImGuiLayer;
}

namespace GameScene
{
	class SceneManager;
}

class TransformSystem;
class RenderSystem;

namespace EZ
{
	struct ProjectContext;
	struct WorldContext;
	class SystemManager;
	class ScriptRegistry;
	class ProjectExtensionContext;

	struct TemplateWorldContext
	{
		ProjectContext* projectContext = nullptr;
		WorldContext* world = nullptr;
		ProjectExtensionContext* projectExtension = nullptr;

		SystemManager* systemManager = nullptr;
		ScriptRegistry* scriptRegistry = nullptr;

		ControlProtocol::EntityManager* entityManager = nullptr;
		ControlProtocol::TransformManager* transformManager = nullptr;
		ControlProtocol::InputController* inputController = nullptr;
		ControlProtocol::AnimationManager* animationManager = nullptr;
		ControlProtocol::CharacterAssetManager* characterAssetManager = nullptr;

		ControlProtocol::WindowController* mainWindow = nullptr;
		ControlProtocol::RenderDeviceController* renderDevice = nullptr;
		ControlProtocol::RenderPipelineController* renderPipelineController = nullptr;
		ControlProtocol::RenderImageBuffer* renderImageBuffer = nullptr;

		Tool::EditorContext* editorContext = nullptr;
		Tool::ImGuiLayer* imguiLayer = nullptr;

		GameScene::SceneManager* sceneManager = nullptr;

		TransformSystem* transformSystem = nullptr;
		RenderSystem* renderSystem = nullptr;
	};
}

#endif