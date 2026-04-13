#pragma once
#ifndef __CORE_I_ENGINE_RUNTIME_H__
#define __CORE_I_ENGINE_RUNTIME_H__

namespace EZ
{
	struct ProjectContext;
	struct WorldContext;

	class IEngineRuntime
	{
	public:
		virtual ~IEngineRuntime() = default;

		virtual int Initialize(ProjectContext& project) = 0;
		virtual bool Tick(ProjectContext& project) = 0;
		virtual int Shutdown(ProjectContext& project) = 0;

		virtual WorldContext& GetWorldContext() = 0;
		virtual const WorldContext& GetWorldContext() const = 0;
	};
}

#endif