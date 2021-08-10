#pragma once

#include <vector>

#include "ChromosomeTemplate.h"
#include "CoverageData.h"
#include "BafData.h"

class ChromosomeData
{
public:
    ChromosomeTemplate chr_template;

    int scale;

	int offset;

    std::vector<CoverageData> log2_coverage_data;

    std::vector<BafData> baf_data;

    ChromosomeData shrink() const;
    int save(const std::string& path) const;
};
