#pragma once
#ifndef __TEST01_PROJECT_H__
#define __TEST01_PROJECT_H__

#include "core/ProjectManager/IProject.h"

class Test01Project : public EZ::IProject
{
public:
	void RegisterScripts(EZ::WorldContext& world) override;
	void RegisterStartupScene(EZ::WorldContext& world) override;
};

#endif