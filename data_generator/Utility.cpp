#include "Utility.h"

std::vector<std::string> split(const std::string& string, char delimiter)
{
	std::vector<std::string> result;
	size_t i = 0;
	while (i < string.size())
	{
		size_t start = i;
		do ++i; while (i < string.size() && string[i] != delimiter);
		std::string element = string.substr(start, i - start);
		++i;
		result.push_back(element);
	}
	return result;
}

std::string trim(const std::string& string)
{
	size_t start = 0;
	while (start < string.size() && isspace(string[start]))
		++start;
	size_t end = string.size() - 1;
	while (end > 0 && isspace(string[end]))
		--end;
	++end;
	if (end == 0)
		return "";
	return string.substr(start, end - start);
}

std::string unwrap(const std::string& string, char wrapper)
{
	size_t i = 0;
	while (i < string.size())
	{
		while (i < string.size() && string[i] != wrapper)
			++i;
		++i;
		if (i >= string.size())
			break;
		size_t start = i;
		while (i < string.size() && string[i] != wrapper)
			++i;
		return string.substr(start, i - start);
	}
	return "";
}
