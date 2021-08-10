#include "CmdOption.h"

CmdOption::CmdOption(char flag_short, const std::string& flag_long, const std::string& short_description, const std::string& long_description, const std::string& help, const std::vector<CmdValue>& values) : flag_short(flag_short), flag_long(flag_long), short_description(short_description), long_description(long_description), help(help), values(values)
{
}

std::string CmdOption::getFlagsString() const
{
	std::string str = "-";
	if (flag_short != 0)
		if (flag_long.empty())
			str += flag_short;
		else
			str += flag_short + ", --" + flag_long;
	else
		str += "-" + flag_long;
	return str;
}

std::string CmdOption::getValuesString() const
{
	std::string str = "[";
	for (size_t i = 0; i < values.size(); ++i)
	{
		if (i != 0)
			str += " ";
		str += values[i].getTypeName();
	}
	str += "]";
	return str;
}
