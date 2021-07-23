#include "CoverageDataLoader.h"

#include <map>
#include <iostream>

#include "GenomeData.h"

CoverageDataLoader::CoverageDataLoader(int scale) : scale(scale)
{
    
}

int CoverageDataLoader::load(GenomeData& data, std::istream& is)
{
    std::map<std::string, std::map<int, float>> coverage_data;

    std::string line;
    while (std::getline(is, line))
    {
        if (line[0] != '@')
            break;
    }

    bool has_warned_scale_mismatch = false;

    std::string last_chr;

    while (is.good())
    {
        std::string chr_name;
        int reg_start, reg_end;
        float coverage;
        is >> chr_name >> reg_start >> reg_end >> coverage;
        if (is.eof())
            break;

        if (last_chr != chr_name)
        {
            data.getChromosomeByName(chr_name);
            std::cout << chr_name << std::endl;
        }
        last_chr = chr_name;

        coverage_data[chr_name][(reg_start + reg_end) / 2] = coverage;

        if (!has_warned_scale_mismatch && reg_end - reg_start != scale - 1)
        {
            std::cerr << "WARNING: scale mismatch, stopping read." << std::endl;
            // scale = reg_end - reg_start + 1;
            break;
            has_warned_scale_mismatch = true;
        }
    }

    for (auto& chr : coverage_data)
    {
        std::cout << chr.first << std::endl;
        auto& chr_data = data.getChromosomeByName(chr.first);
        chr_data.scale = scale;
        chr_data.log2_coverage_offset = chr.second.begin()->first;
        int size = (chr.second.rbegin()->first - chr.second.begin()->first) / scale + 1;
        std::cout << size << std::endl;
        if (size <= 0)
        {
            std::cerr << "WARNING: invalid chromosome size, ignoring." << std::endl;
            continue;
        }
        chr_data.log2_coverage_data.resize(size);
        for (auto& cov : chr.second)
        {
            int index = (cov.first - chr.second.begin()->first) / scale;
            if (index >= 0 && index < chr_data.log2_coverage_data.size())
                chr_data.log2_coverage_data[index] = cov.second;
        }
    }

    return 0;
}
