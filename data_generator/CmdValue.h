#pragma once

#include <string>
#include <vector>

class CmdValue
{
public:
	CmdValue(int * integer);
	CmdValue(float * real);
	CmdValue(bool * flag);
	CmdValue(std::string * string, bool is_file);
	CmdValue(std::vector<int> * integers);
	CmdValue(std::vector<float> * reals);
	CmdValue(std::vector<std::string> * strings, bool is_file);

	bool needsValue() const;
	int parseValue(const std::string& arg);

	const char * getTypeName() const;

	enum class Type
	{
		Integer,
		Real,
		Flag,
		File,
		String,
		IntegerMulti,
		RealMulti,
		FileMulti,
		StringMulti,
	};

	Type type;
	union
	{
		int * p_int;
		float * p_float;
		bool * p_bool;
		std::string * p_string;
		std::vector<int> * p_v_int;
		std::vector<float> * p_v_float;
		std::vector<std::string> * p_v_string;
	};
};
