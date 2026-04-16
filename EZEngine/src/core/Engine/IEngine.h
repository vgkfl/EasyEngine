#pragma once
#ifndef __CORE_I_ENGINE_H__
#define __CORE_I_ENGINE_H__

#include <memory>
#include <string>

#include "core/Context/RunTimeContext/ProjectContext.h"
#include "core/Engine/EngineConfigTypes.h"
#include "core/Engine/IEngineRuntime.h"

namespace DataProtocol
{
	struct WindowDesc;
}

namespace EZ
{
	class IProject;
	struct ProjectContext;
	struct WorldContext;

	class IEngine
	{
	public:
		virtual ~IEngine() = default;

		virtual const char* GetEngineId() const = 0;

		virtual const char* GetEngineKey() const
		{
			return GetEngineId();
		}

		virtual const char* GetEngineDisplayName() const
		{
			return GetEngineId();
		}

		virtual int Start(IProject* project) = 0;
		virtual void RequestStop() = 0;
		virtual bool IsRunning() const = 0;

		virtual void BuildDefaultEngineConfig(EngineConfigTable& outConfig) const
		{
			(void)outConfig;
		}

		virtual void BuildDefaultProjectConfig(ProjectConfigTable& outConfig) const
		{
			(void)outConfig;
		}

		virtual bool NormalizeEngineConfig(EngineConfigTable& ioConfig) const
		{
			(void)ioConfig;
			return true;
		}

		virtual bool NormalizeProjectConfig(ProjectConfigTable& ioConfig) const
		{
			(void)ioConfig;
			return true;
		}

		virtual bool ValidateEngineConfig(const EngineConfigTable& config) const
		{
			(void)config;
			return true;
		}

		virtual bool ValidateProjectConfig(
			const ProjectConfigTable& projectConfig,
			const EngineConfigTable* engineConfig) const
		{
			(void)projectConfig;
			(void)engineConfig;
			return true;
		}

		virtual int InjectEngineConfig(ProjectContext& ctx, const EngineConfigTable& config)
		{
			(void)ctx;
			(void)config;
			return 0;
		}

		virtual int InjectProjectConfig(ProjectContext& ctx, const ProjectConfigTable& config)
		{
			(void)ctx;
			(void)config;
			return 0;
		}

		virtual void ResolveStartupScene(
			const ProjectConfigTable& projectConfig,
			std::string& outScenePath) const
		{
			(void)projectConfig;
			(void)outScenePath;
		}

		virtual std::unique_ptr<IEngineRuntime> CreateRuntime(ProjectContext& ctx)
		{
			(void)ctx;
			return nullptr;
		}

		virtual int OnAttachProjectContext(ProjectContext& ctx)
		{
			if (ctx.launchProfile)
			{
				if (!ctx.engineConfig)
				{
					ctx.engineConfig = &ctx.launchProfile->engineConfig;
				}

				if (!ctx.projectConfig)
				{
					ctx.projectConfig = &ctx.launchProfile->projectConfig;
				}

				if (ctx.startupScenePath.empty())
				{
					ctx.startupScenePath = ctx.launchProfile->startupScene;
				}
			}

			if (ctx.engineConfig && !ValidateEngineConfig(*ctx.engineConfig))
			{
				return -1;
			}

			if (ctx.projectConfig && !ValidateProjectConfig(*ctx.projectConfig, ctx.engineConfig))
			{
				return -1;
			}

			if (ctx.engineConfig)
			{
				const int ret = InjectEngineConfig(ctx, *ctx.engineConfig);
				if (ret != 0)
				{
					return ret;
				}
			}

			if (ctx.projectConfig)
			{
				const int ret = InjectProjectConfig(ctx, *ctx.projectConfig);
				if (ret != 0)
				{
					return ret;
				}

				if (ctx.startupScenePath.empty())
				{
					ResolveStartupScene(*ctx.projectConfig, ctx.startupScenePath);
				}
			}

			return 0;
		}

		virtual void OnDetachProjectContext(ProjectContext& ctx)
		{
			(void)ctx;
		}

		virtual void SetupMainWindowDesc(DataProtocol::WindowDesc& desc) const
		{
			(void)desc;
		}

		virtual void OnWorldContextCreated(ProjectContext& project, WorldContext& world)
		{
			(void)project;
			(void)world;
		}

		virtual void OnWorldContextDestroyed(ProjectContext& project, WorldContext& world)
		{
			(void)project;
			(void)world;
		}
	};
}

#endif