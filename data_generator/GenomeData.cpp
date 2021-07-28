#include "GenomeData.h"

#include <fstream>

#include "DocumentTemplate.h"

void GenomeData::save(const std::string& path, const std::string& template_path) const
{
	std::ifstream is(template_path);
	DocumentTemplate doc(is);

	doc.data["title"] = name;
	for (size_t i = 0; i < chromosomes.size(); ++i)
	{
		std::string chr_var = "chromosomes[" + std::to_string(i) + "].";
		doc.data[chr_var + "name"] = chromosomes[i].chr_template.name;
		doc.data[chr_var + "offset"] = std::to_string(chromosomes[i].log2_coverage_offset);
		doc.data[chr_var + "size"] = std::to_string(chromosomes[i].log2_coverage_data.size() * chromosomes[i].scale);
	}

    std::ofstream f(path + "index.html", std::ofstream::trunc);
	doc.parse(f);
	f.close();

    for (auto& chr : chromosomes)
        chr.save(path);
}

ChromosomeData& GenomeData::getChromosomeByName(const std::string& name)
{
    for (auto& chr : chromosomes)
        if (chr.chr_template.name == name)
            return chr;

    chromosomes.push_back(ChromosomeData{ChromosomeTemplate{name}});

    return chromosomes.back();
}
