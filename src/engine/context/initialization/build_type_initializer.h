#pragma once

#include "core_initializer.h"

#ifdef NDEBUG
using CoreInitializerT = nuff::renderer::CoreInitializer;
#else
#include "debug_initializer_mixin.h"
using CoreInitializerT = nuff::renderer::DebugInitializer;
#endif

