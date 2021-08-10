#pragma once

#include <string>

#include "CmdValue.h"

class CmdOption
{
public:
	CmdOption(char flag_short, const std::string& flag_long, const std::string& short_description, const std::string& long_description, const std::string& help, const std::vector<CmdValue>& values);

	std::string getFlagsString() const;
	std::string getValuesString() const;

	char flag_short;
	std::string flag_long;
	std::string short_description;
	std::string long_description;
	std::string help;
	std::vector<CmdValue> values;
};
