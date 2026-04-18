#pragma once

#include <cstdarg>
#include <string>
#include <vector>

#include "../../../Memory/GameData.h"
#include "../../../Utils/Logger.h"
#include "../../../Utils/TextFormat.h"

// ============================================================================
// Stripped-down command base. Removed: friend-list integration, per-command
// alias vectors, injector-message forwarding, config-persist hooks.
//
// The remaining contract is minimal:
//   - `getCommand()` returns the slash-less name (e.g. "xp").
//   - `getDescription()` returns a one-line help string.
//   - `getUsage()` returns a one-line usage string.
//   - `execute(args)` runs the command. args[0] is the command name itself.
// ============================================================================

class IMCCommand {
protected:
	std::string command;
	std::string description;
	std::string usage;

public:
	IMCCommand(std::string command, std::string description, std::string usage)
		: command(std::move(command)),
		  description(std::move(description)),
		  usage(std::move(usage)) {}

	virtual ~IMCCommand() = default;

	const std::string& getCommand() const { return command; }
	const std::string& getDescription() const { return description; }
	const std::string& getUsage() const { return usage; }

	virtual bool execute(std::vector<std::string>* args) = 0;

	// Minimal "client message" -- emits to the Horion debug log only. The old
	// Horion would print this inside Minecraft's chat UI via a signature-
	// scanned C_GuiData::displayClientMessage, but that hook is also version-
	// specific. Printing to the log keeps the command self-contained and
	// independent of any additional v26.13 reversing beyond what GameData
	// already requires.
	static void clientMessageF(const char* fmt, ...) {
		char buf[512];
		va_list ap;
		va_start(ap, fmt);
		vsnprintf(buf, sizeof(buf), fmt, ap);
		va_end(ap);
		buf[sizeof(buf) - 1] = '\0';
		logF("[horion-xp] %s", buf);
	}
};
