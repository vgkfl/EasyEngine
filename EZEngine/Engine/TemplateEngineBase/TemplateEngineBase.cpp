#include "TemplateEngineBase.h"

#include <memory>

#include "core/Engine/IRuntimeContextConfigurator.h"
#include "core/LauncherManager/LauncherManager.h"
#include "core/ProjectManager/IProject.h"

#include "RuntimeContext.h"

namespace
{
	class TemplateEngineRuntimeAdapter final : public EZ::IEngineRuntime
	{
	public:
		explicit TemplateEngineRuntimeAdapter(EZ::IRuntimeContextConfigurator* configurator)
			: m_Configurator(configurator)
		{
		}

		int Initialize(EZ::ProjectContext& project) override
		{
			return m_Runtime.Initialize(project, m_Configurator);
		}

		bool Tick(EZ::ProjectContext& project) override
		{
			auto& world = m_Runtime.GetWorldContext();
			if (!world.HasOpenWindow())
			{
				return false;
			}

			world.PumpWindowEvents();
			m_Runtime.Tick(project);
			return world.HasOpenWindow();
		}

		int Shutdown(EZ::ProjectContext& project) override
		{
			(void)project;
			m_Runtime.Shutdown();
			return 0;
		}

		EZ::WorldContext& GetWorldContext() override
		{
			return m_Runtime.GetWorldContext();
		}

		const EZ::WorldContext& GetWorldContext() const override
		{
			return m_Runtime.GetWorldContext();
		}

	private:
		EZ::RuntimeContext m_Runtime;
		EZ::IRuntimeContextConfigurator* m_Configurator = nullptr;
	};

	class EngineProgramRunnerAdapter final : public EZ::IProgramRunner
	{
	public:
		EngineProgramRunnerAdapter(
			EZ::IEngineRuntime& runtime,
			EZ::ProjectContext& projectCtx)
			: m_Runtime(runtime)
			, m_ProjectContext(projectCtx)
			, m_Project(projectCtx.project)
		{
		}

		int Initialize() override
		{
			int ret = m_Runtime.Initialize(m_ProjectContext);
			if (ret != 0)
			{
				return ret;
			}
			m_RuntimeInitialized = true;

			if (m_Project)
			{
				ret = m_Project->OnBeforeMainLoop(m_ProjectContext);
				if (ret != 0)
				{
					m_Runtime.Shutdown(m_ProjectContext);
					m_RuntimeInitialized = false;
					return ret;
				}
				m_MainLoopEntered = true;
			}

			return 0;
		}

		bool TickFrame(float deltaTime) override
		{
			if (deltaTime < 0.0f)
			{
				deltaTime = 0.0f;
			}
			else if (deltaTime > 0.25f)
			{
				deltaTime = 0.25f;
			}

			m_ProjectContext.deltaTime = deltaTime;
			return m_Runtime.Tick(m_ProjectContext);
		}

		int Shutdown() override
		{
			if (m_MainLoopEntered && m_Project)
			{
				m_Project->OnAfterMainLoop(m_ProjectContext);
				m_MainLoopEntered = false;
			}

			if (m_RuntimeInitialized)
			{
				m_Runtime.Shutdown(m_ProjectContext);
				m_RuntimeInitialized = false;
			}

			return 0;
		}

	private:
		EZ::IEngineRuntime& m_Runtime;
		EZ::ProjectContext& m_ProjectContext;
		EZ::IProject* m_Project = nullptr;
		bool m_RuntimeInitialized = false;
		bool m_MainLoopEntered = false;
	};
}

namespace EZ
{
	int TemplateEngineBase::Start(IProject* project)
	{
		if (m_IsRunning)
		{
			return -1;
		}

		ProjectContext projectCtx;
		projectCtx.engine = this;
		projectCtx.project = project;

		int ret = OnAttachProjectContext(projectCtx);
		if (ret != 0)
		{
			return ret;
		}

		bool enteredProgram = false;

		if (project)
		{
			ret = project->OnPreEngineInit(projectCtx);
			if (ret != 0)
			{
				OnDetachProjectContext(projectCtx);
				return ret;
			}

			ret = project->OnEngineInit(projectCtx);
			if (ret != 0)
			{
				OnDetachProjectContext(projectCtx);
				return ret;
			}

			ret = project->OnPostEngineInit(projectCtx);
			if (ret != 0)
			{
				OnDetachProjectContext(projectCtx);
				return ret;
			}
		}

		auto launcher = CreateLauncher();
		if (!launcher)
		{
			OnDetachProjectContext(projectCtx);
			return -1;
		}

		auto runtime = CreateRuntime(projectCtx);
		if (!runtime)
		{
			OnDetachProjectContext(projectCtx);
			return -1;
		}

		EngineProgramRunnerAdapter runner(*runtime, projectCtx);

		m_ActiveLauncher = launcher.get();
		m_IsRunning = true;
		enteredProgram = true;

		ret = launcher->Start(runner);

		m_IsRunning = false;
		m_ActiveLauncher = nullptr;

		if (enteredProgram && project)
		{
			project->OnBeforeEngineShutdown(projectCtx);
			project->OnEngineShutdown(projectCtx);
		}

		OnDetachProjectContext(projectCtx);
		return ret;
	}

	void TemplateEngineBase::RequestStop()
	{
		if (m_ActiveLauncher)
		{
			m_ActiveLauncher->RequestQuit();
		}
	}

	bool TemplateEngineBase::IsRunning() const
	{
		return m_IsRunning;
	}

	std::unique_ptr<IEngineRuntime> TemplateEngineBase::CreateRuntime(ProjectContext& ctx)
	{
		return std::make_unique<TemplateEngineRuntimeAdapter>(
			ResolveRuntimeConfigurator(ctx));
	}

	std::unique_ptr<ILauncher> TemplateEngineBase::CreateLauncher() const
	{
		return LauncherManager::Get().Create(GetDefaultLauncherId());
	}

	IRuntimeContextConfigurator* TemplateEngineBase::ResolveRuntimeConfigurator(ProjectContext& ctx) const
	{
		if (!ctx.project)
		{
			return nullptr;
		}

		return static_cast<IRuntimeContextConfigurator*>(ctx.project);
	}
}