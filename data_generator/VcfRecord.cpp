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

	{
		size_t i = 0;
		auto&& line = fields[8];
		while (i < line.size())
		{
			size_t start = i;
			do ++i; while (i < line.size() && line[i] != ':');
			std::string fmt_id = line.substr(start, i - start);
			++i;
			format.push_back(fmt_id);
		}
	}

	for (size_t j = 9; j < fields.size(); ++j)
	{
		samples.resize(samples.size() + 1);
		size_t i = 0;
		auto&& line = fields[j];
		while (i < line.size())
		{
			size_t start = i;
			do ++i; while (i < line.size() && line[i] != ':');
			std::string sample_data = line.substr(start, i - start);
			++i;
			samples.back().push_back(sample_data);
		}
	}
}

std::string VcfRecord::getString() const
{
	std::string str = chrom + "\t";
	str += std::to_string(pos) + "\t";
	str += id + "\t";
	str += ref + "\t";
	str += alt + "\t";
	if (qual < 9000.0f)
		str += ".\t";
	else
		str += std::to_string(qual) + "\t";
	bool not_first = false;
	if (info.empty())
		str += ".";
	for (auto& kv : info)
	{
		if (not_first)
			str += ";";
		str += kv.first + "=" + kv.second;
		not_first = true;
	}
	if (format.size() > 0)
	{
		str += "\t";
		for (size_t i = 0; i < format.size(); ++i)
		{
			if (i != 0)
				str += ":";
			str += format[i];
		}
		for (auto& sample : samples)
		{
			str += "\t";
			for (size_t i = 0; i < sample.size(); ++i)
			{
				if (i != 0)
					str += ":";
				str += sample[i];
			}
		}
	}
	return str;
}

std::vector<float> VcfRecord::getFloatInfoVector(const std::string& key) const
{
	std::vector<float> result;
	auto it = info.find(key);
	if (it == info.end())
		return result;
	std::string s = it->second;
	if (s == ".")
		return result;
	while (s.size() > 0)
	{
		size_t index = s.find(',');
		std::string sub = s.substr(0, index);
		float value = -9999.0f;
		try
		{
			value = std::stof(sub);
		}
		catch ( ... )
		{
		}
		result.push_back(value);
		if (index == std::string::npos)
			break;
		s = s.substr(index + 1);
	}
	return result;
}

size_t VcfRecord::getFormatIndex(const std::string& fmt_id) const
{
	for (size_t i = 0; i < format.size(); ++i)
		if (format[i] == fmt_id)
			return i;
	return std::string::npos;
}

std::string VcfRecord::getSampleData(const std::string& fmt_id) const
{
	if (samples.empty())
		return "";
	auto& sample = samples[0];

	size_t fmt_index = getFormatIndex(fmt_id);
	if (fmt_index >= sample.size())
		return "";
	
	return sample[fmt_index];
}
