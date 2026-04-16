#include "RuntimeContext.h"

#include <memory>

#include "core/Engine/IEngine.h"
#include "core/Engine/IRuntimeContextConfigurator.h"
#include "core/ProjectManager/IProject.h"
#include "core/Render/RenderTypes.h"
#include "core/ScriptManager/ScriptRegistry.h"
#include "core/System/SystemManager.h"

#include "DataProtocol/WindowData.h"

#include "ControlProtocol/AnimationManager/AnimationManager.h"
#include "ControlProtocol/CharacterAssetManager/CharacterAssetManager.h"
#include "ControlProtocol/EntityManager/EntityManager.h"
#include "ControlProtocol/InputController/InputController.h"
#include "ControlProtocol/RenderDeviceController/OpenGL/GLRenderDeviceController.h"
#include "ControlProtocol/RenderPipelineController/RenderImageBuffer.h"
#include "ControlProtocol/RenderPipelineController/RenderPipelineController.h"
#include "ControlProtocol/TransformManager/TransformManager.h"

#include "Launcher/GameLauncher/Scene/SceneManager.h"

#include "System/AnimatorSystem/AnimatorSystem.h"
#include "System/InputSystem/InputSystem.h"
#include "System/PhysicsSystem/PhysicsSystem.h"
#include "System/RenderSystem/RenderFeatures/ForwardOpaqueRenderFeature.h"
#include "System/RenderSystem/RenderFeatures/PresentRenderFeature.h"
#include "System/RenderSystem/RenderSystem.h"
#include "System/ScriptSystem/ScriptSystem.h"
#include "System/TransformSystem/TransformSystem.h"

#include "Tool/Editor/EditorContext.h"
#include "Tool/Editor/Imgui/ImGuiLayer.h"
#include "Tool/Editor/Panels/InspectorPanel.h"
#include "Tool/Editor/Panels/PerformancePanel.h"
#include "Tool/Editor/Panels/SceneControlPanel.h"
#include "Tool/Editor/Panels/WorldHierarchyPanel.h"
#include "Tool/Editor/RenderFeature/EditorOverlayRenderFeature.h"

namespace EZ
{
	struct RuntimeContext::Impl
	{
		ProjectContext* project = nullptr;
		IRuntimeContextConfigurator* configurator = nullptr;

		ControlProtocol::EntityManager entityManager;
		ControlProtocol::TransformManager transformManager;
		ControlProtocol::InputController inputController;
		ControlProtocol::AnimationManager animationManager;
		ControlProtocol::CharacterAssetManager characterAssetManager;
		std::unique_ptr<ControlProtocol::RenderDeviceController> renderDevice;

		ControlProtocol::RenderPipelineController renderPipelineController;
		ControlProtocol::RenderImageBuffer renderImageBuffer;

		SystemManager systemManager;
		TransformSystem* transformSystem = nullptr;
		RenderSystem* renderSystem = nullptr;

		ScriptRegistry scriptRegistry;
		float fixedTimeAccumulator = 0.0f;

		ForwardOpaqueRenderFeature forwardOpaqueFeature;
		PresentRenderFeature presentFeature;

		Tool::EditorContext editorContext;
		Tool::ImGuiLayer imguiLayer;
		Tool::WorldHierarchyPanel worldHierarchyPanel;
		Tool::InspectorPanel inspectorPanel;
		Tool::SceneControlPanel sceneControlPanel;
		Tool::PerformancePanel performancePanel;
		Tool::EditorOverlayRenderFeature editorOverlayFeature;

		TemplateWorldContext templateWorldContext;
		ProjectExtensionContext projectExtension;
		WorldContext world;

		GameScene::SceneManager sceneManager;

		Impl()
			: transformManager(entityManager)
		{
		}

		void RebuildTemplateWorldContext()
		{
			templateWorldContext = {};
			templateWorldContext.projectContext = project;
			templateWorldContext.world = &world;
			templateWorldContext.projectExtension = &projectExtension;

			templateWorldContext.systemManager = &systemManager;
			templateWorldContext.scriptRegistry = &scriptRegistry;

			templateWorldContext.entityManager = &entityManager;
			templateWorldContext.transformManager = &transformManager;
			templateWorldContext.inputController = &inputController;
			templateWorldContext.animationManager = &animationManager;
			templateWorldContext.characterAssetManager = &characterAssetManager;

			templateWorldContext.mainWindow = world.GetPrimaryWindow();
			templateWorldContext.renderDevice = renderDevice.get();
			templateWorldContext.renderPipelineController = &renderPipelineController;
			templateWorldContext.renderImageBuffer = &renderImageBuffer;

			templateWorldContext.editorContext = &editorContext;
			templateWorldContext.imguiLayer = &imguiLayer;

			templateWorldContext.sceneManager = &sceneManager;

			templateWorldContext.transformSystem = transformSystem;
			templateWorldContext.renderSystem = renderSystem;
		}
	};

