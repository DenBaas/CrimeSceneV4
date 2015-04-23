#include "Utils.h"
#include <algorithm>
#include <iostream>
#include <string>
#include <Windows.h>

/*
Splits a string on a deliminator and returns a std::vector containing all substrings
s = input string
delim = deliminator, what string to split on
keep_empty = (default true) whether or not to keep empty strings
Author: Bas Rops - 25-04-2014
Last edit: <name> - dd-mm-yyyy
*/
std::vector<std::string> Utils::splitString(const std::string& s, const std::string& delim, const bool keep_empty)
{
	std::vector<std::string> result;

	if (delim.empty())
	{
		result.push_back(s);
		return result;
	}

	std::string::const_iterator substart = s.begin(), subend;

	while (true)
	{
		subend = std::search(substart, s.end(), delim.begin(), delim.end());
		std::string temp(substart, subend);

		if (keep_empty || !temp.empty())
		{
			result.push_back(temp);
		}

		if (subend == s.end())
		{
			break;
		}
		substart = subend + delim.size();
	}

	return result;
}
