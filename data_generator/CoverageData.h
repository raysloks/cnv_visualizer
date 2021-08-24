#pragma once

class CoverageData
{
public:
	CoverageData();
	CoverageData(float coverage_log2);

	float density_negative;
	float mean_log2_negative;

	float density_positive;
	float mean_log2_positive;

	float density;
	float mean_log2;
	float min_log2;
	float max_log2;

	CoverageData combine(const CoverageData& rhs) const;
};
