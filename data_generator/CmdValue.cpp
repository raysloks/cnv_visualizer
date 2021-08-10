#include "CmdValue.h"

CmdValue::CmdValue(int * integer) : type(Type::Integer), p_int(integer)
{
}

CmdValue::CmdValue(float * real) : type(Type::Real), p_float(real)
{
}

CmdValue::CmdValue(bool * flag) : type(Type::Flag), p_bool(flag)
{
}

CmdValue::CmdValue(std::string * string, bool is_file) : p_string(string)
{
	type = is_file ? Type::File : Type::String;
}

CmdValue::CmdValue(std::vector<int> * integers) : type(Type::IntegerMulti), p_v_int(integers)
{
}

CmdValue::CmdValue(std::vector<float> * reals) : type(Type::RealMulti), p_v_float(reals)
{
}

CmdValue::CmdValue(std::vector<std::string> * strings, bool is_file) : p_v_string(strings)
{
	type = is_file ? Type::FileMulti : Type::StringMulti;
}

bool CmdValue::needsValue() const
{
	return type != Type::Flag;
}

int CmdValue::parseValue(const std::string& arg)
{
	switch (type)
	{
	case Type::Integer:
		*p_int = std::stoi(arg);
		return 0;
	case Type::Real:
		*p_float = std::stof(arg);
		return 0;
	case Type::Flag:
		*p_bool = true;
		return 0;
	case Type::File:
		*p_string = arg;
		return 0;
	case Type::String:
		*p_string = arg;
		return 0;
	case Type::IntegerMulti:
		p_v_int->push_back(std::stoi(arg));
		return 0;
	case Type::RealMulti:
		p_v_float->push_back(std::stof(arg));
		return 0;
	case Type::FileMulti:
		p_v_string->push_back(arg);
		return 0;
	case Type::StringMulti:
		p_v_string->push_back(arg);
		return 0;
	default:
		return 1;
	}	
}

const char * CmdValue::getTypeName() const
{
	switch (type)
	{
	case Type::Integer:
		return "INTEGER";
	case Type::Real:
		return "REAL";
	case Type::Flag:
		return "FLAG";
	case Type::File:
		return "FILE";
	case Type::String:
		return "STRING";
	case Type::IntegerMulti:
		return "INTEGER";
	case Type::RealMulti:
		return "REAL";
	case Type::FileMulti:
		return "FILE";
	case Type::StringMulti:
		return "STRING";
	default:
		return "<<ERROR_TYPE>>";
	}
}
