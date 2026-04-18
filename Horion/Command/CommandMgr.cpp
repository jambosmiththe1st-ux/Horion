#include "CommandMgr.h"

#include <sstream>

bool CommandMgr::tryExecute(const std::string& message) {
	if (message.empty() || message[0] != CommandMgr::getPrefix())
		return false;

	// Split the message (without the leading prefix) on whitespace.
	std::vector<std::string> args;
	{
		std::istringstream iss(message.substr(1));
		std::string tok;
		while (iss >> tok)
			args.push_back(tok);
	}
	if (args.empty())
		return false;

	const std::string& name = args.front();

	// .xp is the only command in this stripped-down build.
	if (name == "xp") {
		XpCommand cmd;
		return cmd.execute(&args);
	}

	return false;
}
