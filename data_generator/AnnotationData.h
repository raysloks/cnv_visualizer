#pragma once

#include <map>
#include <vector>
#include <set>

#include "Annotation.h"

class AnnotationData
{
public:
	AnnotationData();

    void save(const std::string& path);

	std::map<std::string, std::vector<Annotation>> annotations;

	std::map<std::string, int64_t> gene_names, gene_sources, gene_biotypes, transcript_names, transcript_sources, transcript_biotypes, tags;
	std::map<std::string, int64_t> gene_id_prefixes, transcript_id_prefixes, havana_transcript_prefixes, exon_id_prefixes, ccds_id_prefixes, protein_id_prefixes;
	int64_t max_gene_version, max_transcript_version, max_exon_number, max_havana_transcript_version, max_exon_version, max_protein_version;
	int64_t max_gene_id, max_transcript_id, max_havana_transcript, max_exon_id, max_ccds_id, max_protein_id;
	int64_t max_transcript_name_suffix;
	std::map<std::string, int64_t> unknown;
};
