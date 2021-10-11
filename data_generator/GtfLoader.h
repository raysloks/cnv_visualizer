#pragma once

#include <iostream>

#include "AnnotationData.h"

class GtfLoader
{
public:
	int load(AnnotationData& data, std::istream& is);
};
