#pragma once

#include <vector>

#include "ChromosomeData.h"
#include "VcfData.h"

#include "Coal.h"

class GenomeData
{
public:
    void save(const std::string& path, const std::string& template_path) const;

	int addBafData(const VcfData& baf);

	Coal getCallData() const;

    ChromosomeData& getChromosomeByName(const std::string& name);

	std::string name;

    std::vector<ChromosomeData> chromosomes;

	VcfData vcf;
};
