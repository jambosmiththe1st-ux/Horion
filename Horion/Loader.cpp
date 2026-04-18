// ============================================================================
// STRIPPED-DOWN DLL ENTRY FOR .xp COMMAND ONLY
// ============================================================================
// The original Loader.cpp (408 lines) started a background thread for talking
// to the Horion UWP injector over a named pipe, loaded every module, opened
// the ClickGui / TabGui, spun up the ModuleManager, ConfigManager, Scripting
// engine, FriendList, PathFinder, ChakraJS host, etc.
//
// This stripped-down build only does three things:
//   1. On DLL_PROCESS_ATTACH, spawn a thread that calls start().
//   2. start() locates Minecraft.Windows.exe's module base, initialises
//      GameData (which performs the signature scan for the ClientInstance
//      global), installs the chat-send hook, and then idles waiting for the
//      user to press L+Ctrl to cleanly unhook and unload the DLL.
//   3. On DLL_PROCESS_DETACH, restore MinHook state.
// ============================================================================

#include <Windows.h>

#include <memory>
#include <thread>

#include "../Memory/GameData.h"
#include "../Memory/Hooks.h"
#include "../Memory/SlimMem.h"
#include "../Utils/Logger.h"
#include "../Utils/Utils.h"

static HMODULE g_DllModule = nullptr;

static void keyThread() {
	// Uninject hotkey: hold Ctrl and press L to cleanly unhook everything.
	while (!GameData::shouldTerminate()) {
		if ((GetAsyncKeyState(VK_CONTROL) & 0x8000) && (GetAsyncKeyState('L') & 0x8000)) {
			GameData::terminate();
			break;
		}
		Sleep(50);
	}
}

static DWORD WINAPI start(LPVOID) {
	auto slim = std::make_unique<SlimUtils::SlimMem>();
	slim->ParseModules();

	const SlimUtils::SlimModule* mcModule = slim->GetModule(L"Minecraft.Windows.exe");
	if (mcModule == nullptr) {
		logF("[horion-xp] Could not find Minecraft.Windows.exe module. Aborting init.");
		return 0;
	}

	GameData::initGameData(mcModule, slim.get(), g_DllModule);

	Hooks::Init();
	Hooks::Enable();

	logF("[horion-xp] .xp-only build initialised. Press Ctrl+L to unload.");

	std::thread keys(keyThread);
	keys.detach();

	while (!GameData::shouldTerminate())
		Sleep(100);

	Hooks::Restore();

	logF("[horion-xp] Unloading.");
	FreeLibraryAndExitThread(g_DllModule, 0);
	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID) {
	switch (ul_reason_for_call) {
		case DLL_PROCESS_ATTACH:
			g_DllModule = hModule;
			DisableThreadLibraryCalls(hModule);
			CreateThread(nullptr, 0, start, nullptr, 0, nullptr);
			break;
		case DLL_PROCESS_DETACH:
			break;
	}
	return TRUE;
}
