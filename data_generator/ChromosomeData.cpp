#include "ChromosomeData.h"

#include <iostream>
#include <fstream>

ChromosomeData ChromosomeData::shrink() const
{
	ChromosomeData chr_data;

	chr_data.chr_template = chr_template;

	chr_data.scale = scale * 2;

	chr_data.offset = offset;

	chr_data.baf_data.resize(baf_data.size() / 2);
	chr_data.log2_coverage_data.resize(log2_coverage_data.size() / 2);
	chr_data.base_data.resize(base_data.size() / 2);

	for (size_t i = 0; i < chr_data.baf_data.size(); ++i)
		chr_data.baf_data[i] = baf_data[i * 2].combine(baf_data[i * 2 + 1]);

	for (size_t i = 0; i < chr_data.log2_coverage_data.size(); ++i)
		chr_data.log2_coverage_data[i] = log2_coverage_data[i * 2].combine(log2_coverage_data[i * 2 + 1]);

	for (size_t i = 0; i < chr_data.base_data.size(); ++i)
		chr_data.base_data[i] = base_data[i * 2].combine(base_data[i * 2 + 1]);

	return chr_data;
}

int chunk_size = 2048;

template <class T>
std::vector<float> extract(const std::vector<T>& input, size_t offset, float T::*member)
{
	std::vector<float> output(chunk_size);

	for (size_t i = offset; i < input.size() && i < offset + chunk_size; ++i)
	{
		output[i - offset] = input[i].*member;
	}

	return output;
}

template <class T>
void dump_it(const std::vector<T>& input, std::ostream& os)
{
	os.write((char*)input.data(), sizeof(T) * input.size());
}

int ChromosomeData::save(const std::string& path, int max_scale) const
{
	// std::cout << "exporting chromosome " << chr_template.name << "..." << std::endl;

	ChromosomeData shrunk_chr_data = *this; // unnecessary copy, but it's kinda awkward to avoid it
	int expected_scale = shrunk_chr_data.scale;
	
	while (expected_scale < max_scale)
	{
		if (expected_scale >= 100)
		{
			size_t max_len = std::max(shrunk_chr_data.baf_data.size(), std::max(shrunk_chr_data.log2_coverage_data.size(), shrunk_chr_data.base_data.size()));
			for (int i = 0; i < max_len; i += chunk_size)
			{
				std::string fp = path + chr_template.name + "_" + std::to_string(shrunk_chr_data.scale) + "_" + std::to_string(i / chunk_size) + ".cvd";
				// std::cout << fp << std::endl;
				std::ofstream f(fp, std::ofstream::binary | std::ofstream::trunc);

				dump_it(extract(shrunk_chr_data.log2_coverage_data, i, &CoverageData::density_negative), f);
				dump_it(extract(shrunk_chr_data.log2_coverage_data, i, &CoverageData::mean_log2_negative), f);
				dump_it(extract(shrunk_chr_data.log2_coverage_data, i, &CoverageData::density_positive), f);
				dump_it(extract(shrunk_chr_data.log2_coverage_data, i, &CoverageData::mean_log2_positive), f);
				dump_it(extract(shrunk_chr_data.log2_coverage_data, i, &CoverageData::density), f);
				dump_it(extract(shrunk_chr_data.log2_coverage_data, i, &CoverageData::mean_log2), f);
				dump_it(extract(shrunk_chr_data.log2_coverage_data, i, &CoverageData::min_log2), f);
				dump_it(extract(shrunk_chr_data.log2_coverage_data, i, &CoverageData::max_log2), f);

				dump_it(extract(shrunk_chr_data.baf_data, i, &BafData::density), f);
				dump_it(extract(shrunk_chr_data.baf_data, i, &BafData::mean), f);
				dump_it(extract(shrunk_chr_data.baf_data, i, &BafData::mid_density), f);
				dump_it(extract(shrunk_chr_data.baf_data, i, &BafData::mid_mean), f);
				dump_it(extract(shrunk_chr_data.baf_data, i, &BafData::bot_density), f);
				dump_it(extract(shrunk_chr_data.baf_data, i, &BafData::bot_mean), f);
				dump_it(extract(shrunk_chr_data.baf_data, i, &BafData::top_density), f);
				dump_it(extract(shrunk_chr_data.baf_data, i, &BafData::top_mean), f);
				dump_it(extract(shrunk_chr_data.baf_data, i, &BafData::dev_mean), f);
				dump_it(extract(shrunk_chr_data.baf_data, i, &BafData::undersampled), f);

				dump_it(extract(shrunk_chr_data.base_data, i, &BaseData::total_density), f);
				dump_it(extract(shrunk_chr_data.base_data, i, &BaseData::a_density), f);
				dump_it(extract(shrunk_chr_data.base_data, i, &BaseData::c_density), f);
				dump_it(extract(shrunk_chr_data.base_data, i, &BaseData::g_density), f);
				dump_it(extract(shrunk_chr_data.base_data, i, &BaseData::t_density), f);

				/* f.write((char*)&shrunk_chr_data.scale, sizeof(shrunk_chr_data.scale));

				int baf_offset = shrunk_chr_data.baf_offset + i * shrunk_chr_data.scale;
				f.write((char*)&baf_offset, sizeof(baf_offset));
				int baf_size = std::min(chunk_size, std::max(0, (int)shrunk_chr_data.baf_data.size() - i));
				f.write((char*)&baf_size, sizeof(baf_size));
				f.write((char*)(shrunk_chr_data.baf_data.data() + i), sizeof(BafData) * baf_size);

				int log2_offset = shrunk_chr_data.log2_coverage_offset + i * shrunk_chr_data.scale;
				f.write((char*)&log2_offset, sizeof(log2_offset));
				int log2_size = std::min(chunk_size, std::max(0, (int)shrunk_chr_data.log2_coverage_data.size() - i));
				f.write((char*)&log2_size, sizeof(log2_size));
				f.write((char*)(shrunk_chr_data.log2_coverage_data.data() + i), sizeof(CoverageData) * log2_size); */
			}
		}

		expected_scale *= 2;
		if (expected_scale < max_scale)
		{
			shrunk_chr_data = shrunk_chr_data.shrink();
		}
	}

	//std::cout << "chromosome " << chr_template.name << " exported." << std::endl;

	return 0;
}
