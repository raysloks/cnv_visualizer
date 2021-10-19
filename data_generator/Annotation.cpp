#include "Annotation.h"

#include "Utility.h"
#include "AnnotationData.h"

#include <iostream>
#include <cstring>

#include "AnnotationDataDefinitions.h"

std::string remove_id_prefix(const std::string& string)
{
	for (size_t i = 0; i < string.size(); ++i)
		if (isdigit(string[i]))
			return string.substr(i);
	return "";
}

std::string get_id_prefix(const std::string& string)
{
	for (size_t i = 0; i < string.size(); ++i)
		if (isdigit(string[i]))
			return string.substr(0, i);
	return string;
}

std::string remove_transcript_suffix(const std::string& string)
{
	for (size_t i = string.size() - 1; i >= 0; --i)
		if (string[i] == '-')
			return string.substr(0, i);
	return string;
}

std::string get_transcript_suffix(const std::string& string)
{
	for (size_t i = string.size() - 1; i >= 0; --i)
		if (string[i] == '-')
			return string.substr(i + 1);
	return "";
}

Annotation::Annotation(const std::vector<std::string>& fields, AnnotationData& data)
{
	memset(this, 0xff, sizeof(Annotation));

	start = std::stoi(fields[3]);
	end = std::stoi(fields[4]);

	auto extras = split(fields[8], ';');
	for (auto& extra : extras)
	{
		auto kv = split(extra, ' ');
		auto key = trim(kv[0]);
		auto value = unwrap(kv[1]);

		if (key == "transcript_name")
		{
			transcript_name = data.transcript_names[remove_transcript_suffix(value)];
			transcript_name_suffix = std::stoi(get_transcript_suffix(value));
			continue;
		}
		auto find_set = set_map.find(key);
		if (find_set != set_map.end())
		{
			int64_t index = (data.*find_set->second.lookup)[value];
			memcpy(&(this->*find_set->second.index), &index, find_set->second.index_size);
			continue;
		}
		auto find_prefix = prefix_map.find(key);
		if (find_prefix != prefix_map.end())
		{
			int64_t ivalue = std::stoi(remove_id_prefix(value));
			memcpy(&(this->*find_prefix->second.value), &ivalue, find_prefix->second.value_size);
			continue;
		}
		auto find_num = num_map.find(key);
		if (find_num != num_map.end())
		{
			int64_t ivalue = std::stoi(value);
			memcpy(&(this->*find_num->second.value), &ivalue, find_num->second.value_size);
			continue;
		}
	}
}

int Annotation::prepare(const std::vector<std::string>& fields, AnnotationData& data)
{
	auto extras = split(fields[8], ';');
	for (auto& extra : extras)
	{
		auto kv = split(extra, ' ');
		auto key = trim(kv[0]);
		auto value = unwrap(kv[1]);

		if (key == "transcript_name")
		{
			data.transcript_names[remove_transcript_suffix(value)] = -1;
			continue;
		}
		auto find_set = set_map.find(key);
		if (find_set != set_map.end())
		{
			(data.*find_set->second.lookup)[value] = -1;
			continue;
		}
		auto find_prefix = prefix_map.find(key);
		if (find_prefix != prefix_map.end())
		{
			(data.*find_prefix->second.lookup)[get_id_prefix(value)] = -1;
			int64_t ivalue = std::stoi(remove_id_prefix(value));
			if (ivalue > data.*find_prefix->second.max)
				data.*find_prefix->second.max = ivalue;
			continue;
		}
		auto find_num = num_map.find(key);
		if (find_num != num_map.end())
		{
			int64_t ivalue = std::stoi(value);
			if (ivalue > data.*find_num->second.max)
				data.*find_num->second.max = ivalue;
			continue;
		}
		data.unknown[key] = -1;
	}
	return 0;
}
