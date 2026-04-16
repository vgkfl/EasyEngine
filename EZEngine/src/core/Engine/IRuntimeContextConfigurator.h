#pragma once
#ifndef __CORE_I_RUNTIME_CONTEXT_CONFIGURATOR_H__
#define __CORE_I_RUNTIME_CONTEXT_CONFIGURATOR_H__

namespace EZ
{
	class ProjectExtensionContext;
	class RuntimeContext;

	class IRuntimeContextConfigurator
	{
	public:
		virtual ~IRuntimeContextConfigurator() = default;

		virtual void ConfigureProjectExtension(ProjectExtensionContext& extension) const
		{
			(void)extension;
		}

		virtual void RegisterProjectControllers(RuntimeContext& runtime) const
		{
			(void)runtime;
		}

		virtual void RegisterProjectSystems(RuntimeContext& runtime) const
		{
			(void)runtime;
		}

		virtual void InstallProjectTools(RuntimeContext& runtime) const
		{
			(void)runtime;
		}

		virtual void OnRuntimeCreated(RuntimeContext& runtime) const
		{
			(void)runtime;
		}

		virtual void OnRuntimeDestroyed(RuntimeContext& runtime) const
		{
			(void)runtime;
		}
	};
}

#endif