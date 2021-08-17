#include "BaseData.h"

BaseData::BaseData()
{
	total_density = 0.0f;
	a_density = 0.0f;
	c_density = 0.0f;
	g_density = 0.0f;
	t_density = 0.0f;
}

BaseData::BaseData(char base)
{
	total_density = 1.0f;
	a_density = 0.0f;
	c_density = 0.0f;
	g_density = 0.0f;
	t_density = 0.0f;
	switch (base)
	{
	case 'A':
		a_density = 1.0f;
		break;
	case 'C':
		c_density = 1.0f;
		break;
	case 'G':
		g_density = 1.0f;
		break;
	case 'T':
		t_density = 1.0f;
		break;
	default:
		total_density = 0.0f;
		break;
	}
}

BaseData BaseData::combine(const BaseData& rhs) const
{
	BaseData result;

	result.total_density = (total_density + rhs.total_density) / 2.0f;
	result.a_density = (a_density + rhs.a_density) / 2.0f;
	result.c_density = (c_density + rhs.c_density) / 2.0f;
	result.g_density = (g_density + rhs.g_density) / 2.0f;
	result.t_density = (t_density + rhs.t_density) / 2.0f;

	return result;
}

BaseData& BaseData::operator+=(const BaseData& rhs)
{
	total_density += rhs.total_density;
	a_density += rhs.a_density;
	c_density += rhs.c_density;
	g_density += rhs.g_density;
	t_density += rhs.t_density;
	return *this;
}
