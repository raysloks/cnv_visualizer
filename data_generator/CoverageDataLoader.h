#pragma once

#include <istream>

class GenomeData;

class CoverageDataLoader
{
public:
	CoverageDataLoader(int scale);

	int scale;

	int load(GenomeData& data, std::istream& is);
};
