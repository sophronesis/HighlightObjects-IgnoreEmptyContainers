#pragma once

namespace UI
{
	void Register();
	void __stdcall RenderMenu();
	void HelpMarker(const char* desc);

	inline int FindComboIndex(const uint32_t* values, int count, uint32_t target)
	{
		for (int i = 0; i < count; ++i)
		{
			if (values[i] == target) return i;
		}
		return 0;
	}
}
