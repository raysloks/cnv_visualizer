#pragma once

#include <vector>
#include <string>

class AnnotationData;

class Annotation
{
public:
	Annotation(const std::vector<std::string>& fields, AnnotationData& data);

	static int prepare(const std::vector<std::string>& fields, AnnotationData& data);

	uint32_t start, end;
	uint32_t gene_id, transcript_id, havana_transcript, exon_id, ccds_id, protein_id;
	uint16_t gene_name, transcript_name, transcript_name_suffix, exon_number;
	uint8_t gene_source, gene_biotype, transcript_source, transcript_biotype, tag, gene_version, transcript_version, havana_transcript_version, exon_version, protein_version;
};
