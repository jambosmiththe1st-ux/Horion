#pragma once

#include <cstdint>

#include "../SDK/CClientInstance.h"
#include "SlimMem.h"

// ============================================================================
// STRIPPED-DOWN GameData FOR .xp COMMAND ONLY
// ============================================================================
// The original Horion GameData class maintained dozens of subsystems (entity
// list, HID controller, RakNet instance, injector connection, chest list,
// fake skin geometry, ...). For a minimal `.xp`-only build we only need:
//
//   1. Resolve the C_ClientInstance pointer from Minecraft.Windows.exe
//      (signature scan, see GameData.cpp).
//   2. Read the local player pointer off of ClientInstance.
//   3. Start / stop flag for the keyboard uninject hotkey thread.
//
// Everything else has been removed.
// ============================================================================

class GameData {
private:
	C_ClientInstance* clientInstance = nullptr;
	C_LocalPlayer* localPlayer = nullptr;

	const SlimUtils::SlimModule* gameModule = nullptr;
	SlimUtils::SlimMem* slimMem = nullptr;
	void* hDllInst = nullptr;

	bool shouldTerminateB = false;

	static void retrieveClientInstance();

public:
	static void initGameData(const SlimUtils::SlimModule* gameModule,
	                         SlimUtils::SlimMem* slimMem,
	                         void* hDllInst);

	static bool shouldTerminate();
	static void terminate();

	inline void* getDllModule() { return hDllInst; }
	inline C_ClientInstance* getClientInstance() {
		retrieveClientInstance();
		return clientInstance;
	}
	inline C_LocalPlayer* getLocalPlayer() {
		retrieveClientInstance();
		if (clientInstance == nullptr) {
			localPlayer = nullptr;
			return nullptr;
		}
		localPlayer = clientInstance->getLocalPlayer();
		return localPlayer;
	}
	inline const SlimUtils::SlimModule* getModule() { return gameModule; }
	inline const SlimUtils::SlimMem* getSlimMem() { return slimMem; }
};

extern GameData g_Data;
