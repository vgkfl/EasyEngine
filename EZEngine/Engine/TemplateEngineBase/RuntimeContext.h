#pragma once
#ifndef __TEMPLATE_ENGINE_BASE_RUNTIME_CONTEXT_H__
#define __TEMPLATE_ENGINE_BASE_RUNTIME_CONTEXT_H__

#include <memory>

#include "core/Context/RunTimeContext/ProjectContext.h"
#include "core/Context/RunTimeContext/ProjectExtensionContext.h"
#include "core/Context/RunTimeContext/TemplateWorldContext.h"
#include "core/Context/RunTimeContext/WorldContext.h"

namespace EZ
{
	class IRuntimeContextConfigurator;
	class SystemManager;

	class RuntimeContext
	{
	public:
		RuntimeContext();
		~RuntimeContext();

		int Initialize(ProjectContext& project, IRuntimeContextConfigurator* configurator = nullptr);
		void Shutdown();

		void Tick(ProjectContext& project);

		WorldContext& GetWorldContext();
		const WorldContext& GetWorldContext() const;

		TemplateWorldContext& GetTemplateWorldContext();
		const TemplateWorldContext& GetTemplateWorldContext() const;

		ProjectExtensionContext& GetProjectExtensionContext();
		const ProjectExtensionContext& GetProjectExtensionContext() const;

		SystemManager& GetSystemManager();
		const SystemManager& GetSystemManager() const;

	private:
		struct Impl;
		std::unique_ptr<Impl> m_Impl;
	};
}

#endif