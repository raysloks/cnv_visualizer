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
		doc.data[chr_var + "offset"] = std::to_string(chromosomes[i].offset);
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

int GenomeData::addBafData(const VcfData& baf)
{
	if (chromosomes.empty())
		return 1;
	for (auto& chr : chromosomes)
	{
		chr.baf_data.resize(chr.log2_coverage_data.size());
	}
	ChromosomeData * chr = &chromosomes[0];
	int out_of_bounds_count = 0;
	for (auto& record : baf.records)
	{
		if (record.chrom != chr->chr_template.name)
			chr = &getChromosomeByName(record.chrom);
		int pos = (record.pos - chr->offset) / chr->scale;
		if (pos < 0 || pos >= chr->baf_data.size())
		{
			++out_of_bounds_count;
			continue;
		}
		chr->baf_data[pos] += std::stof(record.info.at("BAF"));
	}
	if (out_of_bounds_count > 0)
		std::cout << "WARNING: " << out_of_bounds_count << " base allele frequency data points were out of bounds. TODO FIX" << std::endl;

	return 0;
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
