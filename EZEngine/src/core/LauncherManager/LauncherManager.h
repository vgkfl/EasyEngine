#pragma once

#ifndef __CORE_LAUNCHERMANAGER_H__
#define __CORE_LAUNCHERMANAGER_H__

#include <string>
#include <unordered_map>
#include <memory>
#include <functional>

#include "ILauncher.h"

namespace EZ
{
	class LauncherManager
	{
	public:
		using LauncherFactory = std::function<std::unique_ptr<ILauncher>()>;

	public:
		static LauncherManager& Get()
		{
			static LauncherManager instance;
			return instance;
		}

		void Register(const std::string& name, LauncherFactory factory)
		{
			m_Factories[name] = std::move(factory);
		}

		std::unique_ptr<ILauncher> Create(const std::string& name)
		{
			auto it = m_Factories.find(name);
			if (it == m_Factories.end())
			{
				return nullptr;
			}

			return it->second();
		}

		bool Has(const std::string& name) const
		{
			return m_Factories.find(name) != m_Factories.end();
		}

	private:
		std::unordered_map<std::string, LauncherFactory> m_Factories;
	};
}

// 用于拼接唯一名字
#define EZ_CONCAT_IMPL(x, y) x##y
#define EZ_CONCAT(x, y) EZ_CONCAT_IMPL(x, y)

// launcherClass: 类名
// launcherName : 注册名字符串
#define EZ_REGISTER_LAUNCHER(launcherClass, launcherName)                     \
    namespace                                                                 \
    {                                                                         \
        struct EZ_CONCAT(launcherClass, _AutoRegister_)                       \
        {                                                                     \
            EZ_CONCAT(launcherClass, _AutoRegister_)()                        \
            {                                                                 \
                EZ::LauncherManager::Get().Register((launcherName), []()      \
                {                                                             \
                    return std::make_unique<launcherClass>();                 \
                });                                                           \
            }                                                                 \
        };                                                                    \
                                                                              \
        static EZ_CONCAT(launcherClass, _AutoRegister_)                       \
            EZ_CONCAT(s_, launcherClass##_AutoRegister_Instance);             \
    }

#define EZ_REGISTER_LAUNCHER_AUTO_NAME(launcherClass) \
    EZ_REGISTER_LAUNCHER(launcherClass, #launcherClass)

#endif