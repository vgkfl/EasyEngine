#include "RootMotionDebugScript.h"

#include <cstdio>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif

#include "BaseProtocol/Animation/AnimatorComponent.h"
#include "ControlProtocol/EntityManager/EntityManager.h"
#include "core/Context/RunTimeContext/WorldContext.h"

namespace
{
	bool g_RootMotionConsoleInitialized = false;

	void EnsureDebugConsole()
	{
#ifdef _WIN32
		if (g_RootMotionConsoleInitialized)
		{
			return;
		}

		if (AllocConsole())
		{
			FILE* outFile = nullptr;
			FILE* errFile = nullptr;
			FILE* inFile = nullptr;

			freopen_s(&outFile, "CONOUT$", "w", stdout);
			freopen_s(&errFile, "CONOUT$", "w", stderr);
			freopen_s(&inFile, "CONIN$", "r", stdin);

			std::ios::sync_with_stdio(true);
			std::cout.clear();
			std::cerr.clear();
			std::cin.clear();

			std::printf("[RootMotion] console attached.\n");
			std::fflush(stdout);
		}

		g_RootMotionConsoleInitialized = true;
#endif
	}
}

void RootMotionDebugScript::Awake()
{
	EnsureDebugConsole();
}

void RootMotionDebugScript::Start()
{
	EnsureDebugConsole();
}

void RootMotionDebugScript::Update()
{
	auto* world = GetWorld();
	if (!world)
	{
		return;
	}

	auto* entityManager = world->TryGet<ControlProtocol::EntityManager>();
	if (!entityManager)
	{
		return;
	}

	auto* animator = entityManager->TryGetComponent<BaseProtocol::AnimatorComponent>(GetEntity());
	if (!animator)
	{
		return;
	}

	++m_FrameCounter;
	if (m_FrameCounter < 12)
	{
		return;
	}
	m_FrameCounter = 0;

	std::printf(
		"[RootMotion] has=%d state=%s clip=%s bone=%u prevTime=%.4f currTime=%.4f prevPos=(%.4f, %.4f, %.4f) currPos=(%.4f, %.4f, %.4f) deltaPos=(%.4f, %.4f, %.4f)\n",
		animator->hasRootMotion ? 1 : 0,
		animator->currentStateName.empty() ? "<none>" : animator->currentStateName.c_str(),
		(animator->currentClip && !animator->currentClip->sourcePath.empty())
		? animator->currentClip->sourcePath.c_str()
		: (animator->currentClip ? "<clip>" : "<none>"),
		animator->rootMotionBoneIndex,

#ifndef NDEBUG

		animator->debugRootMotionPreviousTime,
		animator->debugRootMotionCurrentTime,
		animator->debugRootMotionPreviousPosition.x,
		animator->debugRootMotionPreviousPosition.y,
		animator->debugRootMotionPreviousPosition.z,
		animator->debugRootMotionCurrentPosition.x,
		animator->debugRootMotionCurrentPosition.y,
		animator->debugRootMotionCurrentPosition.z,
#endif
		animator->extractedRootMotion.deltaPosition.x,
		animator->extractedRootMotion.deltaPosition.y,
		animator->extractedRootMotion.deltaPosition.z);


	std::fflush(stdout);
}