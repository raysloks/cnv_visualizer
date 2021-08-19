#pragma once

class CoverageData
{
public:
	CoverageData();
	CoverageData(float coverage_log2);

	float density;

	float mean;
	float mean_log2;
	float min_log2;
	float max_log2;

	CoverageData combine(const CoverageData& rhs) const;
};
