#include "VcfLoader.h"

#include <iostream>
#include <limits>

VcfLoader::VcfLoader()
{
	baf_key = "AF";
	baf_cutoff = 0.05f;
}

int VcfLoader::load(VcfData& data, std::istream& is)
{
	std::string line;
	while (std::getline(is, line))
	{
		if (line.substr(0, 2) == "##")
		{
			size_t pos = line.find_first_of('=');
			std::string key = line.substr(2, pos - 2);
			std::string value = line.substr(pos + 1);
			data.meta.insert(std::make_pair(key, value));
		}
		else if (line[0] == '#')
		{
			
		}
		else
		{
			VcfRecord record(line);
			
			data.records.emplace_back(std::move(record));
		}
	}
	std::cout << "loaded " << data.records.size() << " records." << std::endl;

	return 0;
}

int VcfLoader::filterBaf(std::istream& is, std::ostream& os)
{
	std::cout << "compiling filter..." << std::endl;
	std::cout << "key: " << baf_key << std::endl;
	std::cout << "cutoff: " << baf_cutoff << std::endl;

	int64_t chr_index = -1;
	std::string last_chr_name;

	int64_t allele_count = 0;
	int64_t progress_notify = 0;
	
	float average_freq_accumulator = 0.0f;
	int64_t average_freq_accumulator_divisor = 0;
	
	int64_t info_count = 0;
	int64_t no_info_count = 0;

	std::string line;
	while (std::getline(is, line))
	{
		if (line.substr(0, 2) == "##")
		{
			if (++progress_notify % 100 == 0)
			{
				std::cout << progress_notify << " meta lines so far..." << std::endl;
			}
			size_t pos = line.find_first_of('=');
			std::string key = line.substr(2, pos - 2);
			std::string value = line.substr(pos + 1);
			// os << line << "\n";
		}
		else if (line[0] == '#')
		{
			std::cout << "hit header." << std::endl;
			progress_notify = 0;
			// os << line << "\n";
		}
		else
		{
			if (++progress_notify > 10000)
			{
				progress_notify = 1;
				std::cout << allele_count << " alleles passed the filter so far..." << std::endl;
				if (average_freq_accumulator_divisor > 0)
					std::cout << "average frequency is " << average_freq_accumulator / average_freq_accumulator_divisor << "." << std::endl;
				std::cout << info_count << " | " << no_info_count << std::endl;
			}

			VcfRecord record(line);

			if (last_chr_name != record.chrom)
			{
				++chr_index;
				last_chr_name = record.chrom;
			}

			if (record.ref.size() != 1 || record.alt.size() != 1)
				continue;
			char alt = record.alt[0];
			if (alt == '.')
				continue;
			auto it = record.info.find(baf_key);
			if (it != record.info.end())
			{
				++info_count;
				if (it->second == ".")
					continue;
				float freq = std::stof(it->second);
				if (freq > baf_cutoff)
				{
					++allele_count;
					// this block is read as an int64
					os.put(alt);
					os.write((char*)&record.pos, sizeof(record.pos));
					os.write((char*)&chr_index, 3);
					// --
					os.write((char*)&freq, sizeof(freq));
				}
				if (freq >= 0.0f)
				{
					average_freq_accumulator += freq;
					++average_freq_accumulator_divisor;
				}
			}
			else
			{
				++no_info_count;
			}
		}
	}
	int64_t terminator = std::numeric_limits<int64_t>::max();
	float dummy = 0.0f;
	os.write((char*)&terminator, sizeof(terminator));
	os.write((char*)&dummy, sizeof(dummy));
	std::cout << allele_count << " alleles passed the filter." << std::endl;

	return 0;
}

