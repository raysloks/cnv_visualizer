#include "AnnotationData.h"

#include <thread>
#include <fstream>
#include <iostream>

AnnotationData::AnnotationData()
{
	max_gene_version = -1;
	max_transcript_version = -1;
	max_exon_number = -1;
	max_havana_transcript_version = -1;
	max_exon_version = -1;
}

#define PRINT_MAX(x) std::cout << #x": " << x << std::endl;
#define PRINT_SET(x) std::cout << #x": " << x.size() << std::endl; for (auto& kv : x) std::cout << "\t" << kv << std::endl;

void AnnotationData::save(const std::string& path) const
{
	PRINT_SET(gene_id_prefixes)
	PRINT_SET(transcript_id_prefixes)
	PRINT_SET(gene_names)
	PRINT_SET(gene_sources)
	PRINT_SET(gene_biotypes)
	PRINT_SET(transcript_names)
	PRINT_SET(transcript_sources)
	PRINT_SET(transcript_biotypes)
	PRINT_SET(havana_transcript_prefixes)
	PRINT_SET(exon_id_prefixes)
	PRINT_SET(tags)
	
	PRINT_MAX(max_gene_version)
	PRINT_MAX(max_transcript_version)
	PRINT_MAX(max_exon_number)
	PRINT_MAX(max_havana_transcript_version)
	PRINT_MAX(max_exon_version)
	PRINT_MAX(max_protein_version)

	PRINT_SET(unknown)

	std::vector<std::thread> threads;
	for (auto& chr : annotations)
		threads.emplace_back([=]() {
			std::string fp = path + chr.first + ".ann";
			std::ofstream f(fp, std::ofstream::binary | std::ofstream::trunc);

			uint32_t size = chr.second.size();
			f.write((char*)&size, sizeof(size));

			f.write((char*)chr.second.data(), sizeof(chr.second[0]) * chr.second.size());
		});
	for (auto& thread : threads)
		thread.join();
}
