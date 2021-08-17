#pragma once

#include <vector>

class BafData
{
public:
	BafData();
	BafData(float baf);

	float total_density;
	float top_density;
	float mid_density;
	float bot_density;
	float mid_mean;

	BafData combine(const BafData& rhs) const;
	
	BafData& operator+=(const BafData& rhs);
};
