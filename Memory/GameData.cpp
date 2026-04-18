#include "GameData.h"

#include <Windows.h>

#include "../Utils/Logger.h"
#include "../Utils/Utils.h"

GameData g_Data;

// ============================================================================
// !!! v26.13 TODO: SIGNATURE FOR C_ClientInstance POINTER !!!
// ============================================================================
// The byte pattern below (from the archived ~1.18 Horion) locates a static
// pointer-to-pointer-to-C_ClientInstance inside Minecraft.Windows.exe.
// For MC Bedrock Edition v1.26.13 this MUST be re-derived -- the byte pattern
// WILL NOT match the new binary and FindSignature() will return 0, causing
// getClientInstance() to always return nullptr.
//
// How to re-derive for v1.26.13:
//   1. Open the v1.26.13 Minecraft.Windows.exe in IDA / Ghidra / Binary Ninja.
//   2. Find a function that operates on the client-side ClientInstance.
//      Good entrypoints:
//        - Any function that calls `ClientInstance::tick` / `update`.
//        - Any callsite of `setMinecraftGame` or similar.
//        - Xrefs from the chat-input path (same site that Memory/Hooks.cpp
//          hooks -- you will likely find the ClientInstance global right
//          beside the send-chat function).
//   3. The canonical sequence is a RIP-relative `mov rdx, [ClientInstance]`
//      followed by dereferences. Example, from the old build:
//
//        48 8B 15 ?? ?? ?? ??   ; mov rdx, [rip + disp32]   <- disp at +3
//        4C 8B 02               ; mov r8, [rdx]
//        4C 89 06               ; mov [rsi], r8
//        40 84 FF               ; test dil, dil
//        74 ??                  ; jz short
//        ...
//
//      Write a pattern unique to that prologue on the new binary (use `??`
//      wildcards for any bytes that differ between Release rebuilds -- most
//      importantly the 4-byte disp32 at +3).
//   4. The resolver logic below is standard RIP-relative:
//        disp32 = *(int*)(sig + 3);
//        clientInstanceGlobalVA = sig + 7 + disp32;
//      Then read 3 levels of pointer indirection (the {0x0, 0x0, 0x50} path)
//      to land on the actual ClientInstance. Double-check that the
//      indirection levels still match in v1.26.13 -- they may have changed.
//      You can confirm by single-stepping in x64dbg.
// ============================================================================
#define HORION_V26_13_SIG_CLIENT_INSTANCE \
	"48 8B 15 ? ? ? ? 4C 8B 02 4C 89 06 40 84 FF 74 ? 48 8B CD E8 ? ? ? ? 48 8B C6 48 8B 4C 24 ? 48 33 CC E8 ? ? ? ? 48 8B 5C 24 ? 48 8B 6C 24 ? 48 8B 74 24 ? 48 83 C4 ? 5F C3 B9 ? ? ? ? E8 ? ? ? ? CC E8 ? ? ? ? CC CC CC CC CC CC CC CC CC CC CC 48 89 5C 24 ? 48 89 6C 24 ? 56"
/* ^^ TODO(v26.13): replace with a fresh pattern from the 1.26.13 binary. */

void GameData::retrieveClientInstance() {
	static uintptr_t clientInstanceOffset = 0x0;
	static bool sigSearchAttempted = false;
	if (clientInstanceOffset == 0x0 && !sigSearchAttempted) {
		sigSearchAttempted = true;
		uintptr_t sigOffset = FindSignature(HORION_V26_13_SIG_CLIENT_INSTANCE);
		if (sigOffset != 0x0) {
			int disp32 = *reinterpret_cast<int*>(sigOffset + 3);
			// RIP-relative: address of next instruction + displacement.
			clientInstanceOffset = sigOffset - g_Data.gameModule->ptrBase + disp32 + /*length*/ 7;
			logF("[horion-xp] client instance global RVA: %llX", (unsigned long long)clientInstanceOffset);
		} else {
			logF("[horion-xp] FAILED to locate C_ClientInstance via signature. "
				"Update HORION_V26_13_SIG_CLIENT_INSTANCE in Memory/GameData.cpp "
				"for Minecraft Bedrock v1.26.13.");
		}
	}
	if (clientInstanceOffset == 0x0) {
		g_Data.clientInstance = nullptr;
		return;
	}

	g_Data.clientInstance = reinterpret_cast<C_ClientInstance*>(
		g_Data.slimMem->ReadPtr<uintptr_t*>(
			g_Data.gameModule->ptrBase + clientInstanceOffset,
			{0x0, 0x0, 0x50}));
}

bool GameData::shouldTerminate() {
	return g_Data.shouldTerminateB;
}

void GameData::terminate() {
	g_Data.shouldTerminateB = true;
}

void GameData::initGameData(const SlimUtils::SlimModule* gameModule,
                            SlimUtils::SlimMem* slimMem,
                            void* hDllInst) {
	g_Data.gameModule = gameModule;
	g_Data.slimMem = slimMem;
	g_Data.hDllInst = hDllInst;

	retrieveClientInstance();

	logF("[horion-xp] base: %llX", (unsigned long long)g_Data.gameModule->ptrBase);
	logF("[horion-xp] clientInstance: %p", g_Data.clientInstance);
	logF("[horion-xp] localPlayer: %p", g_Data.getLocalPlayer());
}
