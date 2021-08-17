#pragma once

#include <random>

#include "ChromosomeData.h"
#include "ChromosomeTemplate.h"

class FakeDataGenerator
{
public:
	ChromosomeData generateFakeChromosomeData(const ChromosomeTemplate& chr_template, int scale);

private:
	std::default_random_engine rng;
};
