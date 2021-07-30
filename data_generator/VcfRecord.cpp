#include "VcfRecord.h"

#include <sstream>

VcfRecord::VcfRecord(const std::string& line)
{
	size_t i = 0;
	while (i < line.size())
	{
		if (line[i] == '\t')
			++i;
		else
		{
			size_t start = i;
			do ++i; while (i < line.size() && line[i] != '\t');
			fields.push_back(line.substr(start, i - start));
		}
	}
	if (fields.size() < 8)
		throw std::runtime_error("missing fields in vcf record.");

	chrom = fields[0];
	pos = std::stoi(fields[1]);
	id = fields[2];
	ref = fields[3];
	alt = fields[4];
	if (fields[5] != ".")
		qual = std::stof(fields[5]);
	else
		qual = -9999.0f;
	filter = fields[6];

	{
		size_t i = 0;
		auto&& line = fields[7];
		while (i < line.size())
		{
			size_t start = i;
			do ++i; while (i < line.size() && line[i] != ';');
			std::string kv_pair = line.substr(start, i - start);
			++i;
			size_t split = kv_pair.find_first_of('=');
			std::string key = kv_pair.substr(0, split);
			std::string value = kv_pair.substr(split + 1);
			info[key] = value;
		}
	}
}
