#include "BamLoader.h"

#include <iostream>

#include <htslib/hts.h>
#include <htslib/sam.h>

int BamLoader::load(GenomeData& data, const std::string& fname)
{
    htsFile * fp = hts_open(fname.c_str(), "r");

    sam_hdr_t * header = sam_hdr_read(fp);

    std::cout << header->text << std::endl;

    bam1_t * record = bam_init1();

    int count[16] = {};
    while (sam_read1(fp, header, record) >= 0)
    {
        //std::cout << record->core.pos << std::endl;
        hts_pos_t pos = record->core.pos;
        size_t index = 0;
        for (size_t i = 0; i < record->core.n_cigar; ++i)
        {
            int32_t cigar = bam_get_cigar(record)[i];
            int32_t len = cigar >> 4;
            int32_t op = cigar & 0b1111;
            //std::cout << len;
            char op_chars[] = "MIDNSHP=X";
            //std::cout << op_chars[op];
            switch (op)
            {
            case 0: // M
                for (size_t i = 0; i < len; ++i)
                {
                    
                }
                pos += len;
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
        /*std::cout << '\t';
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
        std::cout << std::endl;*/
    }

    bam_destroy1(record);

    sam_hdr_destroy(header);

    hts_close(fp);

    return 0;
}
