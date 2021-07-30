#include "Coal.h"

#include <iostream>
#include <sstream>

Coal parseString(std::istream& is)
{
	std::string string;
	int c;
	do
	{
		c = is.get();
		if (c == '\\')
		{
			c = is.get();
			string += c;
			continue;
		}
		if (c == '"')
		{
			/*do
			{
				c = is.get();
				if (isspace(c))
					continue;
				if (c == '"')
					break;
				is.unget();
				return string;
			} while (!is.eof());
			continue;*/
			break;
		}
		string += c;
	} while (!is.eof());
	return string;
}

Coal parseNumber(std::istream& is)
{
	std::string string;
	bool radix = false;
	int c;
	do
	{
		c = is.get();
		if (isdigit(c) || c == '-' && string.empty())
		{
			string += c;
			continue;
		}
		if (radix == false)
		{
			if (c == '.')
			{
				string += c;
				radix = true;
				continue;
			}
		}
		if (c == 'f')
		{
			radix = true;
			break;
		}
		is.unget();
		break;
	} while (!is.eof());

	std::stringstream ss(string);
	if (radix)
	{
		float real;
		ss >> real;
		return real;
	}
	else
	{
		int64_t integer;
		ss >> integer;
		return integer;
	}
}

std::string parseName(std::istream& is)
{
	int c;
	do
	{
		c = is.get();
		if (isspace(c))
			continue;
		if (isalpha(c) || c == '_')
		{
			std::string name;
			name += c;
			do
			{
				c = is.get();
				if (isalnum(c) || c == '_')
					name += c;
				else
					break;
			} while (!is.eof());
			return name;
		}
		break;
	} while (!is.eof());
	return std::string();
}

Coal parseMembers(std::istream& is)
{
	Coal coal;
	coal.type = Coal::Type::Object;
	do
	{
		auto name = parseName(is);
		if (name.empty())
			break;
		auto member = Coal::parse(is);
		if (member.type != Coal::Type::Nonexistant)
		{
			coal.members.emplace(std::make_pair(name, member));
		}
		else
		{
			throw std::runtime_error("Unable to parse member '" + name + "'.");
		}
	} while (!is.eof());
	return coal;
}

Coal parseElements(std::istream& is)
{
	Coal coal;
	coal.type = Coal::Type::Array;
	do
	{
		auto element = Coal::parse(is);
		if (element.type != Coal::Type::Nonexistant)
		{
			coal.elements.push_back(element);
		}
		else
		{
			break;
		}
	} while (!is.eof());
	return coal;
}

Coal::Coal()
{
	type = Type::Nonexistant;
}

Coal::Coal(std::nullptr_t)
{
	integer = 0;
	real = 0.0f;
	boolean = false;
	type = Type::Null;
}

Coal::Coal(const char * string) : string(string)
{
	integer = 0;
	real = 0.0f;
	boolean = false;
	type = Type::String;
}

Coal::Coal(const std::string& string) : string(string)
{
	integer = 0;
	real = 0.0f;
	boolean = false;
	type = Type::String;
}

Coal::Coal(int64_t integer) : integer(integer)
{
	real = integer;
	boolean = integer;
	type = Type::Integer;
}

Coal::Coal(uint64_t integer) : integer(integer)
{
	real = integer;
	boolean = integer;
	type = Type::Integer;
}

Coal::Coal(int32_t integer) : integer(integer)
{
	real = integer;
	boolean = integer;
	type = Type::Integer;
}

Coal::Coal(uint32_t integer) : integer(integer)
{
	real = integer;
	boolean = integer;
	type = Type::Integer;
}

Coal::Coal(float real) : real(real)
{
	integer = real;
	boolean = real;
	type = Type::Real;
}

Coal::Coal(bool boolean) : boolean(boolean)
{
	integer = boolean;
	real = boolean;
	type = Type::Boolean;
}

Coal::Coal(const std::map<std::string, Coal>& members) : members(members)
{
	integer = 0;
	real = 0.0f;
	boolean = false;
	type = Type::Object;
}

Coal::Coal(const std::vector<Coal>& elements) : elements(elements)
{
	integer = 0;
	real = 0.0f;
	boolean = false;
	type = Type::Array;
}

Coal::Coal(const Vec2& vector)
{
	elements = { vector.x, vector.y };
	type = Type::Array;
}

Coal::Coal(const Vec3& vector)
{
	elements = { vector.x, vector.y, vector.z };
	type = Type::Array;
}

