#include "WorldRunTime.h"

#include "core/Engine/IEngine.h"
#include "core/ProjectManager/IProject.h"
#include "core/Render/RenderTypes.h"

#include "DataProtocol/WindowData.h"
#include "System/AnimatorSystem/AnimatorSystem.h"
#include "System/InputSystem/InputSystem.h"
#include "System/PhysicsSystem/PhysicsSystem.h"
#include "System/RenderSystem/RenderSystem.h"
#include "System/ScriptSystem/ScriptSystem.h"
#include "System/TransformSystem/TransformSystem.h"

WorldRuntime::WorldRuntime()
	: m_TransformManager(m_EntityManager)
{
}

WorldRuntime::~WorldRuntime() = default;

int WorldRuntime::Initialize(EZ::ProjectContext& project)
{
	m_Project = &project;
	m_FixedTimeAccumulator = 0.0f;
	m_AnimationManager.Reset();
	m_CharacterAssetManager.Clear();
	m_RenderImageBuffer.ClearAll();
	m_RenderPipelineController.ClearFeatures();

	m_World.Register(project);
	m_World.Register(m_EntityManager);
	m_World.Register(m_TransformManager);
	m_World.Register(m_InputController);
	m_World.Register(m_AnimationManager);
	m_World.Register(m_CharacterAssetManager);
	//m_World.Register(m_GLRenderDeviceController);
	m_World.RegisterAs<ControlProtocol::RenderDeviceController>(m_GLRenderDeviceController);
	m_World.Register(m_RenderPipelineController);
	m_World.Register(m_RenderImageBuffer);
	m_World.Register(m_EditorContext);

	m_World.Register(m_SystemManager);
	m_World.Register(m_ScriptRegistry);
	m_World.Register(m_SceneManager);

	DataProtocol::WindowDesc desc;
	desc.title = "EZEngine";
	desc.size = { 1600, 900 };
	desc.backendHint = DataProtocol::WindowBackendHint::OpenGL;

	if (project.engine)
	{
		m_World.Register(*project.engine);
		project.engine->SetupMainWindowDesc(desc);
	}

	auto* mainWindow = m_World.CreateWindow(desc);
	if (!mainWindow)
	{
		return -1;
	}

	if (!m_GLRenderDeviceController.Initialize(*mainWindow))
	{
		return -1;
	}

	if (!m_ImGuiLayer.Initialize(m_World))
	{
		return -1;
	}

	m_World.Register(m_ImGuiLayer);
	m_World.Register(m_WorldHierarchyPanel);
	m_World.Register(m_InspectorPanel);

	m_SystemManager.AddSystem<InputSystem>();
	m_SystemManager.AddSystem<ScriptSystem>();
	m_SystemManager.AddSystem<AnimatorSystem>();
	m_SystemManager.AddSystem<PhysicsSystem>();

	m_TransformSystem = m_SystemManager.AddSystem<TransformSystem>(m_TransformManager);
	if (m_TransformSystem)
	{
		m_World.Register(*m_TransformSystem);
	}

	m_RenderSystem = m_SystemManager.AddSystem<RenderSystem>();
	if (m_RenderSystem)
	{
		m_World.Register(*m_RenderSystem);
	}

	m_RenderPipelineController.AddFeature(m_ForwardOpaqueFeature);
	m_RenderPipelineController.AddFeature(m_EditorOverlayFeature);
	m_RenderPipelineController.AddFeature(m_PresentFeature);

	if (project.engine)
	{
		project.engine->OnWorldContextCreated(project, m_World);
	}

	if (project.project)
	{
		m_World.Register(*project.project);
		project.project->OnWorldCreated(m_World);
	}

	return m_SystemManager.InitializeAll(project, m_World);
}

void WorldRuntime::Shutdown()
{
	if (m_Project)
	{
		m_SystemManager.ShutdownAll(*m_Project, m_World);
	}

	if (m_Project && m_Project->project)
	{
		m_Project->project->OnWorldDestroyed(m_World);
	}

	if (m_Project && m_Project->engine)
	{
		m_Project->engine->OnWorldContextDestroyed(*m_Project, m_World);
	}

	auto destroySlot = [&](EZ::RenderImageTag tag)
		{
			if (auto* slot = m_RenderImageBuffer.TryGet(tag))
			{
				m_GLRenderDeviceController.DestroyRenderTarget(*slot);
				m_GLRenderDeviceController.DestroyImage(*slot);
			}
		};

	destroySlot(EZ::RenderImageTag::ForwardOpaque);
	destroySlot(EZ::RenderImageTag::SceneDepth);
	destroySlot(EZ::RenderImageTag::FinalColor);

	m_RenderPipelineController.ClearFeatures();
	m_RenderImageBuffer.ClearAll();

	m_ImGuiLayer.Shutdown();
	m_GLRenderDeviceController.Shutdown();
	m_AnimationManager.Reset();
	m_CharacterAssetManager.Clear();
	m_SceneManager.ClearRegisteredScenes();

	m_RenderSystem = nullptr;
	m_TransformSystem = nullptr;
	m_FixedTimeAccumulator = 0.0f;
	m_World.Clear();
	m_Project = nullptr;
}

void WorldRuntime::Tick(EZ::ProjectContext& project)
{
	m_SystemManager.BeginFrameAll(project, m_World, project.deltaTime);
	m_SystemManager.UpdateAll(project, m_World, project.deltaTime);

	const float fixedDeltaTime = (project.fixedDeltaTime > 0.0f)
		? project.fixedDeltaTime
		: (1.0f / 60.0f);

	m_FixedTimeAccumulator += project.deltaTime;

	int fixedStepCount = 0;
	constexpr int kMaxFixedStepsPerFrame = 4;

	while (m_FixedTimeAccumulator >= fixedDeltaTime)
	{
		m_SystemManager.FixedUpdateAll(project, m_World, fixedDeltaTime);
		m_FixedTimeAccumulator -= fixedDeltaTime;

		++fixedStepCount;
		if (fixedStepCount >= kMaxFixedStepsPerFrame)
		{
			m_FixedTimeAccumulator = 0.0f;
			break;
		}
	}

	m_SystemManager.LateUpdateAll(project, m_World, project.deltaTime);
	m_SystemManager.EndFrameAll(project, m_World, project.deltaTime);

	if (m_RenderPipelineController.BeginFrame(
		project,
		m_World,
		m_RenderImageBuffer,
		m_GLRenderDeviceController))
	{
		m_RenderPipelineController.ExecuteFeatures(
			project,
			m_World,
			m_RenderImageBuffer,
			m_GLRenderDeviceController);

		m_RenderPipelineController.EndFrame(
			project,
			m_World,
			m_RenderImageBuffer,
			m_GLRenderDeviceController);
	}
}