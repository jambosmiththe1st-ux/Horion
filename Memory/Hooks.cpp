#include "Hooks.h"

#include <Windows.h>

#include "../Horion/Command/CommandMgr.h"
#include "../Utils/Logger.h"
#include "../Utils/Utils.h"

Hooks g_Hooks;

// ============================================================================
// !!! v26.13 TODO: SIGNATURE FOR CHAT-SEND HOOK !!!
// ============================================================================
// The byte pattern below locates the entrypoint Horion detours so that
// every message the local player types in chat passes through our command
// dispatcher FIRST. If the text starts with the command prefix ("." by
// default, see CommandMgr), we intercept and run the command without
// forwarding it to the server.
//
// For MC Bedrock Edition v1.26.13 this signature MUST be re-derived. Until
// it is, the FindSignature() call returns 0, the MH_CreateHook call silently
// no-ops, and typing `.xp 100` in chat will NOT trigger the command -- the
// message will instead be sent to the server verbatim.
//
// How to re-derive the chat-send signature for v1.26.13:
//   1. Open Minecraft.Windows.exe (v1.26.13) in IDA/Ghidra/Binary Ninja.
//   2. Find `ClientInstanceScreenModel::sendChatMessage` (older name) or its
//      modern equivalent. Heuristics:
//        - xref from strings like "chat" or "sendChatMessage" if present.
//        - It takes a `this` pointer and a `TextHolder&` (SSO string).
//        - It calls packet-creation code for MinecraftPackets::ChatPacket.
//        - It is called from the chat UI's "send button" handler.
//   3. Grab 32-48 bytes of its unique prologue and write them out as a
//      `?`-masked pattern. The old prologue for reference:
//
//        48 89 5C 24 ??       ; mov [rsp+var], rbx
//        55                    ; push rbp
//        56                    ; push rsi
//        57                    ; push rdi
//        41 54 41 55 41 56 41 57
//        48 8D AC 24 ?? ?? ?? ??
//        48 81 EC ?? ?? ?? ??
//        48 8B 05 ?? ?? ?? ??   ; __security_cookie
//        48 33 C4
//        48 89 85 ?? ?? ?? ??
//        4C 8B EA 4C 8B F9 48 8B 49
//
//   4. Paste the pattern into HORION_V26_13_SIG_CHAT_SEND below.
//   5. Confirm by loading the DLL in a debug build and checking the log at
//      %APPDATA%\Horion\horion.log (see Logger.cpp) -- you should see no
//      "FAILED to locate" warnings on inject.
// ============================================================================
#define HORION_V26_13_SIG_CHAT_SEND \
	"48 89 5C 24 ?? 55 56 57 41 54 41 55 41 56 41 57 48 8D AC 24 ?? ?? ?? ?? 48 81 EC ?? ?? ?? ?? 48 8B 05 ?? ?? ?? ?? 48 33 C4 48 89 85 ?? ?? ?? ?? 4C 8B EA 4C 8B F9 48 8B 49"
/* ^^ TODO(v26.13): replace with a fresh pattern from the 1.26.13 binary. */

// ---------------------------------------------------------------------------
// Detour body
// ---------------------------------------------------------------------------
void Hooks::ClientInstanceScreenModel_sendChatMessage(void* _this, TextHolder* text) {
	// Original function signature: void(__fastcall*)(void*, TextHolder*).
	auto original = g_Hooks.ClientInstanceScreenModel_sendChatMessageHook
		? g_Hooks.ClientInstanceScreenModel_sendChatMessageHook->GetFastcall<void, void*, TextHolder*>()
		: nullptr;

	if (text != nullptr && text->getText() != nullptr) {
		std::string message(text->getText(), text->getTextLength());
		if (CommandMgr::tryExecute(message)) {
			// Swallow the message -- command handled it.
			return;
		}
	}

	if (original != nullptr)
		original(_this, text);
}

// ---------------------------------------------------------------------------
// Install / remove / enable
// ---------------------------------------------------------------------------
void Hooks::Init() {
	MH_Initialize();

	uintptr_t sendChat = FindSignature(HORION_V26_13_SIG_CHAT_SEND);
	if (sendChat == 0) {
		logF("[horion-xp] FAILED to locate sendChatMessage signature. "
			"Update HORION_V26_13_SIG_CHAT_SEND in Memory/Hooks.cpp for "
			"Minecraft Bedrock v1.26.13. The .xp command will not be "
			"triggered from chat input until this is fixed.");
	}
	g_Hooks.ClientInstanceScreenModel_sendChatMessageHook =
		std::make_unique<FuncHook>(sendChat,
			reinterpret_cast<void*>(&Hooks::ClientInstanceScreenModel_sendChatMessage));
}

void Hooks::Enable() {
	if (g_Hooks.ClientInstanceScreenModel_sendChatMessageHook)
		g_Hooks.ClientInstanceScreenModel_sendChatMessageHook->enableHook(true);
}

void Hooks::Restore() {
	if (g_Hooks.ClientInstanceScreenModel_sendChatMessageHook)
		g_Hooks.ClientInstanceScreenModel_sendChatMessageHook->enableHook(false);
	MH_Uninitialize();
}
