#include "GtfLoader.h"

#include "Utility.h"

#include "AnnotationDataDefinitions.h"

int GtfLoader::load(AnnotationData& data, std::istream& is)
{
	size_t notify_counter = 0;
	std::vector<std::vector<std::string>> lines;
	std::string line;
	while (std::getline(is, line))
	{
		if (line[0] == '#')
		{
			
		}
		else
		{
			if (++notify_counter % 100000 == 0)
				std::cout << notify_counter << std::endl;
			auto fields = split(line, '\t');
			Annotation::prepare(fields, data);
			lines.push_back(fields);
		}
	}
	std::cout << "read " << lines.size() << " annotations." << std::endl;

	for (auto& set : set_map)
	{
		int64_t i = 0;
		for (auto& kv : data.*set.second.lookup)
			kv.second = i++;
	}
	std::cout << "updated indices." << std::endl;

	notify_counter = 0;
	for (auto& fields : lines)
	{
		if (++notify_counter % 100000 == 0)
			std::cout << 100.0f * notify_counter / lines.size() << "%" << std::endl;
		Annotation annotation(fields, data);
		auto chr = fields[0];
		data.annotations[chr].push_back(annotation);
	}

	return 0;
}
