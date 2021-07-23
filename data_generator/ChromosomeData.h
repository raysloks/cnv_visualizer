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

    std::vector<CoverageData> log2_coverage_data;
    int log2_coverage_offset;

    std::vector<BafData> baf_data;
    int baf_offset;

    ChromosomeData shrink() const;
    int save(const std::string& path) const;
};
