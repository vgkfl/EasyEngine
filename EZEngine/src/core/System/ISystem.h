#pragma once
#ifndef __CORE_I_SYSTEM_H__
#define __CORE_I_SYSTEM_H__

namespace EZ
{
	struct ProjectContext;
	struct WorldContext;

	class ISystem
	{
	public:
		virtual ~ISystem() = default;

		virtual const char* GetName() const = 0;

		virtual int Initialize(ProjectContext& project, WorldContext& world) { return 0; }
		virtual void Shutdown(ProjectContext& project, WorldContext& world) {}

		virtual void BeginFrame(ProjectContext& project, WorldContext& world, float deltaTime) {}
		virtual void Update(ProjectContext& project, WorldContext& world, float deltaTime) {}
		virtual void FixedUpdate(ProjectContext& project, WorldContext& world, float fixedDeltaTime) {}
		virtual void LateUpdate(ProjectContext& project, WorldContext& world, float deltaTime) {}
		virtual void EndFrame(ProjectContext& project, WorldContext& world, float deltaTime) {}
	};
}
#endif
