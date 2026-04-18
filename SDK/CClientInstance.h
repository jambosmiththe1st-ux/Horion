#pragma once

#include <cstdint>
#include "CEntity.h"

// ============================================================================
// STRIPPED-DOWN SDK FOR .xp COMMAND ONLY
// ============================================================================
// Minimal C_ClientInstance definition. All .xp needs is a way to get from the
// ClientInstance pointer (located via signature scan, see Memory/GameData.cpp)
// to the local player pointer.
//
// In older Horion builds, C_ClientInstance was 0x900+ bytes with hundreds of
// inlined fields. We only need to read one pointer: `localPlayer`.
//
// ============================================================================
// !!! v26.13 TODO: OFFSET OF localPlayer WITHIN C_ClientInstance !!!
// ============================================================================
// The offset from ClientInstance* to its LocalPlayer* field changes between
// game versions. The value below (0x138) was correct for MC Bedrock ~1.18
// which is when Horion was last updated. For 1.26.13 this almost certainly
// needs updating.
//
// How to re-derive:
//   1. Find C_ClientInstance in Minecraft.Windows.exe (1.26.13). Easiest way
//      is by following the signature scan in Memory/GameData.cpp to its
//      target, then inspecting the resulting class in IDA/Ghidra.
//   2. Find the field of type `LocalPlayer*` (or `ClientPlayer*`). It's
//      typically the first or second `Player*`-typed member. You can usually
//      identify it by:
//        - It is dereferenced right before calls to Player virtuals like
//          `tick`, `swing`, `addExperience`, etc.
//        - It is written in `ClientInstance::initializeLocalPlayer` / similar.
//   3. Note its byte offset from the start of C_ClientInstance and update
//      HORION_V26_13_OFFSET_CLIENT_INSTANCE_TO_LOCAL_PLAYER below.
// ============================================================================

// TODO(v26.13): verify / update against the 1.26.13 binary.
#define HORION_V26_13_OFFSET_CLIENT_INSTANCE_TO_LOCAL_PLAYER 0x138  /* TODO(v26.13): verify */

class C_ClientInstance {
public:
	inline C_LocalPlayer* getLocalPlayer() const {
		auto raw = reinterpret_cast<const uint8_t*>(this)
			+ HORION_V26_13_OFFSET_CLIENT_INSTANCE_TO_LOCAL_PLAYER;
		return *reinterpret_cast<C_LocalPlayer* const*>(raw);
	}
};
