#pragma once

#include <windows.h>
#include "Logger.h"
#include "ReTypes.h"

typedef void* (*CreateAppFunc)();
typedef void (*DestroyAppFunc)(void*);
typedef void (*AppInitFunc)(void*);
typedef void (*AppUpdateFunc)(void*);
typedef void (*AppRenderFunc)(void*);
typedef bool (*AppIsRunningFunc)(void*);
typedef void* (*AppInitSysyemsFunc)(void*);
typedef void (*AppStartThreadsFunc)(void*);
typedef void (*AppStartThreadsFunc)(void*);
typedef void (*AppOnUpdateUIFunc)(void*, FunctionDelegate);
typedef void (*AppOnPostUpdateUIFunc)(void*, FunctionDelegate);
typedef void (*AppOnPreUpdateUIFunc)(void*, FunctionDelegate);

class ApplicationWrapper
{
public:
	ApplicationWrapper() {
		LOGF_INFO("%s", "Initializing Application Wrapper")
	}

	void LoadFunctions(HMODULE engineDLL) {
		// --- Load Application Functions ---
		CreateApplication = (CreateAppFunc)GetProcAddress(engineDLL, "CreateApplication");
		DestroyApplication = (DestroyAppFunc)GetProcAddress(engineDLL, "DestroyApplication");
		Application_Init = (AppInitFunc)GetProcAddress(engineDLL, "Application_Init");
		Application_Update = (AppUpdateFunc)GetProcAddress(engineDLL, "Application_Update");
		Application_Render = (AppRenderFunc)GetProcAddress(engineDLL, "Application_Render");
		Application_IsRunning = (AppIsRunningFunc)GetProcAddress(engineDLL, "Application_IsRunning");
		Application_InitSystems = (AppInitSysyemsFunc)GetProcAddress(engineDLL, "Application_InitSystems");
		Application_StartThreads = (AppStartThreadsFunc)GetProcAddress(engineDLL, "Application_StartThreads");
		AppOnUpdateUI = (AppOnUpdateUIFunc)GetProcAddress(engineDLL, "Application_OnUpdateUI");
		AppOnPostUpdateUI = (AppOnPostUpdateUIFunc)GetProcAddress(engineDLL, "Application_OnPostUpdateUI");
		AppOnPreUpdateUI = (AppOnPreUpdateUIFunc)GetProcAddress(engineDLL, "Application_OnPreUpdateUI");
	}

	bool IsProperlyLoaded() {
		if (!CreateApplication || !DestroyApplication || !Application_Init || !Application_Update || !Application_Render || !Application_IsRunning) {
			LOGF_ERROR("%s", "Failed while initializing engine functions!")
			return false;
		}
		return true;
	}

public:
	void* app;

	CreateAppFunc CreateApplication;
	DestroyAppFunc DestroyApplication;
	AppInitFunc Application_Init;
	AppUpdateFunc Application_Update;
	AppRenderFunc Application_Render;
	AppIsRunningFunc Application_IsRunning;
	AppInitSysyemsFunc Application_InitSystems;
	AppStartThreadsFunc Application_StartThreads;

	AppOnUpdateUIFunc AppOnUpdateUI;
	AppOnPostUpdateUIFunc AppOnPostUpdateUI;
	AppOnPreUpdateUIFunc AppOnPreUpdateUI;

};

