#include "Annotation.h"

#include "Utility.h"
#include "AnnotationData.h"

#include <iostream>

#define CAT(A, B) A ## B

#define SET_PAIR(x) std::pair<std::string, std::set<std::string> AnnotationData::*>(#x, &AnnotationData::CAT(x, s))
#define NUM_PAIR(x) std::pair<std::string, int64_t AnnotationData::*>(#x, &AnnotationData::CAT(max_, x))

Annotation::Annotation(const std::string& line, AnnotationData& data)
{
	auto fields = split(line, '\t');
	start = std::stoi(fields[3]);
	end = std::stoi(fields[4]);

	std::map<std::string, std::set<std::string> AnnotationData::*> set_map = {
		SET_PAIR(gene_name),
		SET_PAIR(gene_source),
		SET_PAIR(gene_biotype),
		SET_PAIR(transcript_name),
		SET_PAIR(transcript_source),
		SET_PAIR(transcript_biotype),
		SET_PAIR(tag)
	};

	std::map<std::string, int64_t AnnotationData::*> num_map = {
		NUM_PAIR(gene_version),
		NUM_PAIR(transcript_version),
		NUM_PAIR(exon_number),
		NUM_PAIR(havana_transcript_version),
		NUM_PAIR(exon_version),
		NUM_PAIR(protein_version)
	};

	auto extras = split(fields[8], ';');
	for (auto& extra : extras)
	{
		auto kv = split(extra, ' ');
		auto key = trim(kv[0]);
		auto value = unwrap(kv[1]);

		auto find_set = set_map.find(key);
		if (find_set != set_map.end())
		{
			(data.*find_set->second).insert(value);
			continue;
		}
		auto find_num = num_map.find(key);
		if (find_num != num_map.end())
		{
			int64_t ivalue = std::stoi(value);
			if (ivalue > data.*find_num->second)
				data.*find_num->second = ivalue;			
			continue;
		}
		data.unknown.insert(key);
	}
}
