#pragma once
#include <string>
#include <unordered_map>
#include <string_view>

namespace Strings
{
	const char* Get(std::string_view a_key);
	const char* Get(std::string_view a_key, const char* a_fallback);
	void Load();
	inline std::unordered_map<std::string, std::string> s_map;
}
