#include "pch.h"
#include "SKSEMenuFramework.h"
#include "UI.h"
#include "Config.h"
#include "Strings.h"

// SKSE Menu Framework 3 headers expose the ImGui API under ImGuiMCP
namespace ImGui = ImGuiMCP;
using namespace ImGuiMCP;

namespace
{
	const char* dxKbNames[] = {
		"[NONE]",
		"Escape", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0",
		"Minus", "Equals", "Backspace", "Tab",
		"Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P",
		"Left Bracket", "Right Bracket", "Enter",
		"Left Control", "A", "S", "D", "F", "G", "H", "J", "K", "L",
		"Semicolon", "Apostrophe", "~ (Console)", "Left Shift", "Back Slash",
		"Z", "X", "C", "V", "B", "N", "M",
		"Comma", "Period", "Forward Slash", "Right Shift",
		"NUM*", "Left Alt", "Spacebar", "Caps Lock",
		"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10",
		"Num Lock", "Scroll Lock",
		"NUM7", "NUM8", "NUM9", "NUM-", "NUM4", "NUM5", "NUM6", "NUM+",
		"NUM1", "NUM2", "NUM3", "NUM0", "NUM.",
		"F11", "F12",
		"NUM Enter", "Right Control",
		"NUM/",
		"SysRq / PrtScr", "Right Alt",
		"Pause",
		"Home", "Up Arrow", "PgUp", "Left Arrow", "Right Arrow", "End", "Down Arrow", "PgDown",
		"Insert", "Delete",
		"Left Mouse Button", "Right Mouse Button", "Middle/Wheel Mouse Button",
		"Mouse Button 3", "Mouse Button 4", "Mouse Button 5",
		"Mouse Button 6", "Mouse Button 7", "Mouse Wheel Up", "Mouse Wheel Down"
	};

	const uint32_t dxKbValues[] = {
		0,
		1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
		12, 13, 14, 15,
		16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
		26, 27, 28,
		29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
		39, 40, 41, 42, 43,
		44, 45, 46, 47, 48, 49, 50,
		51, 52, 53, 54,
		55, 56, 57, 58,
		59, 60, 61, 62, 63, 64, 65, 66, 67, 68,
		69, 70,
		71, 72, 73, 74, 75, 76, 77, 78,
		79, 80, 81, 82, 83,
		87, 88,
		156, 157,
		181,
		183, 184,
		197,
		199, 200, 201, 203, 205, 207, 208, 209,
		210, 211,
		256, 257, 258,
		259, 260, 261,
		262, 263, 264, 265
	};

	constexpr auto kbComboCount = std::size(dxKbValues);

	const char* dxGpNames[] = {
		"[NONE]",
		"DPAD UP", "DPAD DOWN", "DPAD LEFT", "DPAD RIGHT",
		"START", "BACK",
		"LEFT THUMB", "RIGHT THUMB",
		"LEFT SHOULDER", "RIGHT SHOULDER",
		"A", "B", "X", "Y",
		"LT", "RT"
	};

	const uint32_t dxGpValues[] = {
		0,
		266, 267, 268, 269,
		270, 271,
		272, 273,
		274, 275,
		276, 277, 278, 279,
		280, 281
	};

	constexpr auto gpComboCount = std::size(dxGpValues);
}

