#pragma once

#if defined(EZ_ENTITYMANAGER_BACKEND_ENTT)
#include "ControlProtocol/EntityManager/Backend/EnTT/EntityManagerBackend_EnTT.inl"
#else
#error "No EntityManager backend selected. Configure one in CMake."
#endif
