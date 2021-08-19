#include "CoverageData.h"

#include <algorithm>

CoverageData::CoverageData()
{
	density = 0.0f;
	mean_log2 = 0.0f;
	min_log2 = 0.0f;
	max_log2 = 0.0f;
	mean = 0.0f;
}

CoverageData::CoverageData(float coverage_log2)
{
	density = 1.0f;
	mean_log2 = coverage_log2;
	min_log2 = coverage_log2;
	max_log2 = coverage_log2;
	mean = exp2f(coverage_log2);
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
	result.mean_log2 = (mean_log2 * density + rhs.mean_log2 * rhs.density) / (density + rhs.density);
	result.min_log2 = std::min(min_log2, rhs.min_log2);
	result.max_log2 = std::max(max_log2, rhs.max_log2);
	result.mean = (mean * density + rhs.mean * rhs.density) / (density + rhs.density);

	return result;
}