int VcfLoader::filterBafData(GenomeData& data, std::istream& is_filter, std::istream& is_calls)
{
	std::cout << "filtering..." << std::endl;

	for (auto& chr : data.chromosomes)
	{
		chr.baf_data.resize(chr.log2_coverage_data.size());
	}

	ChromosomeData * chr_data;

	int64_t filter_last_key = 0;
	float filter_last_freq = 0.0f;

	int64_t filter_last_chr_pos = 0;
	float filter_freq_accumulator = 0.0f;
	bool filter_use_ref = false;

	int64_t calls_chr_index = -1;
	std::string calls_last_chr_name;

	int64_t progress_notify = 0;
	int64_t out_of_bounds_count = 0;
	int64_t match_count = 0;
	int64_t ref_count = 0;
	
	std::string line;
	while (std::getline(is_calls, line))
	{
		if (line.substr(0, 2) == "##")
		{
			size_t pos = line.find_first_of('=');
			std::string key = line.substr(2, pos - 2);
			std::string value = line.substr(pos + 1);
		}
		else if (line[0] == '#')
		{
			std::cout << "hit header." << std::endl;
		}
		else
		{
			VcfRecord record(line);

			if (calls_last_chr_name != record.chrom)
			{
				++calls_chr_index;
				calls_last_chr_name = record.chrom;
				chr_data = &data.getChromosomeByName(record.chrom);
			}

			char alt = record.alt[0];
			if (alt == '<')
				continue;

			int64_t calls_key = calls_chr_index;
			calls_key <<= 32;
			calls_key |= record.pos;
			calls_key <<= 8;
			calls_key |= alt;

			while (filter_last_key < calls_key)
			{
				if (is_filter.eof())
				{
					std::cerr << "ERROR: unexpected end of filter file." << std::endl;
					return 1;
				}
				is_filter.read((char*)&filter_last_key, sizeof(filter_last_key));
				is_filter.read((char*)&filter_last_freq, sizeof(filter_last_freq));
				
				// add a data point for the reference allele
				int64_t filter_chr_pos = filter_last_key >> 8;
				if (filter_chr_pos != filter_last_chr_pos)
				{
					if (filter_use_ref)
					{
						int32_t filter_chr_index = filter_last_key >> 40;
						int32_t filter_pos = filter_chr_pos & 0xffffffff;
						ChromosomeData * filter_chr_data = &data.chromosomes[filter_chr_index];
						int pos = (filter_pos - filter_chr_data->offset) / filter_chr_data->scale;
						if (pos < 0 || pos >= filter_chr_data->baf_data.size())
							++out_of_bounds_count;
						else
							filter_chr_data->baf_data[pos] += 1.0f - filter_freq_accumulator;
						++ref_count;
					}
					filter_last_chr_pos = filter_chr_pos;
					filter_freq_accumulator = 0.0f;
					filter_use_ref = true;
				}

				filter_freq_accumulator += filter_last_freq;
			}

			if (++progress_notify > 1000)
			{
				progress_notify = 1;
				int32_t filter_pos = (filter_last_key >> 8) & 0xffffffff;
				char filter_alt = filter_last_key & 0xff;
				int32_t filter_chr_index = filter_last_key >> 40;
				std::cout << filter_chr_index << " " << calls_chr_index << std::endl;
				std::cout << filter_pos << " " << record.pos << std::endl;
				std::cout << filter_alt << " " << alt << std::endl;
				std::cout << record.ref << std::endl;
				std::cout << match_count << std::endl;
				std::cout << ref_count << std::endl;
			}

			if (filter_last_key == calls_key)
			{
				int pos = (record.pos - chr_data->offset) / chr_data->scale;
				if (pos < 0 || pos >= chr_data->baf_data.size())
					++out_of_bounds_count;
				else
					chr_data->baf_data[pos] += filter_last_freq;
				++match_count;
				filter_use_ref = false;
			}
			else if (filter_last_key == std::numeric_limits<int64_t>::max())
			{
				std::cout << "end of filter reached." << std::endl;
				break;
			}
		}
	}
	if (out_of_bounds_count > 0)
		std::cout << "WARNING: " << out_of_bounds_count << " base allele frequency data points were out of bounds. TODO: expand data region." << std::endl;

	return 0;
}
