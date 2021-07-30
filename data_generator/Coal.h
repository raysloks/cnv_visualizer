#pragma once

#include <string>
#include <map>
#include <vector>

#include "Vec2.h"
#include "Vec3.h"

class Coal
{
public:
	Coal();
	Coal(std::nullptr_t);
	Coal(const char * string);
	Coal(const std::string& string);
	Coal(int64_t integer);
	Coal(uint64_t integer);
	Coal(int32_t integer);
	Coal(uint32_t integer);
	Coal(float real);
	Coal(bool boolean);
	Coal(const std::map<std::string, Coal>& members);
	Coal(const std::vector<Coal>& elements);
	Coal(const Vec2& vector);
	Coal(const Vec3& vector);

	static Coal parse(std::istream& is);

	static std::string escape(const std::string& string);
	static void printCoal(std::ostream& os, const Coal& coal, const std::string& indent);
	static void printJson(std::ostream& os, const Coal& coal, const std::string& indent);

	void print(std::ostream& os) const;
	void printJson(std::ostream& os) const;

	enum class Type
	{
		Null,
		String,
		Integer,
		Real,
		Boolean,
		Object,
		Array,
		Nonexistant
	};

	Type type;
	std::string string;
	int64_t integer;
	float real;
	bool boolean;
	std::map<std::string, Coal> members;
	std::vector<Coal> elements;

	operator std::string() const;

	operator int64_t() const;
	operator uint64_t() const;

	operator float() const;

	operator bool() const;

	operator Vec2() const;
	operator Vec3() const;

	bool operator==(const Coal& rhs) const;
	bool operator!=(const Coal& rhs) const;

	Coal& operator[](const char * key);
	Coal& operator[](const std::string& key);
	Coal& operator[](std::string&& key);
	Coal& operator[](size_t index);
	Coal& operator[](int index);
	Coal operator[](const char * key) const;
	Coal operator[](const std::string& key) const;
	Coal operator[](std::string&& key) const;
	Coal operator[](size_t index) const;
	Coal operator[](int index) const;
};

