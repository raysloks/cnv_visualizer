#include "CoverageData.h"

#include <algorithm>

CoverageData::CoverageData()
{
	density = 0.0f;
	mean = 0.0f;
	min = 0.0f;
	max = 0.0f;
}

CoverageData::CoverageData(float coverage)
{
	density = 1.0f;
	mean = coverage;
	min = coverage;
	max = coverage;
}

CoverageData CoverageData::combine(const CoverageData& rhs) const
{
	CoverageData result;

	if (density == 0.0f)
	{
		result = rhs;
		result.density *= 0.5f;
		return result;
	}
	else if (rhs.density == 0.0f)
	{
		result = *this;
		result.density *= 0.5f;
		return result;
	}

	result.density = (density + rhs.density) * 0.5f;
	result.mean = (mean * density + rhs.mean * rhs.density) / (density + rhs.density);
	result.min = std::min(min, rhs.min);
	result.max = std::max(max, rhs.max);

	return result;
}
