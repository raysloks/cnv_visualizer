#pragma once

#include <istream>

#include "VcfData.h"

class VcfLoader
{
public:
	int load(VcfData& data, std::istream& is);
};
