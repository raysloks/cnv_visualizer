#pragma once

#include <vector>

class BafData
{
public:
	BafData();
	BafData(float baf);

	float density;
	float mean;
	float mid_density;
	float mid_mean;
	float bot_density;
	float bot_mean;
	float top_density;
	float top_mean;
	float dev_mean;
	float undersampled;

	BafData combine(const BafData& rhs) const;
	
	BafData& operator+=(const BafData& rhs);
};