Coal Coal::parse(std::istream& is)
{
	int c;
	do
	{
		c = is.get();
		if (isspace(c))
		{
			continue;
		}
		if (c == '"')
		{
			return parseString(is);
		}
		if (isalpha(c))
		{
			std::string string;
			string += c;
			do
			{
				c = is.get();
				if (isalpha(c))
					string += c;
				else
					break;
			} while (!is.eof());
			if (string == "true")
			{
				return true;
			}
			if (string == "false")
			{
				return false;
			}
			if (string == "null")
			{
				return nullptr;
			}
			throw std::runtime_error("Unrecognized symbol '" + string + "'.");
		}
		if (isdigit(c) || c == '-')
		{
			is.unget();
			return parseNumber(is);
		}
		if (c == '{')
		{
			return parseMembers(is);
		}
		if (c == '[')
		{
			return parseElements(is);
		}
		break;
	} while (!is.eof());
	return Coal();
}

std::string Coal::escape(const std::string& string)
{
	std::string escaped;
	for (auto c : string)
	{
		switch (c)
		{
		case '"':
			escaped += '\\';
			break;
		case '\\':
			escaped += '\\';
			break;
		default:
			break;
		}
		escaped += c;
	}
	return escaped;
}

void Coal::printCoal(std::ostream& os, const Coal& coal, const std::string& indent)
{
	switch (coal.type)
	{
	case Coal::Type::Null:
		os << "null";
		break;
	case Coal::Type::String:
		os << "\"" << escape(coal.string) << "\"";
		break;
	case Coal::Type::Integer:
		os << coal.integer;
		break;
	case Coal::Type::Real:
		os << coal.real << "f";
		break;
	case Coal::Type::Boolean:
		os << (coal.boolean ? "true" : "false");
		break;
	case Coal::Type::Object:
		os << "{\n";
		for (auto&& member : coal.members)
		{
			os << indent + " " << member.first << " ";
			printCoal(os, member.second, indent + " ");
		}
		os << indent << "}";
		break;
	case Coal::Type::Array:
		os << "[\n";
		for (auto&& element : coal.elements)
		{
			os << indent + " ";
			printCoal(os, element, indent + " ");
			/*if (&element != &coal.elements.back())
				os << indent << " ,\n";*/
		}
		os << indent << "]";
		break;
	default:
		break;
	}
	os << '\n';
}

void Coal::printJson(std::ostream& os, const Coal& coal, const std::string& indent)
{
	switch (coal.type)
	{
	case Coal::Type::Null:
		os << "null";
		break;
	case Coal::Type::String:
		os << "\"" << escape(coal.string) << "\"";
		break;
	case Coal::Type::Integer:
		os << coal.integer;
		break;
	case Coal::Type::Real:
		os << coal.real;
		break;
	case Coal::Type::Boolean:
		os << (coal.boolean ? "true" : "false");
		break;
	case Coal::Type::Object:
		{
			os << "{\n";
			bool first = true;
			for (auto&& member : coal.members)
			{
				os << indent + "\t";
				if (!first)
					os << ", ";
				first = false;
				os << "\"" << member.first << "\": ";
				printJson(os, member.second, indent + "\t");
			}
			os << indent << "}";
			break;
		}
	case Coal::Type::Array:
		{
			os << "[\n";
			bool first = true;
			for (auto&& element : coal.elements)
			{
				os << indent + "\t";
				if (!first)
					os << ", ";
				first = false;
				printJson(os, element, indent + "\t");
			}
			os << indent << "]";
		}
		break;
	default:
		os << "undefined";
		break;
	}
	os << '\n';
}

void Coal::print(std::ostream& os) const
{
	printCoal(os, *this, "");
}

void Coal::printJson(std::ostream& os) const
{
	printJson(os, *this, "");
}

Coal::operator std::string() const
{
	switch (type)
	{
	case Coal::Type::Null:
		return std::string();
	case Coal::Type::String:
		return string;
	case Coal::Type::Integer:
		return std::string();
	case Coal::Type::Real:
		return std::string();
	case Coal::Type::Boolean:
		return std::string();
	case Coal::Type::Object:
		return std::string();
	case Coal::Type::Array:
		return std::string();
	default:
		return std::string();
	}
}

