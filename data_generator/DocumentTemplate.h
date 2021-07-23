#pragma once

#include <iostream>
#include <vector>
#include <map>

class DocumentTemplate
{
public:
	DocumentTemplate(std::istream& is);

	int parse(std::ostream& os);
	std::string resolve(std::string token) const;

	std::vector<std::string> lines;

	// maybe just use Coal?
	std::map<std::string, std::string> data;

	struct Scope
	{
		int variable;
		int line;
		int index;
		std::string iterable;
	};

	std::vector<std::pair<std::string, std::string>> variables;
	std::vector<Scope> scopes;
};
