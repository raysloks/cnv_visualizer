#include "VcfLoader.h"

#include <iostream>

int VcfLoader::load(VcfData& data, std::istream& is)
{
	std::string line;
	while (std::getline(is, line))
	{
		if (line.substr(0, 2) == "##")
		{
			size_t pos = line.find_first_of('=');
			std::string key = line.substr(2, pos - 2);
			std::string value = line.substr(pos + 1);
			data.meta[key] = value;
		}
		else if (line[0] == '#')
		{
			
		}
		else
		{
			VcfRecord record(line);
			
			data.records.emplace_back(std::move(record));
		}
	}
	std::cout << "loaded " << data.records.size() << " records." << std::endl;

	return 0;
}
