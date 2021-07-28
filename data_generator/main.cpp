#include <iostream>
#include <fstream>
#include <vector>

#include "ChromosomeTemplate.h"
#include "ChromosomeData.h"
#include "FakeDataGenerator.h"
#include "GenomeData.h"
#include "CoverageDataLoader.h"

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

    if (argc < 3)
        return 1;

    GenomeData data;

    CoverageDataLoader loader(100);

    std::ifstream is(argv[1]);
    loader.load(data, is);
    
    std::string path = argv[1];
    path = path.substr(0, path.find_last_of('/'));
	data.name = path.substr(path.find_last_of('/') + 1);
    path += "/vis/";

    data.save(path, argv[2]);

    return 0;
}
