#include <iostream>
#include <fstream>
#include <vector>

#include "ChromosomeTemplate.h"
#include "ChromosomeData.h"
#include "FakeDataGenerator.h"
#include "GenomeData.h"
#include "CoverageDataLoader.h"
#include "VcfLoader.h"
#include "BamLoader.h"
#include "CmdOption.h"

#include "Coal.h"

std::vector<ChromosomeTemplate> chr_templates;

int load_chr_templates()
{
    std::cout << "loading chromosome sizes..." << std::endl;

    std::ifstream f("chrom_sizes38.tsv");

    if (!f.good())
    {
        std::cout << "failed to open chromosome sizes file." << std::endl;
        return 1;
    }

    while (!f.eof())
    {
        std::string name;
        int size;
        f >> name >> size;
        if (f.eof())
            break;
        chr_templates.push_back(ChromosomeTemplate{name, size});
    }

    std::cout << "chromosome sizes loaded." << std::endl;

    return 0;
}

int main(int argc, char ** argv)
{
	/* Coal coal(100.0f);
	std::ofstream os("test.json");
	coal.printJson(os);
	return 0; */

	// GenomeData test_data;
	// test_data.save("../data/subjects/BAB1692-R/vis/");
	// return 0;

    // if (load_chr_templates())
    // {
    //     std::cout << "failed to load chromosome sizes." << std::endl;
    //     return 1;
    // }

    // FakeDataGenerator fdg;

    // for (auto& chr : chr_templates)
    // {
    //     std::cout << "generating fake chromosome " << chr.name << "..." << std::endl;
    //     ChromosomeData chr_data = fdg.generateFakeChromosomeData(chr, 100);
    //     std::cout << "fake chromosome " << chr.name << " generated." << std::endl;

    //     chr_data.save("vscode/data/");
    // }

	std::string filter_vcf_output;
	std::string filter_vcf_key;
	float filter_vcf_cutoff;

	std::string baf_calls;
	std::string baf_filter;

	std::string bam_fname;

	std::vector<CmdOption> options = {
		CmdOption('f', "filter-vcf", "filter a gVCF file.", "long desc here yada yada", "help and example here", std::vector<CmdValue>({ CmdValue(&filter_vcf_output, true), CmdValue(&filter_vcf_key, false), CmdValue(&filter_vcf_cutoff) })),
		CmdOption('b', "baf", "use base allele frequency as well.", "long desc", "help and example", std::vector<CmdValue>({ CmdValue(&baf_calls, true), CmdValue(&baf_filter, true) })),
		CmdOption('k', "bam-test", "do htslib test.", "long desc", "help and example", std::vector<CmdValue>({ CmdValue(&bam_fname, true) }))
	};

	std::string tsv_path, html_path;
	std::vector<std::string> vcf_paths;

	for (size_t i = 0; i < argc; ++i)
	{
		std::string arg = argv[i];

		if (arg[0] == '-' && arg.size() >= 2)
		{
			std::vector<CmdOption*> matches;
			if (arg[1] == '-')
			{
				for (auto& option : options)
					if (option.flag_long == arg.substr(2))
					{
						matches.push_back(&option);
						break;
					}
				if (matches.empty())
					std::cerr << "ERROR: unrecognized option '" << arg << "'." << std::endl;
			}
			else
			{
				for (size_t i = 1; i < arg.size(); ++i)
				{
					bool found = false;
					for (auto& option : options)
						if (option.flag_short == arg[i])
						{
							matches.push_back(&option);
							found = true;
							break;
						}
					if (!found)
						std::cerr << "ERROR: unrecognized option '-" << arg[i] << "'." << std::endl;
				}
			}
			for (auto& match : matches)
			{
				for (auto& value : match->values)
				{
					if (value.needsValue())
					{
						++i;
						if (i >= argc)
						{
							std::cerr << "ERROR: missing argument '' for option ''." << std::endl;
							break;
						}
						value.parseValue(argv[i]);
					}
					else
						value.parseValue(std::string());
				}
			}
		}

		size_t index = arg.find_last_of('.');
		if (index != std::string::npos)
		{
			std::string extension = arg.substr(index);
			for (auto& c : extension)
				c = std::tolower(c);
			if (extension == ".tsv")
				tsv_path = arg;
			if (extension == ".vcf")
				vcf_paths.push_back(arg);
			if (extension == ".html")
				html_path = arg;
		}
	}

	if (bam_fname.size())
	{
		BamLoader bam_loader;
		GenomeData data;
		bam_loader.load(data, bam_fname);
		return 0;
	}

	if (filter_vcf_output.size())
	{
		std::ofstream vcf_os(filter_vcf_output, std::ofstream::binary | std::ofstream::trunc);
		VcfLoader vcf_loader;
		vcf_loader.baf_key = filter_vcf_key;
		vcf_loader.baf_cutoff = filter_vcf_cutoff;
		vcf_loader.filterBaf(std::cin, vcf_os);
	}

	if (tsv_path.size())
	{
		GenomeData data;

		CoverageDataLoader cov_loader(100);
		std::ifstream tsv_is(tsv_path);
		cov_loader.load(data, tsv_is);

		VcfLoader vcf_loader;

		for (auto& path : vcf_paths)
		{
			std::ifstream vcf_is(path);
			vcf_loader.load(data.vcf, vcf_is);
		}

		if (baf_calls.size())
		{
			std::ifstream baf_calls_is(baf_calls);
			std::ifstream baf_filter_is(baf_filter);
			vcf_loader.filterBafData(data, baf_filter_is, baf_calls_is);
		}

		std::string path = tsv_path;
		path = path.substr(0, path.find_last_of('/'));
		data.name = path.substr(path.find_last_of('/') + 1);
		path += "/vis/";

		data.save(path, html_path);
	}

    return 0;
}
