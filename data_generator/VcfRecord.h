#pragma once

#include <vector>
#include <map>
#include <string>

class VcfRecord
{
public:
	VcfRecord(const std::string& line);

	std::string getString() const;

	std::vector<float> getFloatInfoVector(const std::string& key) const;

	size_t getFormatIndex(const std::string& fmt_id) const;
	std::string getSampleData(const std::string& fmt_id) const;

	std::vector<std::string> fields;
	std::string chrom;
	int pos;
	std::string id, ref, alt;
	float qual;
	std::string filter;
	std::map<std::string, std::string> info;
	std::vector<std::string> format;
	std::vector<std::vector<std::string>> samples;
};
