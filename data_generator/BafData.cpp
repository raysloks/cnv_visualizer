#include "BafData.h"

const float epsilon_top = 0.05f;
const float epsilon_bot = 0.1f;

BafData::BafData()
{
	total_density = 0.0f;
	top_density = 0.0f;
	bot_density = 0.0f;
	mean = 0.0f;
}

BafData::BafData(float baf)
{
	total_density = 1.0f;
	top_density = 0.0f;
	bot_density = 0.0f;
	mean = baf;
	if (baf > 1.0f - epsilon_top)
		top_density = 1.0f;
	else if (baf < epsilon_bot)
		bot_density = 1.0f;
}

BafData BafData::combine(const BafData& rhs) const
{
	BafData result;

	result.total_density = (total_density + rhs.total_density) / 2.0f;
	result.top_density = (top_density + rhs.top_density) / 2.0f;
	result.bot_density = (bot_density + rhs.bot_density) / 2.0f;
	if (total_density + rhs.total_density > 0.0f)
		result.mean = (mean * total_density + rhs.mean * rhs.total_density) / (total_density + rhs.total_density);
	else
		result.mean = 0.0f;

	return result;
}

BafData& BafData::operator+=(const BafData& rhs)
{
	mean = mean * total_density + rhs.mean * rhs.total_density;
	total_density += rhs.total_density;
	if (total_density > 0.0f)
		mean /= total_density;

	top_density += rhs.top_density;
	bot_density += rhs.bot_density;
	
	return *this;
}
