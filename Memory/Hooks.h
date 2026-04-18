#pragma once

#include <memory>

#include "../SDK/TextHolder.h"
#include "GameData.h"
#include "MinHook.h"

// ============================================================================
// STRIPPED-DOWN Hooks FOR .xp COMMAND ONLY
// ============================================================================
// The full Horion Hooks class installed 30+ MinHook detours into
// Minecraft.Windows.exe (renderers, networking, physics, etc.).
//
// For a minimal `.xp`-only build we only install one detour: an intercept on
// the client-side "send chat message" entrypoint. Whenever the local player
// types something in chat, the detour gets a chance to inspect the text. If
// the text starts with our command prefix (e.g. `.xp 100`), we dispatch it
// to CommandMgr and skip sending it upstream. Otherwise we forward to the
// original function unchanged.
// ============================================================================

class FuncHook;

class Hooks {
public:
	static void Init();
	static void Restore();
	static void Enable();

private:
	static void ClientInstanceScreenModel_sendChatMessage(void* _this, TextHolder* text);

	std::unique_ptr<FuncHook> ClientInstanceScreenModel_sendChatMessageHook;
};

extern Hooks g_Hooks;

class FuncHook {
public:
	void* funcPtr = nullptr;
	void* funcReal = nullptr;

	FuncHook(void* func, void* hooked) {
		funcPtr = func;
		if (func == nullptr) {
			// FindSignature() returned 0 -- signature wasn't located. Leave
			// funcReal null and skip Create/Enable so the game keeps running
			// as if no hook was installed.
			return;
		}
		MH_STATUS ret = MH_CreateHook(func, hooked, &funcReal);
		if (ret != MH_OK) {
			// logF is not available here without pulling Logger in; the
			// failures are also logged at the Hooks::Init callsite.
		}
	}

	FuncHook(uintptr_t func, void* hooked)
		: FuncHook(reinterpret_cast<void*>(func), hooked) {}

	void enableHook(bool enable = true) {
		if (funcPtr == nullptr) return;
		if (enable) MH_EnableHook(funcPtr);
		else        MH_DisableHook(funcPtr);
	}

	~FuncHook() { Restore(); }

	void Restore() {
		if (funcPtr != nullptr)
			MH_DisableHook(funcPtr);
	}

	template <typename TRet, typename... TArgs>
	inline auto* GetFastcall() {
		using Fn = TRet(__fastcall*)(TArgs...);
		return reinterpret_cast<Fn>(funcReal);
	}
};
