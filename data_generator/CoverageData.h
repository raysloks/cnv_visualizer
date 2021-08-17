#pragma once

class CoverageData
{
public:
	CoverageData();
	CoverageData(float coverage);

	float density;

	float mean;
	float min;
	float max;

	CoverageData combine(const CoverageData& rhs) const;
};