namespace UI
{
	void HelpMarker(const char* desc)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{ 0.55f, 0.55f, 0.55f, 1.0f });
		ImGui::TextUnformatted("(?)");
		ImGui::PopStyleColor();
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 28.0f);
			ImGui::TextUnformatted(desc);
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
	}

	void __stdcall RenderMenu()
	{
		auto& config = HLO::Config::Get();
		const float winWidth = ImGui::GetWindowWidth();
		bool changed = false;

		ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));

		ImGui::Spacing();
		ImGui::TextColored(ImVec4(0.7f, 0.85f, 1.0f, 1.0f), "%s", Strings::Get("$HLO_SectionDebug$", "-- Debug --"));
		ImGui::Separator();

		{
			bool val = config.debug.load();
			if (ImGui::Checkbox(Strings::Get("$DebugCheckbox$", "Debug"), &val))
			{
				config.debug.store(val);
				spdlog::set_level(val ? spdlog::level::trace : spdlog::level::info);
				changed = true;
			}
			ImGui::SameLine();
			HelpMarker(Strings::Get("$DebugHelp$", "Enables detailed log output for troubleshooting."));
		}

		ImGui::Spacing();
		ImGui::TextColored(ImVec4(0.7f, 0.85f, 1.0f, 1.0f), "%s", Strings::Get("$HLO_SectionFilter$", "-- Filter --"));
		ImGui::Separator();

		{
			bool val = config.ignoreUnnamed.load();
			if (ImGui::Checkbox(Strings::Get("$HLO_IgnoreUnnamed$", "Ignore all objects without a name"), &val))
			{
				config.ignoreUnnamed.store(val);
				changed = true;
			}
			ImGui::SameLine();
			HelpMarker(Strings::Get("$HLO_IgnoreUnnamedHelp$", "When enabled, objects that have no display name will not be highlighted. This filters out unnamed activators, furniture, doors, and other objects."));
		}

		{
			bool val = config.ignoreEmptyContainers.load();
			if (ImGui::Checkbox(Strings::Get("$HLO_IgnoreEmptyContainers$", "Ignore empty containers"), &val))
			{
				config.ignoreEmptyContainers.store(val);
				changed = true;
			}
			ImGui::SameLine();
			HelpMarker(Strings::Get("$HLO_IgnoreEmptyContainersHelp$", "When enabled, containers you have already emptied will not be highlighted. Containers you have never opened are always highlighted, since their leveled loot is not rolled until then."));
		}

		ImGui::Spacing();
		ImGui::TextColored(ImVec4(0.7f, 0.85f, 1.0f, 1.0f), "%s", Strings::Get("$HLO_SectionRadius$", "-- Radius Highlight --"));
		ImGui::Separator();

		{
			bool val = config.radiusHighlightOnly.load();
			if (ImGui::Checkbox(Strings::Get("$RadiusOnlyCheckbox$", "Radius Highlight Only"), &val))
			{
				config.radiusHighlightOnly.store(val);
				changed = true;
			}
			ImGui::SameLine();
			HelpMarker(Strings::Get("$RadiusOnlyHelp$", "When enabled, only the hotkey radius highlight system works. Objects you look at normally will no longer glow."));
		}

		{
			float val = config.scanRadius.load();
			ImGui::SetNextItemWidth(winWidth * 0.45f);
			if (ImGui::SliderFloat(Strings::Get("$ScanRadiusSlider$", "Scan Radius"), &val, 100.0f, 5000.0f, "%.0f"))
			{
				config.scanRadius.store(val);
				changed = true;
			}
			ImGui::SameLine();
			HelpMarker(Strings::Get("$ScanRadiusHelp$", "Radius in game units around your character to scan for interactive objects. Larger values cover more area."));
		}

		ImGui::Spacing();
		ImGui::TextColored(ImVec4(0.7f, 0.85f, 1.0f, 1.0f), "%s", Strings::Get("$HLO_SectionHotkeys$", "-- Hotkeys --"));
		ImGui::Separator();

		{
			ImGui::SetNextItemWidth(winWidth * 0.45f);
			int idx = FindComboIndex(dxKbValues, static_cast<int>(kbComboCount), static_cast<uint32_t>(config.scanHotkeyKeyboard.load()));
			if (ImGui::Combo(Strings::Get("$KBHotkeyLabel$", "Keyboard Hotkey"), &idx, dxKbNames, static_cast<int>(kbComboCount)))
			{
				config.scanHotkeyKeyboard.store(static_cast<int32_t>(dxKbValues[idx]));
				changed = true;
			}
			ImGui::SameLine();
			HelpMarker(Strings::Get("$KBHotkeyHelp$", "Keyboard or mouse button that highlights all nearby interactive objects. Set to [NONE] to disable."));
		}

		{
			ImGui::SetNextItemWidth(winWidth * 0.45f);
			int idx = FindComboIndex(dxGpValues, static_cast<int>(gpComboCount), static_cast<uint32_t>(config.scanHotkeyGamepad.load()));
			if (ImGui::Combo(Strings::Get("$GPHotkeyLabel$", "Gamepad Hotkey"), &idx, dxGpNames, static_cast<int>(gpComboCount)))
			{
				config.scanHotkeyGamepad.store(static_cast<int32_t>(dxGpValues[idx]));
				changed = true;
			}
			ImGui::SameLine();
			HelpMarker(Strings::Get("$GPHotkeyHelp$", "Gamepad button that highlights all nearby interactive objects. Set to [NONE] to disable."));
		}

		ImGui::Spacing();
		ImGui::TextColored(ImVec4(0.7f, 0.85f, 1.0f, 1.0f), "%s", Strings::Get("$HLO_SectionDuration$", "-- Highlight Duration --"));
		ImGui::Separator();

		{
			float val = config.crosshairDuration.load();
			ImGui::SetNextItemWidth(winWidth * 0.45f);
			if (ImGui::SliderFloat(Strings::Get("$CrosshairDurationSlider$", "Crosshair Select Highlight Duration"), &val, 0.1f, 60.0f, "%.1f s"))
			{
				config.crosshairDuration.store(val);
				changed = true;
			}
			ImGui::SameLine();
			HelpMarker(Strings::Get("$CrosshairDurationHelp$", "How long the edge-glow highlight lasts on objects you look at, in seconds."));
		}

		{
			float val = config.scanDuration.load();
			ImGui::SetNextItemWidth(winWidth * 0.45f);
			if (ImGui::SliderFloat(Strings::Get("$ScanDurationSlider$", "Radius Highlight Duration"), &val, 0.1f, 60.0f, "%.1f s"))
			{
				config.scanDuration.store(val);
				changed = true;
			}
			ImGui::SameLine();
			HelpMarker(Strings::Get("$ScanDurationHelp$", "How long the edge-glow highlight lasts on objects found by the area scan, in seconds."));
		}

		ImGui::Spacing();
		ImGui::TextColored(ImVec4(0.7f, 0.85f, 1.0f, 1.0f), "%s", Strings::Get("$HLO_SectionCategories$", "-- Highlight Categories --"));
		ImGui::Separator();

		{
			bool val = config.activatorEnabled.load();
			if (ImGui::Checkbox(Strings::Get("$HLO_CatActivator$", "Activator"), &val))
			{
				config.activatorEnabled.store(val);
				changed = true;
			}
		}

		{
			bool val = config.talkingActivatorEnabled.load();
			if (ImGui::Checkbox(Strings::Get("$HLO_CatTalkingActivator$", "Talking Activator"), &val))
			{
				config.talkingActivatorEnabled.store(val);
				changed = true;
			}
		}

		{
			bool val = config.containerEnabled.load();
			if (ImGui::Checkbox(Strings::Get("$HLO_CatContainer$", "Container"), &val))
			{
				config.containerEnabled.store(val);
				changed = true;
			}
		}

		{
			bool val = config.doorEnabled.load();
			if (ImGui::Checkbox(Strings::Get("$HLO_CatDoor$", "Door"), &val))
			{
				config.doorEnabled.store(val);
				changed = true;
			}
		}

		{
			bool val = config.floraEnabled.load();
			if (ImGui::Checkbox(Strings::Get("$HLO_CatFlora$", "Flora"), &val))
			{
				config.floraEnabled.store(val);
				changed = true;
			}
		}

		{
			bool val = config.furnitureEnabled.load();
			if (ImGui::Checkbox(Strings::Get("$HLO_CatFurniture$", "Furniture"), &val))
			{
				config.furnitureEnabled.store(val);
				changed = true;
			}
		}

		{
			bool val = config.armorEnabled.load();
			if (ImGui::Checkbox(Strings::Get("$HLO_CatArmor$", "Armor"), &val))
			{
				config.armorEnabled.store(val);
				changed = true;
			}
		}

		{
			bool val = config.weaponEnabled.load();
			if (ImGui::Checkbox(Strings::Get("$HLO_CatWeapon$", "Weapon"), &val))
			{
				config.weaponEnabled.store(val);
				changed = true;
			}
		}

		{
			bool val = config.ammoEnabled.load();
			if (ImGui::Checkbox(Strings::Get("$HLO_CatAmmo$", "Ammo"), &val))
			{
				config.ammoEnabled.store(val);
				changed = true;
			}
		}

		{
			bool val = config.bookEnabled.load();
			if (ImGui::Checkbox(Strings::Get("$HLO_CatBook$", "Book"), &val))
			{
				config.bookEnabled.store(val);
				changed = true;
			}
		}

		{
			bool val = config.ingredientEnabled.load();
			if (ImGui::Checkbox(Strings::Get("$HLO_CatIngredient$", "Ingredient"), &val))
			{
				config.ingredientEnabled.store(val);
				changed = true;
			}
		}

		{
			bool val = config.keyMasterEnabled.load();
			if (ImGui::Checkbox(Strings::Get("$HLO_CatKeyMaster$", "Key"), &val))
			{
				config.keyMasterEnabled.store(val);
				changed = true;
			}
		}

		{
			bool val = config.miscEnabled.load();
			if (ImGui::Checkbox(Strings::Get("$HLO_CatMisc$", "Misc"), &val))
			{
				config.miscEnabled.store(val);
				changed = true;
			}
		}

		{
			bool val = config.alchemyItemEnabled.load();
			if (ImGui::Checkbox(Strings::Get("$HLO_CatAlchemyItem$", "Potion / Poison"), &val))
			{
				config.alchemyItemEnabled.store(val);
				changed = true;
			}
		}

		{
			bool val = config.scrollEnabled.load();
			if (ImGui::Checkbox(Strings::Get("$HLO_CatScroll$", "Scroll"), &val))
			{
				config.scrollEnabled.store(val);
				changed = true;
			}
		}

		{
			bool val = config.soulGemEnabled.load();
			if (ImGui::Checkbox(Strings::Get("$HLO_CatSoulGem$", "Soul Gem"), &val))
			{
				config.soulGemEnabled.store(val);
				changed = true;
			}
		}

		if (changed)
		{
			config.Save();
		}

		ImGui::PopStyleColor();
	}

	void Register()
	{
		if (!SKSEMenuFramework::IsInstalled())
		{
			SKSE::log::warn("SKSE Menu Framework not installed, skipping UI registration");
			return;
		}

		SKSEMenuFramework::SetSection(Strings::Get("$ModName$", "Highlight Objects"));
		SKSEMenuFramework::AddSectionItem(Strings::Get("$SettingsLabel$", "Settings"), RenderMenu);

		SKSE::log::info("UI registered with SKSE Menu Framework");
	}
}
