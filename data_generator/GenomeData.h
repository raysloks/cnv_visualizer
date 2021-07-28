#pragma once

#include <vector>

#include "ChromosomeData.h"

class GenomeData
{
public:
    void save(const std::string& path, const std::string& template_path) const;

    ChromosomeData& getChromosomeByName(const std::string& name);

	std::string name;

    std::vector<ChromosomeData> chromosomes;
};
