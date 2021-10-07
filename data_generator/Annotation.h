#pragma once

#include <string>

class AnnotationData;

class Annotation
{
public:
	Annotation(const std::string& line, AnnotationData& data);

	uint32_t start, end;
};
