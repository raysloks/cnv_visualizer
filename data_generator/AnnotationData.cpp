#include "AnnotationData.h"

#include <thread>
#include <fstream>
#include <iostream>

#include "Coal.h"

#include "AnnotationDataDefinitions.h"

AnnotationData::AnnotationData()
{
	max_gene_version = -1;
	max_transcript_version = -1;
	max_exon_number = -1;
	max_havana_transcript_version = -1;
	max_exon_version = -1;

	max_gene_id = -1;
	max_transcript_id = -1;
	max_havana_transcript = -1;
	max_exon_id = -1;
	max_ccds_id = -1;
	max_protein_id = -1;
	
	max_transcript_name_suffix = -1;
}

#define PRINT_NUM(x) std::cout << #x": " << x << std::endl;
#define ADD_SET(x) coal[#x] = Coal(std::vector<Coal>()); for (auto& kv : x) coal[#x].elements.push_back(kv.first);

void AnnotationData::save(const std::string& path) const
{
	PRINT_NUM(max_gene_id)
	PRINT_NUM(max_transcript_id)
	PRINT_NUM(max_havana_transcript)
	PRINT_NUM(max_exon_id)
	PRINT_NUM(max_ccds_id)
	PRINT_NUM(max_protein_id)
	PRINT_NUM(max_transcript_name_suffix)

	auto effective_num_map = num_map;
	effective_num_map["start"] = NumPP { nullptr, (char Annotation::*)&Annotation::start, sizeof(Annotation::start) };
	effective_num_map["end"] = NumPP { nullptr, (char Annotation::*)&Annotation::end, sizeof(Annotation::end) };

	{
		Coal coal;
		coal.type = Coal::Type::Object;

		ADD_SET(gene_names)
		ADD_SET(gene_sources)
		ADD_SET(gene_biotypes)
		ADD_SET(transcript_names)
		ADD_SET(transcript_sources)
		ADD_SET(transcript_biotypes)
		ADD_SET(tags)
		
		ADD_SET(gene_id_prefixes)
		ADD_SET(transcript_id_prefixes)
		ADD_SET(havana_transcript_prefixes)
		ADD_SET(exon_id_prefixes)
		ADD_SET(ccds_id_prefixes)
		ADD_SET(protein_id_prefixes)

		ADD_SET(unknown)

		coal["data_sets"] = Coal(std::vector<Coal>());
		for (auto& kv : set_map)
		{
			Coal data_line;
			data_line.type = Coal::Type::Object;
			data_line["name"] = kv.first;
			data_line["size"] = kv.second.index_size;
			coal["data_sets"].elements.push_back(data_line);
		}
		coal["data_prefixes"] = Coal(std::vector<Coal>());
		for (auto& kv : prefix_map)
		{
			Coal data_line;
			data_line.type = Coal::Type::Object;
			data_line["name"] = kv.first;
			data_line["size"] = kv.second.value_size;
			coal["data_prefixes"].elements.push_back(data_line);
		}
		coal["data_values"] = Coal(std::vector<Coal>());
		for (auto& kv : effective_num_map)
		{
			Coal data_line;
			data_line.type = Coal::Type::Object;
			data_line["name"] = kv.first;
			data_line["size"] = kv.second.value_size;
			coal["data_values"].elements.push_back(data_line);
		}

		std::string fp = path + "lookup.json";
		std::ofstream f(fp, std::ofstream::trunc);
		coal.printJson(f);
	}

	std::vector<std::thread> threads;
	for (auto& chr : annotations)
		threads.emplace_back([=]() {
			std::string fp = path + chr.first + ".ann";
			std::ofstream f(fp, std::ofstream::binary | std::ofstream::trunc);

			uint64_t size = chr.second.size();
			f.write((char*)&size, sizeof(size));

			uint64_t dummy = 0;

			for (auto& kv : set_map)
			{
				size_t offset = 0;
				for (auto& annotation : chr.second)
				{
					f.write(&(annotation.*kv.second.index), kv.second.index_size);
					offset += kv.second.index_size;
				}
				f.write((char*)&dummy, 7 - (offset - 1) % 8);
			}
			for (auto& kv : prefix_map)
			{
				size_t offset = 0;
				for (auto& annotation : chr.second)
				{
					f.write(&(annotation.*kv.second.value), kv.second.value_size);
					offset += kv.second.value_size;
				}
				f.write((char*)&dummy, 7 - (offset - 1) % 8);
			}
			for (auto& kv : effective_num_map)
			{
				size_t offset = 0;
				for (auto& annotation : chr.second)
				{
					f.write(&(annotation.*kv.second.value), kv.second.value_size);
					offset += kv.second.value_size;
				}
				f.write((char*)&dummy, 7 - (offset - 1) % 8);
			}
		});
	for (auto& thread : threads)
		thread.join();
}
