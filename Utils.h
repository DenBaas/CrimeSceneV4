#pragma once
#include <vector>

class Utils
{
public:
	static std::vector<std::string> splitString(const std::string& s, const std::string& delim, const bool keep_empty = true);
};

