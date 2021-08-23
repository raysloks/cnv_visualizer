#include "BamLoader.h"

#include <iostream>

#include <htslib/hts.h>
#include <htslib/sam.h>

int BamLoader::load(GenomeData& data, const std::string& fname)
{
	htsFile * fp = hts_open(fname.c_str(), "r");

	sam_hdr_t * header = sam_hdr_read(fp);

	//std::cout << header->text << std::endl;

	char base_int_to_char[16] = {};
	base_int_to_char[1] = 'A';
	base_int_to_char[2] = 'C';
	base_int_to_char[4] = 'G';
	base_int_to_char[8] = 'T';
	base_int_to_char[15] = 'N';

	bam1_t * record = bam_init1();

	for (auto& chr : data.chromosomes)
		chr.base_data.resize(chr.log2_coverage_data.size());

	size_t notify_counter = 0;

	while (sam_read1(fp, header, record) >= 0)
	{
		bool print_cigar_and_sequence = ++notify_counter % 10000000 == 0;
		if (print_cigar_and_sequence)
			std::cout << record->core.tid << ":" << record->core.pos << std::endl;

		if (record->core.tid >= data.chromosomes.size())
			break;
		ChromosomeData * chr_data = &data.chromosomes[record->core.tid];
		hts_pos_t pos = record->core.pos;
		size_t index = 0;
		uint8_t * sequence = bam_get_seq(record);
		for (size_t i = 0; i < record->core.n_cigar; ++i)
		{
			int32_t cigar = bam_get_cigar(record)[i];
			int32_t len = cigar >> 4;
			int32_t op = cigar & 0b1111;
			char op_chars[] = "MIDNSHP=X";
			if (print_cigar_and_sequence)
				std::cout << len << op_chars[op];
			switch (op)
			{
			case 0: // M
				for (size_t i = 0; i < len; ++i)
				{
					int64_t effective_pos = (pos - chr_data->offset) / chr_data->scale;
					if (effective_pos >= 0 && effective_pos < chr_data->base_data.size())
						chr_data->base_data[effective_pos] += base_int_to_char[bam_seqi(sequence, index)];
					++pos;
				}
				index += len;
				break;
			case 1: // I
				index += len;
				break;
			case 2: // D
				pos += len;
				break;
			case 3: // N
				break;
			case 4: // S
				pos += len;
				index += len;
				break;
			case 5: // H
				pos += len;
				break;
			case 6: // P
				break;
			case 7: // =
				break;
			case 8: // X
				break;
			default:
				break;
			}
		}
		if (print_cigar_and_sequence)
		{
			std::cout << std::endl;
			for (size_t i = 0; i < record->core.l_qseq; ++i)
			{
				int base = bam_seqi(bam_get_seq(record), i);
				switch (base)
				{
				case 1:
					std::cout << 'A';
					break;
				case 2:
					std::cout << 'C';
					break;
				case 4:
					std::cout << 'G';
					break;
				case 8:
					std::cout << 'T';
					break;
				case 15:
					std::cout << 'N';
					break;
				default:
					std::cout << '.';
					break;
				}
			}
			std::cout << std::endl;
		}
	}

	bam_destroy1(record);

	sam_hdr_destroy(header);

	hts_close(fp);

	return 0;
}
