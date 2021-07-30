#pragma once

#include <vector>
#include <string>

class VcfColumn
{
public:
	std::string name;
	std::vector<std::string> fields;
};
