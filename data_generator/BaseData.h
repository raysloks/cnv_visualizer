#pragma once

#include <vector>

class BaseData
{
public:
	BaseData();
	BaseData(char base);

	float total_density;
	float a_density;
	float c_density;
	float g_density;
	float t_density;

	BaseData combine(const BaseData& rhs) const;
	
	BaseData& operator+=(const BaseData& rhs);
};
