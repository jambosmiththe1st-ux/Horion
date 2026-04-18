#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Commands/ICommand.h"
#include "Commands/XpCommand.h"

// ============================================================================
// STRIPPED-DOWN CommandMgr FOR .xp COMMAND ONLY
// ============================================================================
// The only command registered is XpCommand. All other commands were removed.
// ============================================================================

class CommandMgr {
public:
	static char getPrefix() { return '.'; }

	// Returns true when `message` matched a registered command (and was
	// consumed). Returns false when it should be forwarded to the game's
	// normal chat path.
	static bool tryExecute(const std::string& message);

	// Kept for parity with the old API, but implemented inline in terms of
	// tryExecute so there is no static state to initialise.
	static void initCommands() {}
};
