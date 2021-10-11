#pragma once

#include <vector>
#include <string>

std::vector<std::string> split(const std::string& string, char delimiter);

std::string trim(const std::string& string);

std::string unwrap(const std::string& string, char wrapper = '"');