Coal::operator int64_t() const
{
	switch (type)
	{
	case Coal::Type::Null:
		return 0;
	case Coal::Type::String:
		return 0;
	case Coal::Type::Integer:
		return integer;
	case Coal::Type::Real:
		return real;
	case Coal::Type::Boolean:
		return boolean;
	case Coal::Type::Object:
		return 0;
	case Coal::Type::Array:
		return 0;
	default:
		return 0;
	}
}

Coal::operator uint64_t() const
{
	switch (type)
	{
	case Coal::Type::Null:
		return 0;
	case Coal::Type::String:
		return 0;
	case Coal::Type::Integer:
		return integer;
	case Coal::Type::Real:
		return real;
	case Coal::Type::Boolean:
		return boolean;
	case Coal::Type::Object:
		return 0;
	case Coal::Type::Array:
		return 0;
	default:
		return 0;
	}
}

Coal::operator float() const
{
	switch (type)
	{
	case Coal::Type::Null:
		return 0.0f;
	case Coal::Type::String:
		return 0.0f;
	case Coal::Type::Integer:
		return integer;
	case Coal::Type::Real:
		return real;
	case Coal::Type::Boolean:
		return boolean;
	case Coal::Type::Object:
		return 0.0f;
	case Coal::Type::Array:
		return 0.0f;
	default:
		return 0.0f;
	}
}

Coal::operator bool() const
{
	switch (type)
	{
	case Coal::Type::Null:
		return false;
	case Coal::Type::String:
		return false;
	case Coal::Type::Integer:
		return integer;
	case Coal::Type::Real:
		return real;
	case Coal::Type::Boolean:
		return boolean;
	case Coal::Type::Object:
		return false;
	case Coal::Type::Array:
		return false;
	default:
		return false;
	}
}

Coal::operator Vec2() const
{
	switch (type)
	{
	case Coal::Type::Object:
		return Vec2((*this)["x"], (*this)["y"]);
	case Coal::Type::Array:
		return Vec2((*this)[0], (*this)[1]);
	default:
		return (float)*this;
	}
}

Coal::operator Vec3() const
{
	switch (type)
	{
	case Coal::Type::Object:
		return Vec3((*this)["x"], (*this)["y"], (*this)["z"]);
	case Coal::Type::Array:
		return Vec3((*this)[0], (*this)[1], (*this)[2]);
	default:
		return (float)*this;
	}
}

bool Coal::operator==(const Coal& rhs) const
{
	if (type != rhs.type)
		return false;
	switch (type)
	{
	case Coal::Type::Null:
		return true;
	case Coal::Type::String:
		return string == rhs.string;
	case Coal::Type::Integer:
		return integer == rhs.integer;
	case Coal::Type::Real:
		return real == rhs.real;
	case Coal::Type::Boolean:
		return boolean == rhs.boolean;
	case Coal::Type::Object:
		return members == rhs.members;
	case Coal::Type::Array:
		return elements == rhs.elements;
	default:
		return false;
	}
}

bool Coal::operator!=(const Coal& rhs) const
{
	if (type != rhs.type)
		return true;
	switch (type)
	{
	case Coal::Type::Null:
		return false;
	case Coal::Type::String:
		return string != rhs.string;
	case Coal::Type::Integer:
		return integer != rhs.integer;
	case Coal::Type::Real:
		return real != rhs.real;
	case Coal::Type::Boolean:
		return boolean != rhs.boolean;
	case Coal::Type::Object:
		return members != rhs.members;
	case Coal::Type::Array:
		return elements != rhs.elements;
	default:
		return false;
	}
}

Coal & Coal::operator[](const char * key)
{
	return members[key];
}

Coal& Coal::operator[](const std::string& key)
{
	return members[key];
}

Coal& Coal::operator[](std::string&& key)
{
	return members[key];
}

Coal& Coal::operator[](size_t index)
{
	return elements[index];
}

Coal & Coal::operator[](int index)
{
	return elements[index];
}

Coal Coal::operator[](const char * key) const
{
	auto it = members.find(key);
	if (it != members.end())
		return it->second;
	return Coal();
}

Coal Coal::operator[](const std::string& key) const
{
	auto it = members.find(key);
	if (it != members.end())
		return it->second;
	return Coal();
}

Coal Coal::operator[](std::string&& key) const
{
	auto it = members.find(key);
	if (it != members.end())
		return it->second;
	return Coal();
}

Coal Coal::operator[](size_t index) const
{
	if (index < elements.size())
		return elements[index];
	return Coal();
}

Coal Coal::operator[](int index) const
{
	if (index < elements.size())
		return elements[index];
	return Coal();
}
