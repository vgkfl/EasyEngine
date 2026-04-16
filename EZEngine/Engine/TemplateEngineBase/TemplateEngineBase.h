#pragma once
#ifndef __TEMPLATE_ENGINE_BASE_H__
#define __TEMPLATE_ENGINE_BASE_H__

#include <memory>

#include "core/Engine/IEngine.h"

namespace EZ
{
	class ILauncher;
	class IRuntimeContextConfigurator;

	class TemplateEngineBase : public IEngine
	{
	public:
		int Start(IProject* project) override;
		void RequestStop() override;
		bool IsRunning() const override;

		std::unique_ptr<IEngineRuntime> CreateRuntime(ProjectContext& ctx) override;

	protected:
		virtual const char* GetDefaultLauncherId() const = 0;
		virtual std::unique_ptr<ILauncher> CreateLauncher() const;
		virtual IRuntimeContextConfigurator* ResolveRuntimeConfigurator(ProjectContext& ctx) const;

	private:
		bool m_IsRunning = false;
		ILauncher* m_ActiveLauncher = nullptr;
	};
}

#endif