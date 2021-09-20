#include "BafData.h"

const float epsilon_top = 0.05f;
const float epsilon_bot = 0.1f;

BafData::BafData()
{
	density = 0.0f;
	mean = 0.0f;
	min = 1.0f;
	max = 0.0f;
	under = 0.0f;
}

BafData::BafData(float baf)
{
	density = 1.0f;
	mean = baf;
	min = baf;
	max = baf;
	under = baf;
}

BafData BafData::combine(const BafData& rhs) const
{
	BafData result;

	result.density = (density + rhs.density) / 2.0f;
	if (density + rhs.density > 0.0f)
		result.mean = (mean * density + rhs.mean * rhs.density) / (density + rhs.density);
	else
		result.mean = 0.0f;
	result.min = fminf(min, rhs.min);
	result.max = fmaxf(max, rhs.max);
	result.under = density != 0.0f ? under : rhs.under;

	return result;
}

BafData& BafData::operator+=(const BafData& rhs)
{
	result.under = density != 0.0f ? under : rhs.under;

	mean = mean * density + rhs.mean * rhs.density;
	density += rhs.density;
	if (density > 0.0f)
		mean /= density;
	result.min = fminf(min, rhs.min);
	result.max = fmaxf(max, rhs.max);
	
	return *this;
}
