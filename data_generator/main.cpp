#include <iostream>
#include <fstream>
#include <vector>

#include "ChromosomeTemplate.h"
#include "ChromosomeData.h"
#include "FakeDataGenerator.h"
#include "GenomeData.h"
#include "CoverageDataLoader.h"
#include "VcfLoader.h"

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

	std::string tsv_path, html_path;
	std::vector<std::string> vcf_paths;

	for (size_t i = 0; i < argc; ++i)
	{
		std::string arg = argv[i];
		std::string extension = arg.substr(arg.find_last_of('.'));
		for (auto& c : extension)
			c = std::tolower(c);
		if (extension == ".tsv")
			tsv_path = arg;
		if (extension == ".vcf")
			vcf_paths.push_back(arg);
		if (extension == ".html")
			html_path = arg;
	}

    GenomeData data;

    CoverageDataLoader cov_loader(100);
    std::ifstream tsv_is(tsv_path);
    cov_loader.load(data, tsv_is);

	for (auto& path : vcf_paths)
	{
		std::ifstream vcf_is(path);
		VcfLoader vcf_loader;
		vcf_loader.load(data.vcf, vcf_is);
	}

    std::string path = tsv_path;
    path = path.substr(0, path.find_last_of('/'));
	data.name = path.substr(path.find_last_of('/') + 1);
    path += "/vis/";

    data.save(path, html_path);

    return 0;
}
