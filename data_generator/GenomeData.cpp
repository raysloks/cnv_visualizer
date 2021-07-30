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

	doc.data["chunk_size"] = std::to_string(2048);
	std::vector<std::string> data_lines = {
		"coverage_density",
		"coverage_mean",
		"coverage_min",
		"coverage_max",
		"baf_total_density",
		"baf_top_density",
		"baf_mid_density",
		"baf_bot_density",
		"baf_mid_mean"
	};
	for (size_t i = 0; i < data_lines.size(); ++i)
	{
		doc.data["data_lines[" + std::to_string(i) + "]"] = data_lines[i];
	}

    std::ofstream f(path + "index.html", std::ofstream::trunc);
	doc.parse(f);
	f.close();

	{
		std::ofstream f(path + "calls.json", std::ofstream::trunc);
		Coal coal = getCallData();
		coal.printJson(f);
	}

    for (auto& chr : chromosomes)
        chr.save(path);
}

Coal GenomeData::getCallData() const
{
	Coal coal = std::map<std::string, Coal>();
		
	coal["meta"] = std::map<std::string, Coal>();
	coal["chromosomes"] = std::map<std::string, Coal>();

	Coal& meta = coal["meta"];
	Coal& chromosomes = coal["chromosomes"];

	for (auto& record : vcf.records)
	{
		Coal coal;
		coal.type = Coal::Type::Object;
		coal["chrom"] = record.chrom;
		coal["pos"] = record.pos;
		coal["id"] = record.id;
		coal["ref"] = record.ref;
		coal["alt"] = record.alt;
		coal["qual"] = record.qual;
		coal["filter"] = record.filter;
		coal["info"] = std::map<std::string, Coal>();
		Coal& info = coal["info"];
		for (auto& pair : record.info)
		{
			info[pair.first] = pair.second;
		}
		if (chromosomes.members.find(record.chrom) == chromosomes.members.end())
		{
			chromosomes[record.chrom] = std::map<std::string, Coal>();
			chromosomes[record.chrom]["records"] = std::vector<Coal>();
		}
		chromosomes[record.chrom]["records"].elements.emplace_back(std::move(coal));
	}

	return coal;
}

ChromosomeData& GenomeData::getChromosomeByName(const std::string& name)
{
    for (auto& chr : chromosomes)
        if (chr.chr_template.name == name)
            return chr;

    chromosomes.push_back(ChromosomeData{ChromosomeTemplate{name}});

    return chromosomes.back();
}
