#pragma once

#include <map>
#include <vector>
#include <set>

#include "Annotation.h"

class AnnotationData
{
public:
	AnnotationData();

    void save(const std::string& path) const;

	std::map<std::string, std::vector<Annotation>> annotations;

	std::set<std::string> gene_id_prefixes, transcript_id_prefixes, gene_names, gene_sources, gene_biotypes, transcript_names, transcript_sources, transcript_biotypes, havana_transcript_prefixes, exon_id_prefixes, tags;
	int64_t max_gene_version, max_transcript_version, max_exon_number, max_havana_transcript_version, max_exon_version, max_protein_version;
	std::set<std::string> unknown;
};
