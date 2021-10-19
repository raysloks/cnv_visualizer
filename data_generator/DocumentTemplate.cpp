#include "DocumentTemplate.h"

#include <sstream>

DocumentTemplate::DocumentTemplate(std::istream& is)
{
	std::string line;
	while (std::getline(is, line))
		lines.push_back(line);
}

int DocumentTemplate::parse(std::ostream& os)
{
	variables.clear();
	scopes.clear();

	int skipping = 0;

	for (int i = 0; i < lines.size(); ++i)
	{
		auto&& line = lines[i];
		size_t first_pos = line.find("%%");
		if (first_pos == std::string::npos)
			os << line << '\n';
		else
		{
			size_t last_pos = line.rfind("%%");
			std::string cmd;
			if (last_pos == first_pos)
				cmd = line.substr(first_pos + 2);
			else
				cmd = line.substr(first_pos + 2, last_pos - first_pos - 2);
			std::stringstream ss(cmd);

			std::vector<std::string> tokens;
			while (!ss.eof())
			{
				std::string token;
				ss >> token;
				if (token.empty())
					break;
				tokens.push_back(token);
				//std::cout << token << " ";
			}
			//std::cout << std::endl;

			if (tokens.size() > 0)
			{
				if (tokens[0] == "for")
				{
					if (tokens.size() == 4)
					{
						if (tokens[2] != "in")
						{
							std::cerr << "ERROR: template syntax error, missing 'in' in 'for'-expression." << std::endl;
							return 1;
						}
						if (skipping)
						{
							skipping += 1;
							continue;
						}
						if (scopes.size() > 0)
						{
							Scope& scope = scopes.back();
							if (scope.line == i)
							{
								++scope.index;
								//std::cout << scope.index << std::endl;
								variables[scope.variable + 1].second = std::to_string(scope.index);
								std::string& iterator_value = variables[scope.variable].second;
								iterator_value = scope.iterable + "[" + std::to_string(scope.index) + "]";
								auto it = data.lower_bound(iterator_value);
								//std::cout << iterator_value << std::endl;
								//std::cout << it->first << std::endl;
								if (it != data.end())
									if (it->first.rfind(iterator_value, 0) == 0)
										continue;
								skipping = 1;
								continue;
							}
						}
						Scope scope;
						scope.variable = variables.size();
						scope.line = i;
						scope.index = -1;
						scope.iterable = resolve(tokens[3]);
						scopes.push_back(scope);
						variables.push_back(std::make_pair(tokens[1], std::string()));
						variables.push_back(std::make_pair("@" + tokens[1], std::string()));
						--i;
						continue;
					}
					else
					{
						std::cerr << "ERROR: template syntax error, incorrect number of tokens in 'for'-expression." << std::endl;
						return 1;
					}
				}
				else if (tokens[0] == "end")
				{
					if (scopes.size() > 0)
					{
						Scope& scope = scopes.back();
						if (skipping == 1)
						{
							variables.resize(scope.variable);
							scopes.pop_back();
							skipping -= 1;
						}
						else if (skipping == 0)
						{
							i = scope.line - 1;
						}
						else
						{
							skipping -= 1;
						}
						continue;
					}
					else
					{
						std::cerr << "ERROR: template syntax error, unexpected 'end'." << std::endl;
						return 1;
					}
				}
				else
				{
					if (skipping)
						continue;

					os << line.substr(0, first_pos);

					for (auto token : tokens)
					{
						token = resolve(token);
						auto it = data.find(token);
						if (it != data.end())
							os << it->second;
						else
						{
							os << token;
							std::cerr << "WARNING: template data warning, failed to resolve '" << token << "'." << std::endl;
						}
					}

					if (last_pos != first_pos)
						os << line.substr(last_pos + 2);
					os << "\n";
				}
			}
		}
	}
	return 0;
}

std::string DocumentTemplate::resolve(std::string token) const
{
	//std::cout << "matching '" << token << "'..." << std::endl;
	for (auto i = variables.rbegin(); i != variables.rend(); ++i)
	{
		size_t pos = token.find(i->first);
		if (pos != std::string::npos)
		{
			if (token.size() > pos + i->first.size())
			{
				char following_char = token[pos + i->first.size()];
				if (isalnum(following_char))
					continue;
			}
			token.replace(pos, i->first.size(), i->second);
			break;
		}
	}
	//std::cout << "matched to '" << token << "'." << std::endl;

	return token;
}
