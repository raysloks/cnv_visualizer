#pragma once

#include <set>
#include <string>
#include <map>

struct SetPP
{
	std::map<std::string, int64_t> AnnotationData::*lookup;
	char Annotation::*index;
	size_t index_size;
};

struct PrefixPP
{
	std::map<std::string, int64_t> AnnotationData::*lookup;
	int64_t AnnotationData::*max;
	char Annotation::*value;
	size_t value_size;
};

struct NumPP
{
	int64_t AnnotationData::*max;
	char Annotation::*value;
	size_t value_size;
};

#define CAT(A, B) A ## B

#define SET_PP(x) std::pair<std::string, SetPP>(#x, SetPP { &AnnotationData::CAT(x, s), (char Annotation::*)&Annotation::x, sizeof(Annotation::x) })
#define PREFIX_PP(x) std::pair<std::string, PrefixPP>(#x, PrefixPP { &AnnotationData::CAT(x, _prefixes), &AnnotationData::CAT(max_, x), (char Annotation::*)&Annotation::x, sizeof(Annotation::x) })
#define NUM_PP(x) std::pair<std::string, NumPP>(#x, NumPP { &AnnotationData::CAT(max_, x), (char Annotation::*)&Annotation::x, sizeof(Annotation::x) })

const std::map<std::string, SetPP> set_map = {
	SET_PP(gene_name),
	SET_PP(gene_source),
	SET_PP(gene_biotype),
	SET_PP(transcript_source),
	SET_PP(transcript_biotype),
	SET_PP(tag)
};

const std::map<std::string, PrefixPP> prefix_map = {
	PREFIX_PP(gene_id),
	PREFIX_PP(transcript_id),
	PREFIX_PP(havana_transcript),
	PREFIX_PP(exon_id),
	PREFIX_PP(ccds_id),
	PREFIX_PP(protein_id)
};

const std::map<std::string, NumPP> num_map = {
	NUM_PP(gene_version),
	NUM_PP(transcript_version),
	NUM_PP(exon_number),
	NUM_PP(havana_transcript_version),
	NUM_PP(exon_version),
	NUM_PP(protein_version)
};
