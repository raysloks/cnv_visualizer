#include "GenomeData.h"

#include <fstream>
#include <thread>

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

	int max_scale = 1000000;
	int max_bin_size = chromosomes.front().scale;
	while (max_bin_size * 2 < max_scale)
		max_bin_size *= 2;

	doc.data["chunk_size"] = std::to_string(2048);
	doc.data["base_bin_size"] = std::to_string(chromosomes.front().scale);
	doc.data["max_bin_size"] = std::to_string(max_bin_size);

	std::vector<std::string> data_lines = {
		"coverage_mean_log2",
		"baf_undersampled"
	};
	/*std::vector<std::string> data_lines = {
		"coverage_density_negative",
		"coverage_mean_log2_negative",
		"coverage_density_positive",
		"coverage_mean_log2_positive",
		"coverage_density",
		"coverage_mean_log2",
		"coverage_min_log2",
		"coverage_max_log2",
		"baf_density",
		"baf_mean",
		"baf_mid_density",
		"baf_mid_mean",
		"baf_bot_density",
		"baf_bot_mean",
		"baf_top_density",
		"baf_top_mean",
		"baf_dev_mean",
		"baf_undersampled",
		"base_total_density",
		"base_a_density",
		"base_c_density",
		"base_g_density",
		"base_t_density"
	};*/
	for (size_t i = 0; i < data_lines.size(); ++i)
	{
		doc.data["data_lines[" + std::to_string(i) + "]"] = data_lines[i];
	}

	{
		std::ofstream f(path + "index.html", std::ofstream::trunc);
		doc.parse(f);
	}

	{
		std::ofstream f(path + "calls.json", std::ofstream::trunc);
		getCallData().printJson(f);
	}

	std::vector<std::thread> threads;
	for (auto& chr : chromosomes)
		threads.emplace_back([=]() { chr.save(path + "chunks/", max_scale); });
	for (auto& thread : threads)
		thread.join();
}

const std::vector<std::string> gens_prefixes = {
	"o",
	"a",
	"b",
	"c",
	"d"
};

const std::vector<int64_t> gens_bin_sizes = {
	100000,
	25000,
	5000,
	1000,
	100
};

const std::vector<int64_t> gens_cov_sizes = {
	28000,
	110000,
	550000,
	2700000,
	26400000
};

const std::vector<int64_t> gens_baf_sizes = {
	47000,
	188000,
	754000,
	1900000,
	7500000
};

void GenomeData::save_gens(const std::string& path) const
{
	std::ofstream baf(path + "baf.bed", std::ofstream::trunc);
	for (auto& chr : chromosomes)
	{
		for (size_t i = 0; i < 5; ++i)
		{
			for (size_t x = 0; x < chr.chr_template.size / gens_bin_sizes[i]; ++x)
			{
				int64_t pos = chr.offset + x * gens_bin_sizes[i];
				if (x < chr.baf_data.size())
					baf << gens_prefixes[i] << "_" << chr.chr_template.name << "\t" << (pos - 1) << "\t" << pos << "\t" << chr.baf_data[x].undersampled << "\n";
			}
		}
	}

	std::ofstream cov(path + "cov.bed", std::ofstream::trunc);
	for (auto& chr : chromosomes)
	{
		for (size_t i = 0; i < 5; ++i)
		{
			for (size_t x = 0; x < chr.chr_template.size / gens_bin_sizes[i]; ++x)
			{
				int64_t pos = chr.offset + x * gens_bin_sizes[i];
				if (x < chr.log2_coverage_data.size())
					cov << gens_prefixes[i] << "_" << chr.chr_template.name << "\t" << (pos - 1) << "\t" << pos << "\t" << chr.log2_coverage_data[x].mean_log2 << "\n";
			}
		}
	}
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
