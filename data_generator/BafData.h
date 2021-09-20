#pragma once

#include <vector>

class BafData
{
public:
	BafData();
	BafData(float baf);

	float density;
	float mean;
	float min;
	float max;
	float under;

	BafData combine(const BafData& rhs) const;
	
	BafData& operator+=(const BafData& rhs);
};
