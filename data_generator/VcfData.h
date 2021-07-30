#pragma once

#include "VcfRecord.h"

class VcfData
{
public:
	std::map<std::string, std::string> meta;
	std::vector<VcfRecord> records;
};
