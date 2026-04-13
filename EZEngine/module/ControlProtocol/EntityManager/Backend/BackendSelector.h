#pragma once
#ifndef __C_P_ENTITYMANAGER_BACKEND_SELECTOR_H__
#define __C_P_ENTITYMANAGER_BACKEND_SELECTOR_H__

#if defined(EZ_ENTITYMANAGER_BACKEND_ENTT)
#include "ControlProtocol/EntityManager/Backend/EnTT/EntityManagerBackend_EnTT.h"
#else
#error "No EntityManager backend selected. Define a backend target in CMake."
#endif

#endif
