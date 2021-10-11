#include "BafData.h"

#include <cmath>

const float cutoff_bot = 0.25f;
const float cutoff_top = 0.75f;
const float cutoff_mid_bot = 0.25f;
const float cutoff_mid_top = 0.75f;

BafData::BafData()
{
	density = 0.0f;
	mean = 0.0f;
	mid_density = 0.0f;
	mid_mean = 0.0f;
	bot_density = 0.0f;
	bot_mean = 0.0f;
	top_density = 0.0f;
	top_mean = 0.0f;
	dev_mean = 0.0f;
	undersampled = 0.0f;
}

BafData::BafData(float baf)
{
	density = 1.0f;
	mean = baf;
	dev_mean = fabsf(baf - 0.5f);
	
	mid_density = 0.0f;
	mid_mean = 0.0f;
	bot_density = 0.0f;
	bot_mean = 0.0f;
	top_density = 0.0f;
	top_mean = 0.0f;

	if (baf <= cutoff_mid_top && baf >= cutoff_mid_bot)
	{
		mid_density = 1.0f;
		mid_mean = baf;
	}

	if (baf <= cutoff_bot)
	{
		bot_density = 1.0f;
		bot_mean = baf;
	}

	if (baf >= cutoff_top)
	{
		top_density = 1.0f;
		top_mean = baf;
	}

	undersampled = baf;
}

BafData BafData::combine(const BafData& rhs) const
{
	BafData result;

	result.density = (density + rhs.density) / 2.0f;
	if (density + rhs.density > 0.0f)
	{
		result.mean = (mean * density + rhs.mean * rhs.density) / (density + rhs.density);
		result.dev_mean = (dev_mean * density + rhs.dev_mean * rhs.density) / (density + rhs.density);
	}

	result.mid_density = (mid_density + rhs.mid_density) / 2.0f;
	if (mid_density + rhs.mid_density > 0.0f)
		result.mid_mean = (mid_mean * mid_density + rhs.mid_mean * rhs.mid_density) / (mid_density + rhs.mid_density);

	result.bot_density = (bot_density + rhs.bot_density) / 2.0f;
	if (bot_density + rhs.bot_density > 0.0f)
		result.bot_mean = (bot_mean * bot_density + rhs.bot_mean * rhs.bot_density) / (bot_density + rhs.bot_density);

	result.top_density = (top_density + rhs.top_density) / 2.0f;
	if (top_density + rhs.top_density > 0.0f)
		result.top_mean = (top_mean * top_density + rhs.top_mean * rhs.top_density) / (top_density + rhs.top_density);
	
	result.undersampled = density != 0.0f ? undersampled : rhs.undersampled;

	return result;
}

BafData& BafData::operator+=(const BafData& rhs)
{
	undersampled = density != 0.0f ? undersampled : rhs.undersampled;

	mean = mean * density + rhs.mean * rhs.density;
	dev_mean = dev_mean * density + rhs.dev_mean * rhs.density;
	density += rhs.density;
	if (density > 0.0f)
	{
		mean /= density;
		dev_mean /= density;
	}

	mid_mean = mid_mean * mid_density + rhs.mid_mean * rhs.mid_density;
	mid_density += rhs.mid_density;
	if (mid_density > 0.0f)
		mid_mean /= mid_density;

	bot_mean = bot_mean * bot_density + rhs.bot_mean * rhs.bot_density;
	bot_density += rhs.bot_density;
	if (bot_density > 0.0f)
		bot_mean /= bot_density;

	top_mean = top_mean * top_density + rhs.top_mean * rhs.top_density;
	top_density += rhs.top_density;
	if (top_density > 0.0f)
		top_mean /= top_density;
	
	return *this;
}
