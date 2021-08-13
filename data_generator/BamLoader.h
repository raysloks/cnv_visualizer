#pragma once

#include <string>

#include "GenomeData.h"

class BamLoader
{
public:
    int load(GenomeData& data, const std::string& fname);
};
