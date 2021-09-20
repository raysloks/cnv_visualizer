#include <iostream>
#include <fstream>
#include <vector>
#include <thread>

#include <sys/stat.h>

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

	bool help = false;

	std::string create_baf_filter_output;
	std::string create_baf_filter_key;
	float create_baf_filter_cutoff;

	std::string baf_calls;
	std::string baf_filter;

	std::string bam_fname;

	bool test = false;

	std::vector<CmdOption> options = {
		CmdOption('h', "help", "Show this text.", 
			"long help desc", "help help and help example", std::vector<CmdValue>({ CmdValue(&help)})),
		CmdOption('f', "filter-create", "Create a .baf filter file from a genomic .vcf allele database file (such as gnomAD) piped into standard input.", 
			"long desc here yada yada", "help and example here", std::vector<CmdValue>({ CmdValue(&create_baf_filter_output, true), CmdValue(&create_baf_filter_key, false), CmdValue(&create_baf_filter_cutoff) })),
		CmdOption('b', "baf", "Add BAF data to final output using a genomic .vcf calls file and a .baf filter file created with '-f' or '--filter-create'.", 
			"long desc", "help and example", std::vector<CmdValue>({ CmdValue(&baf_calls, true), CmdValue(&baf_filter, true) })),
		CmdOption('s', "staining", "Add nucleotide base frequency data to final output using a .bam file.", 
			"long desc", "help and example", std::vector<CmdValue>({ CmdValue(&bam_fname, true) })),
		CmdOption('t', "test", "Generate fake testing data.", 
			"long desc", "help and example", std::vector<CmdValue>({ CmdValue(&test) }))
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

	if (help)
	{
		for (auto& option : options)
		{
			std::cout << "-" << option.flag_short << ", --" << option.flag_long << std::endl;
			std::cout << "\t" << option.short_description << std::endl;
		}
		return 0;
	}

	if (create_baf_filter_output.size())
	{
		std::ofstream vcf_os(create_baf_filter_output, std::ofstream::binary | std::ofstream::trunc);
		VcfLoader vcf_loader;
		vcf_loader.baf_key = create_baf_filter_key;
		vcf_loader.baf_cutoff = create_baf_filter_cutoff;
		vcf_loader.createBafFilter(std::cin, vcf_os);
	}

	if (tsv_path.size() || test)
	{
		GenomeData data;

		if (tsv_path.size())
		{
			CoverageDataLoader cov_loader(100);
			std::ifstream tsv_is(tsv_path);
			cov_loader.load(data, tsv_is);
		}

		VcfLoader vcf_loader;

		for (auto& path : vcf_paths)
		{
			std::ifstream vcf_is(path);
			vcf_loader.load(data.vcf, vcf_is);
		}

		std::vector<std::thread> threads;

		if (bam_fname.size())
		{
			threads.emplace_back([&]()
			{
				std::cout << "reading bam..." << std::endl;
				BamLoader bam_loader;
				bam_loader.load(data, bam_fname);
				std::cout << "finished reading bam." << std::endl;
			});
		}

		if (baf_calls.size())
		{
			threads.emplace_back([&]()
			{
				std::cout << "filtering baf..." << std::endl;
				std::ifstream baf_calls_is(baf_calls);
				std::ifstream baf_filter_is(baf_filter);
				vcf_loader.filterBafCalls(data, baf_filter_is, baf_calls_is);
				std::cout << "filtered baf." << std::endl;
			});
		}

		for (auto& thread : threads)
			thread.join();

		std::string path = tsv_path;
		path = path.substr(0, path.find_last_of('/'));
		data.name = path.substr(path.find_last_of('/') + 1);
		path += "/vis/";

		std::cout << "creating directories..." << std::endl;
		if (mkdir(path.c_str(), ALLPERMS) == -1)
			std::cerr << "ERROR: failed to create a directory." << std::endl;
		if (mkdir((path + "chunks/").c_str(), ALLPERMS) == -1)
			std::cerr << "ERROR: failed to create a directory." << std::endl;

		std::cout << "saving data..." << std::endl;
		data.save(path, html_path);
		std::cout << "saved data." << std::endl;
	}

    return 0;
}
