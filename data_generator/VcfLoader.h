#pragma once

#include <iostream>

#include "VcfData.h"
#include "GenomeData.h"

class VcfLoader
{
public:
	VcfLoader();

	int load(VcfData& data, std::istream& is);
	int filterBaf(std::istream& is, std::ostream& os);
	int filterBafData(GenomeData& data, std::istream& is_filter, std::istream& is_calls);

	std::string baf_key;
	float baf_cutoff;
};
