#pragma once

#ifndef __CORE_I_LAUNCHER_H__
#define __CORE_I_LAUNCHER_H__


namespace EZ
{
	class ILauncher
	{
	public:
		virtual ~ILauncher() = default;
		virtual int Start() = 0;
	};
}
#endif