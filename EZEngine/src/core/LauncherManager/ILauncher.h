#pragma once
#ifndef __CORE_I_LAUNCHER_H__
#define __CORE_I_LAUNCHER_H__

namespace EZ
{
	class IProgramRunner
	{
	public:
		virtual ~IProgramRunner() = default;

		virtual int Initialize() = 0;
		virtual bool TickFrame(float deltaTime) = 0;
		virtual int Shutdown() = 0;
	};

	class ILauncher
	{
	public:
		virtual ~ILauncher() = default;

		virtual int Start(IProgramRunner& runner) = 0;
		virtual void RequestQuit() = 0;
		virtual bool ShouldQuit() const = 0;
	};
}

#endif