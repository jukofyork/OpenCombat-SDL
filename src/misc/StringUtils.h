#pragma once

#include <string>

namespace StringUtils
{
	// Trim whitespace from both ends of a string
	inline std::string trim(const std::string &s)
	{
		const std::string ws = " \t\n\r\f\v";
		size_t start = s.find_first_not_of(ws);
		if (start == std::string::npos) return "";
		size_t end = s.find_last_not_of(ws);
		return s.substr(start, end - start + 1);
	}
}
