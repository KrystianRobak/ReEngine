#pragma once

#if defined(REENGINE_CORE_EXPORTS)
#define CORE_API __declspec(dllexport)
#else
#define CORE_API __declspec(dllimport)
#endif