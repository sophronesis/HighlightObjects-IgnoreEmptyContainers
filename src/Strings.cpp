#include "pch.h"
#include "Strings.h"

namespace Strings
{
	const char* Get(std::string_view a_key)
	{
		return Get(a_key, "");
	}

	const char* Get(std::string_view a_key, const char* a_fallback)
	{
		auto it = s_map.find(std::string(a_key));
		if (it != s_map.end() && !it->second.empty())
		{
			return it->second.c_str();
		}
		return a_fallback;
	}

	void Load()
	{
		constexpr auto path = L"Data/SKSE/Plugins/HighlightObjects_Translation.ini";

		CSimpleIniA ini;
		ini.SetUnicode();
		if (ini.LoadFile(path) != SI_OK)
		{
			return;
		}

		CSimpleIniA::TNamesDepend sections;
		ini.GetAllSections(sections);
		for (const auto& section : sections)
		{
			CSimpleIniA::TNamesDepend keys;
			ini.GetAllKeys(section.pItem, keys);
			for (const auto& key : keys)
			{
				const char* val = ini.GetValue(section.pItem, key.pItem);
				if (val)
				{
					s_map[key.pItem] = val;
				}
			}
		}
	}
}