	RuntimeContext::RuntimeContext()
		: m_Impl(std::make_unique<Impl>())
	{
	}

	RuntimeContext::~RuntimeContext() = default;

	int RuntimeContext::Initialize(ProjectContext& project, IRuntimeContextConfigurator* configurator)
	{
		auto& impl = *m_Impl;

		impl.project = &project;
		impl.configurator = configurator;
		impl.fixedTimeAccumulator = 0.0f;

		impl.animationManager.Reset();
		impl.characterAssetManager.Clear();
		impl.scriptRegistry.Clear();
		impl.sceneManager.ClearRegisteredScenes();
		impl.renderImageBuffer.ClearAll();
		impl.renderPipelineController.ClearFeatures();
		impl.systemManager.Clear();
		impl.projectExtension.Clear();
		impl.templateWorldContext = {};
		impl.world.Clear();
		impl.renderDevice.reset();
		impl.transformSystem = nullptr;
		impl.renderSystem = nullptr;

		if (configurator)
		{
			configurator->ConfigureProjectExtension(impl.projectExtension);
		}

		impl.world.Register(project);
		impl.world.Register(impl.entityManager);
		impl.world.Register(impl.transformManager);
		impl.world.Register(impl.inputController);
		impl.world.Register(impl.animationManager);
		impl.world.Register(impl.characterAssetManager);
		impl.world.Register(impl.renderPipelineController);
		impl.world.Register(impl.renderImageBuffer);
		impl.world.Register(impl.editorContext);

		impl.world.Register(impl.systemManager);
		impl.world.Register(impl.scriptRegistry);
		impl.world.Register(impl.sceneManager);

		impl.world.Register(impl.templateWorldContext);
		impl.world.Register(impl.projectExtension);

		DataProtocol::WindowDesc desc;
		desc.title = "EZEngine";
		desc.size = { 1600, 900 };
		desc.backendHint = DataProtocol::WindowBackendHint::OpenGL;

		if (project.engine)
		{
			impl.world.Register(*project.engine);
			project.engine->SetupMainWindowDesc(desc);
		}

		auto* mainWindow = impl.world.CreateWindow(desc);
		if (!mainWindow)
		{
			return -1;
		}

		impl.renderDevice = std::make_unique<ControlProtocol::GLRenderDeviceController>();
		if (!impl.renderDevice->Initialize(*mainWindow))
		{
			return -1;
		}

		impl.world.RegisterAs<ControlProtocol::RenderDeviceController>(*impl.renderDevice);

		if (!impl.imguiLayer.Initialize(impl.world))
		{
			return -1;
		}

		impl.world.Register(impl.imguiLayer);
		impl.world.Register(impl.worldHierarchyPanel);
		impl.world.Register(impl.inspectorPanel);
		impl.world.Register(impl.sceneControlPanel);
		impl.world.Register(impl.performancePanel);

		impl.systemManager.AddSystem<InputSystem>();
		impl.systemManager.AddSystem<ScriptSystem>();
		impl.systemManager.AddSystem<AnimatorSystem>();
		impl.systemManager.AddSystem<PhysicsSystem>();

		impl.transformSystem = impl.systemManager.AddSystem<TransformSystem>(impl.entityManager);
		if (impl.transformSystem)
		{
			impl.world.Register(*impl.transformSystem);
		}

		impl.renderSystem = impl.systemManager.AddSystem<RenderSystem>();
		if (impl.renderSystem)
		{
			impl.world.Register(*impl.renderSystem);
		}

		impl.renderPipelineController.AddFeature(impl.forwardOpaqueFeature);
		impl.renderPipelineController.AddFeature(impl.editorOverlayFeature);
		impl.renderPipelineController.AddFeature(impl.presentFeature);

		impl.RebuildTemplateWorldContext();

		if (configurator)
		{
			configurator->RegisterProjectControllers(*this);
			configurator->RegisterProjectSystems(*this);
			configurator->InstallProjectTools(*this);
			impl.RebuildTemplateWorldContext();
		}

		if (project.engine)
		{
			project.engine->OnWorldContextCreated(project, impl.world);
		}

		if (project.project)
		{
			impl.world.Register(*project.project);
			project.project->OnWorldCreated(impl.world);
		}

		const int ret = impl.systemManager.InitializeAll(project, impl.world);
		if (ret != 0)
		{
			return ret;
		}

		if (configurator)
		{
			configurator->OnRuntimeCreated(*this);
		}

		return 0;
	}

