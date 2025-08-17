// ReflectionCoreExport.h
#pragma once

#ifdef REFLECTIONCORE_EXPORTS
#define REFLECTION_API __declspec(dllexport)
#else
#define REFLECTION_API __declspec(dllimport)
#endif
