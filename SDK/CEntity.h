#pragma once

#include <cstdint>

// ============================================================================
// STRIPPED-DOWN SDK FOR .xp COMMAND ONLY
// ============================================================================
// This is a minimal subset of the Horion C_Entity SDK, kept only to the extent
// that the `.xp <amount>[L]` command needs: calling LocalPlayer::addExperience
// and LocalPlayer::addLevels.
//
// The original Horion SDK exposed dozens of fields at hard-coded offsets that
// were valid for the MC Bedrock build Horion was last updated against (~1.18).
// The `.xp` command only needs two virtual methods on the local player, so we
// expose them through vtable slots -- no field offsets required.
//
// ============================================================================
// !!! v26.13 TODO: VTABLE INDICES FOR addExperience / addLevels !!!
// ============================================================================
// The vtable slot indices for `C_LocalPlayer::addExperience(int)` and
// `C_LocalPlayer::addLevels(int)` below MUST be verified against the MC Bedrock
// Edition v1.26.13 `Minecraft.Windows.exe` binary. They were extracted from an
// older (Dec 2021 / ~1.18) build and are VERY likely to have changed.
//
// How to verify / re-derive:
//   1. Load Minecraft.Windows.exe (1.26.13) in IDA / Ghidra / Binary Ninja.
//   2. Find the `Player` (or `Actor`/`LocalPlayer`) vtable. One way:
//        - Search for the string "addExperience" (or the error string logged
//          when you add experience).
//        - xref to the function, get its address, then find it inside a
//          vtable (a rdata array of function pointers).
//      Alternative:
//        - Find `Player::addExperience(int)` by xref from experience-orb code
//          or from the /xp command handler on the server side.
//   3. Count entries in the vtable until you hit addExperience / addLevels.
//      Update VTIDX_ADD_EXPERIENCE / VTIDX_ADD_LEVELS below.
//   4. Double-check the calling convention has not changed
//      (it has been `void __fastcall Player::addExperience(Player*, int)` for
//      many versions -- if it changes, update the CallVFunc signature too).
// ============================================================================

// TODO(v26.13): verify these vtable slot indices against the 1.26.13 binary.
// These values are from the old (~1.18) Horion SDK and are placeholders.
#define HORION_V26_13_VTIDX_ADD_EXPERIENCE 0xA5  /* TODO(v26.13): verify */
#define HORION_V26_13_VTIDX_ADD_LEVELS     0xA6  /* TODO(v26.13): verify */

class C_Entity {
public:
	// Virtual destructor + lots of virtual methods live here. We never call
	// anything by index on C_Entity directly from .xp, so no dummies needed.
	virtual ~C_Entity() = default;
};

class C_Player : public C_Entity {
};

class C_LocalPlayer : public C_Player {
public:
	// These two virtuals are what the .xp command calls. We dispatch through
	// the vtable by index so we don't have to reproduce every virtual method
	// above them (which would be brittle and version-specific).
	inline void addExperience(int amount) {
		using Fn = void(__thiscall*)(C_LocalPlayer*, int);
		Fn fn = (*reinterpret_cast<Fn**>(this))[HORION_V26_13_VTIDX_ADD_EXPERIENCE];
		fn(this, amount);
	}

	inline void addLevels(int amount) {
		using Fn = void(__thiscall*)(C_LocalPlayer*, int);
		Fn fn = (*reinterpret_cast<Fn**>(this))[HORION_V26_13_VTIDX_ADD_LEVELS];
		fn(this, amount);
	}
};
