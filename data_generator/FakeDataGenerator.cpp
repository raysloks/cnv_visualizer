#include "FakeDataGenerator.h"

ChromosomeData FakeDataGenerator::generateFakeChromosomeData(const ChromosomeTemplate& chr_template, int scale)
{
    ChromosomeData chr_data;

    chr_data.chr_template = chr_template;

    std::normal_distribution<float> edge_distribution(10000.0f, 50000.0f);
    std::uniform_real_distribution<float> uniform_distribution(0.0f, 1.0f);
    std::uniform_real_distribution<float> down_up_distribution(-1.0f, 1.0f);
    std::normal_distribution<float> normal_distribution(0.0f, 1.0f);

    int start_offset = std::abs(edge_distribution(rng));
    int end_offset = std::abs(edge_distribution(rng));

    int size = chr_template.size - start_offset - end_offset;

    size /= scale;

    chr_data.scale = scale;

    //chr_data.baf_offset = start_offset;
    //chr_data.log2_coverage_offset = start_offset;

    chr_data.baf_data.resize(size);
    chr_data.log2_coverage_data.resize(size);

    float allein_allein = 0.5f;
    for (int i = 0; i < size; ++i)
    {
        allein_allein += normal_distribution(rng) * 0.05f;
        allein_allein = std::min(std::max(allein_allein, 0.0f), 1.0f);
        if (allein_allein > std::uniform_real_distribution<float>(0.25f, 1.25f)(rng))
        {
            float decider = uniform_distribution(rng);
            if (decider < 0.4)
            {
                chr_data.baf_data[i] = 0.0f;
            }
            else if (decider < 0.8)
            {
                chr_data.baf_data[i] = std::min(std::max(normal_distribution(rng) * 0.1f + 0.5f, 0.0f), 1.0f);
            }
            else
            {
                chr_data.baf_data[i] = 1.0f;
            }
        }
    }

    for (int i = 0; i < size; ++i)
    {
        chr_data.log2_coverage_data[i] = 
            sinf(i / 300.0f) * 0.45f + 
            down_up_distribution(rng) * 0.45f + 
            normal_distribution(rng) * 0.1f;
    }

    return chr_data;
}
