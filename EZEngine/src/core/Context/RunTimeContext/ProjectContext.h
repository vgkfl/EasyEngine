#pragma once
#ifndef __PROJECT_CONTEXT_H__
#define __PROJECT_CONTEXT_H__

#include <string>

namespace EZ
{
	class IProject;
	class IEngine;
	struct EngineConfigTable;
	struct ProjectConfigTable;
	struct ProjectLaunchProfile;

	struct ProjectContext
	{
		IProject* project = nullptr;
		IEngine* engine = nullptr;

		const ProjectLaunchProfile* launchProfile = nullptr;
		const EngineConfigTable* engineConfig = nullptr;
		const ProjectConfigTable* projectConfig = nullptr;
		std::string startupScenePath;

		float deltaTime = 0.0f;
		float fixedDeltaTime = 1.0f / 60.0f;
		bool isEditor = false;
	};
}

#endif