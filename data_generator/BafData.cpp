#include "BafData.h"

const float epsilon_top = 0.05f;
const float epsilon_bot = 0.1f;

BafData::BafData()
{
    total_density = 0.0f;
    top_density = 0.0f;
    mid_density = 0.0f;
    bot_density = 0.0f;
    mid_mean = 0.0f;
}

BafData::BafData(float baf)
{
    total_density = 1.0f;
    top_density = 0.0f;
    mid_density = 0.0f;
    bot_density = 0.0f;
    mid_mean = 0.0f;
    if (baf > 1.0f - epsilon_top)
    {
        top_density = 1.0f;
    }
    else if (baf < epsilon_bot)
    {
        bot_density = 1.0f;
    }
    else
    {
        mid_density = 1.0f;
        mid_mean = baf;
    }
}

BafData BafData::combine(const BafData& rhs) const
{
    BafData result;

    result.total_density = (total_density + rhs.total_density) / 2.0f;
    result.top_density = (top_density + rhs.top_density) / 2.0f;
    result.mid_density = (mid_density + rhs.mid_density) / 2.0f;
    result.bot_density = (bot_density + rhs.bot_density) / 2.0f;
    if (mid_density + rhs.mid_density > 0.0f)
        result.mid_mean = (mid_mean * mid_density + rhs.mid_mean * rhs.mid_density) / (mid_density + rhs.mid_density);
    else
        result.mid_mean = 0.0f;

    return result;
}

BafData& BafData::operator+=(const BafData& rhs)
{
	mid_mean = mid_mean * mid_density + rhs.mid_mean * rhs.mid_density;
	mid_density += rhs.mid_density;
	if (mid_density > 0.0f)
		mid_mean /= mid_density;

	total_density += rhs.total_density;
	top_density += top_density;
	bot_density += bot_density;
	
	return *this;
}
