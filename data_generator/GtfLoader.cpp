#include "GtfLoader.h"

#include "Utility.h"

int GtfLoader::load(AnnotationData& data, std::istream& is)
{
	size_t annotation_count = 0;
	std::string line;
	while (std::getline(is, line))
	{
		if (line[0] == '#')
		{
			
		}
		else
		{
			auto fields = split(line, '\t');
			data.annotations[fields[0]].push_back(Annotation(line, data));
			++annotation_count;
		}
	}
	std::cout << "loaded " << annotation_count << " annotations." << std::endl;

	return 0;
}