	void RuntimeContext::Shutdown()
	{
		auto& impl = *m_Impl;

		if (impl.project)
		{
			impl.systemManager.ShutdownAll(*impl.project, impl.world);
		}

		if (impl.configurator)
		{
			impl.configurator->OnRuntimeDestroyed(*this);
		}

		if (impl.project && impl.project->project)
		{
			impl.project->project->OnWorldDestroyed(impl.world);
		}

		if (impl.project && impl.project->engine)
		{
			impl.project->engine->OnWorldContextDestroyed(*impl.project, impl.world);
		}

		auto destroySlot = [&](EZ::RenderImageTag tag)
			{
				if (!impl.renderDevice)
				{
					return;
				}

				if (auto* slot = impl.renderImageBuffer.TryGet(tag))
				{
					impl.renderDevice->DestroyRenderTarget(*slot);
					impl.renderDevice->DestroyImage(*slot);
				}
			};

		destroySlot(EZ::RenderImageTag::ForwardOpaque);
		destroySlot(EZ::RenderImageTag::SceneDepth);
		destroySlot(EZ::RenderImageTag::FinalColor);

		impl.renderPipelineController.ClearFeatures();
		impl.renderImageBuffer.ClearAll();

		impl.imguiLayer.Shutdown();

		if (impl.renderDevice)
		{
			impl.renderDevice->Shutdown();
			impl.renderDevice.reset();
		}

		impl.animationManager.Reset();
		impl.characterAssetManager.Clear();
		impl.scriptRegistry.Clear();
		impl.sceneManager.ClearRegisteredScenes();
		impl.systemManager.Clear();

		impl.renderSystem = nullptr;
		impl.transformSystem = nullptr;
		impl.fixedTimeAccumulator = 0.0f;

		impl.projectExtension.Clear();
		impl.templateWorldContext = {};

		impl.world.Clear();

		impl.project = nullptr;
		impl.configurator = nullptr;
	}

	void RuntimeContext::Tick(ProjectContext& project)
	{
		auto& impl = *m_Impl;

		impl.systemManager.BeginFrameAll(project, impl.world, project.deltaTime);
		impl.systemManager.UpdateAll(project, impl.world, project.deltaTime);

		const float fixedDeltaTime = (project.fixedDeltaTime > 0.0f)
			? project.fixedDeltaTime
			: (1.0f / 60.0f);

		constexpr int kMaxFixedStepsPerFrame = 4;

		impl.fixedTimeAccumulator += project.deltaTime;
		const float maxAccumulator = fixedDeltaTime * static_cast<float>(kMaxFixedStepsPerFrame);
		if (impl.fixedTimeAccumulator > maxAccumulator)
		{
			impl.fixedTimeAccumulator = maxAccumulator;
		}

		int fixedStepCount = 0;
		while (impl.fixedTimeAccumulator >= fixedDeltaTime && fixedStepCount < kMaxFixedStepsPerFrame)
		{
			impl.systemManager.FixedUpdateAll(project, impl.world, fixedDeltaTime);
			impl.fixedTimeAccumulator -= fixedDeltaTime;
			++fixedStepCount;
		}

		impl.systemManager.LateUpdateAll(project, impl.world, project.deltaTime);
		impl.systemManager.EndFrameAll(project, impl.world, project.deltaTime);

		if (!impl.renderDevice)
		{
			return;
		}

		if (impl.renderPipelineController.BeginFrame(
			project,
			impl.world,
			impl.renderImageBuffer,
			*impl.renderDevice))
		{
			impl.renderPipelineController.ExecuteFeatures(
				project,
				impl.world,
				impl.renderImageBuffer,
				*impl.renderDevice);

			impl.renderPipelineController.EndFrame(
				project,
				impl.world,
				impl.renderImageBuffer,
				*impl.renderDevice);
		}
	}

	WorldContext& RuntimeContext::GetWorldContext()
	{
		return m_Impl->world;
	}

	const WorldContext& RuntimeContext::GetWorldContext() const
	{
		return m_Impl->world;
	}

	TemplateWorldContext& RuntimeContext::GetTemplateWorldContext()
	{
		return m_Impl->templateWorldContext;
	}

	const TemplateWorldContext& RuntimeContext::GetTemplateWorldContext() const
	{
		return m_Impl->templateWorldContext;
	}

	ProjectExtensionContext& RuntimeContext::GetProjectExtensionContext()
	{
		return m_Impl->projectExtension;
	}

	const ProjectExtensionContext& RuntimeContext::GetProjectExtensionContext() const
	{
		return m_Impl->projectExtension;
	}

	SystemManager& RuntimeContext::GetSystemManager()
	{
		return m_Impl->systemManager;
	}

	const SystemManager& RuntimeContext::GetSystemManager() const
	{
		return m_Impl->systemManager;
	}
}