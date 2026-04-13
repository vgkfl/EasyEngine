#pragma once
#ifndef __CORE_I_PROJECT_H__
#define __CORE_I_PROJECT_H__

namespace EZ
{
	struct ProjectContext;
	struct WorldContext;

	class IProject
	{
	public:
		virtual ~IProject() = default;

		virtual int OnPreEngineInit(ProjectContext& ctx) { return 0; }
		virtual int OnEngineInit(ProjectContext& ctx) { return 0; }
		virtual int OnPostEngineInit(ProjectContext& ctx) { return 0; }

		virtual int OnBeforeMainLoop(ProjectContext& ctx) { return 0; }
		virtual int OnAfterMainLoop(ProjectContext& ctx) { return 0; }

		virtual int OnBeforeEngineShutdown(ProjectContext& ctx) { return 0; }
		virtual int OnEngineShutdown(ProjectContext& ctx) { return 0; }

		virtual void OnWorldCreated(WorldContext& world)
		{
			RegisterScripts(world);
			RegisterStartupScene(world);
		}

		virtual void OnWorldDestroyed(WorldContext& world) { (void)world; }

		virtual void RegisterScripts(EZ::WorldContext& world) { (void)world; }
		virtual void RegisterStartupScene(EZ::WorldContext& world) { (void)world; }

		virtual void OnProjectGUI(ProjectContext& ctx) { (void)ctx; }
		virtual void OnProjectEditorGUI(ProjectContext& ctx) { (void)ctx; }
	};
}

#endif