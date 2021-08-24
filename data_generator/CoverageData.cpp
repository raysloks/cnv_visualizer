#include "CoverageData.h"

#include <algorithm>

CoverageData::CoverageData()
{
	density_negative = 0.0f;
	mean_log2_negative = 0.0f;

	density_positive = 0.0f;
	mean_log2_positive = 0.0f;

	density = 0.0f;
	mean_log2 = 0.0f;
	min_log2 = 0.0f;
	max_log2 = 0.0f;
}

CoverageData::CoverageData(float coverage_log2)
{
	density_negative = 0.0f;
	mean_log2_negative = 0.0f;

	density_positive = 0.0f;
	mean_log2_positive = 0.0f;

	density = 1.0f;
	mean_log2 = coverage_log2;
	min_log2 = coverage_log2;
	max_log2 = coverage_log2;

	if (coverage_log2 < 0.0f)
	{
		density_negative = 1.0f;
		mean_log2_negative = coverage_log2;
	}
	else if (coverage_log2 > 0.0f)
	{
		density_positive = 1.0f;
		mean_log2_positive = coverage_log2;
	}
}

CoverageData CoverageData::combine(const CoverageData& rhs) const
{
	CoverageData result;

	result.density_negative = density_negative + rhs.density_negative;
	if (result.density_negative > 0.0f)
	{
		result.mean_log2_negative = (mean_log2_negative * density_negative + rhs.mean_log2_negative * rhs.density_negative) / result.density_negative;
		result.density_negative *= 0.5f;
	}

	result.density_positive = density_positive + rhs.density_positive;
	if (result.density_positive > 0.0f)
	{
		result.mean_log2_positive = (mean_log2_positive * density_positive + rhs.mean_log2_positive * rhs.density_positive) / result.density_positive;
		result.density_positive *= 0.5f;
	}

	result.density = density + rhs.density;
	if (result.density > 0.0f)
	{
		result.mean_log2 = (mean_log2 * density + rhs.mean_log2 * rhs.density) / result.density;
		result.density *= 0.5f;
	}

	result.min_log2 = std::min(min_log2, rhs.min_log2);
	result.max_log2 = std::max(max_log2, rhs.max_log2);

	return result;
}
