#include "ChromosomeData.h"

#include <iostream>
#include <fstream>

ChromosomeData ChromosomeData::shrink() const
{
    ChromosomeData chr_data;

    chr_data.chr_template = chr_template;

    chr_data.scale = scale * 2;

    chr_data.baf_offset = baf_offset;
    chr_data.log2_coverage_offset = log2_coverage_offset;

    chr_data.baf_data.resize(baf_data.size() / 2);
    chr_data.log2_coverage_data.resize(log2_coverage_data.size() / 2);

    for (int i = 0; i < chr_data.baf_data.size(); ++i)
    {
        chr_data.baf_data[i] = baf_data[i * 2].combine(baf_data[i * 2 + 1]);
    }

    for (int i = 0; i < chr_data.log2_coverage_data.size(); ++i)
    {
        chr_data.log2_coverage_data[i] = log2_coverage_data[i * 2].combine(log2_coverage_data[i * 2 + 1]);
    }

    return chr_data;
}

int ChromosomeData::save(const std::string& path) const
{
    std::cout << "exporting chromosome " << chr_template.name << "..." << std::endl;

    int max_scale = 200000;

    ChromosomeData shrunk_chr_data = *this; // unnecessary copy, but it's kinda awkward to avoid it
    int expected_scale = shrunk_chr_data.scale;
    
    while (expected_scale < max_scale)
    {
        if (expected_scale >= 100)
        {
            int chunk_size = 2048;
            for (int i = 0; i < shrunk_chr_data.baf_data.size() || i < shrunk_chr_data.log2_coverage_data.size(); i += chunk_size)
            {
                std::string fp = path + chr_template.name + "_" + std::to_string(shrunk_chr_data.scale) + "_" + std::to_string(i / chunk_size) + ".cvd";
                // std::cout << fp << std::endl;
                std::ofstream f(fp, std::ofstream::binary | std::ofstream::trunc);

                f.write((char*)&shrunk_chr_data.scale, sizeof(shrunk_chr_data.scale));

                int baf_offset = shrunk_chr_data.baf_offset + i * shrunk_chr_data.scale;
                f.write((char*)&baf_offset, sizeof(baf_offset));
                int baf_size = std::min(chunk_size, std::max(0, (int)shrunk_chr_data.baf_data.size() - i));
                f.write((char*)&baf_size, sizeof(baf_size));
                f.write((char*)(shrunk_chr_data.baf_data.data() + i), sizeof(BafData) * baf_size);

                int log2_offset = shrunk_chr_data.log2_coverage_offset + i * shrunk_chr_data.scale;
                f.write((char*)&log2_offset, sizeof(log2_offset));
                int log2_size = std::min(chunk_size, std::max(0, (int)shrunk_chr_data.log2_coverage_data.size() - i));
                f.write((char*)&log2_size, sizeof(log2_size));
                f.write((char*)(shrunk_chr_data.log2_coverage_data.data() + i), sizeof(CoverageData) * log2_size);
            }
        }

        expected_scale *= 2;
        if (expected_scale < max_scale)
        {
            shrunk_chr_data = shrunk_chr_data.shrink();
        }
    }

    std::cout << "chromosome " << chr_template.name << " exported." << std::endl;

    return 0;
}
