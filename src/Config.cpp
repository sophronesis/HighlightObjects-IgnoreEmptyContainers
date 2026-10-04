#include "pch.h"
#include "Config.h"

namespace HLO
{
	Config& Config::Get()
	{
		static Config instance;
		return instance;
	}

	void Config::Load()
	{
		constexpr auto path = L"Data/SKSE/Plugins/HighlightObjects.ini";

		CSimpleIniA ini;
		ini.SetUnicode();
		ini.LoadFile(path);

		debug.store(ini.GetBoolValue("General", "debug", false));

		radiusHighlightOnly.store(ini.GetBoolValue("Radius Highlight", "radiusHighlightOnly", false));
		scanHotkeyKeyboard.store(static_cast<int32_t>(ini.GetLongValue("Radius Highlight", "scanHotkeyKeyboard", -1)));
		scanHotkeyGamepad.store(static_cast<int32_t>(ini.GetLongValue("Radius Highlight", "scanHotkeyGamepad", -1)));
		scanRadius.store(static_cast<float>(ini.GetDoubleValue("Radius Highlight", "scanRadius", 1500.0)));
		crosshairDuration.store(static_cast<float>(ini.GetDoubleValue("Highlight Duration", "crosshairDuration", 0.8)));
		scanDuration.store(static_cast<float>(ini.GetDoubleValue("Highlight Duration", "scanDuration", 3.0)));

		activatorEnabled.store(ini.GetBoolValue("Highlight Categories", "activator", true));
		talkingActivatorEnabled.store(ini.GetBoolValue("Highlight Categories", "talkingActivator", true));
		containerEnabled.store(ini.GetBoolValue("Highlight Categories", "container", true));
		doorEnabled.store(ini.GetBoolValue("Highlight Categories", "door", true));
		floraEnabled.store(ini.GetBoolValue("Highlight Categories", "flora", true));
		furnitureEnabled.store(ini.GetBoolValue("Highlight Categories", "furniture", true));
		armorEnabled.store(ini.GetBoolValue("Highlight Categories", "armor", true));
		weaponEnabled.store(ini.GetBoolValue("Highlight Categories", "weapon", true));
		ammoEnabled.store(ini.GetBoolValue("Highlight Categories", "ammo", true));
		bookEnabled.store(ini.GetBoolValue("Highlight Categories", "book", true));
		ingredientEnabled.store(ini.GetBoolValue("Highlight Categories", "ingredient", true));
		keyMasterEnabled.store(ini.GetBoolValue("Highlight Categories", "keyMaster", true));
		miscEnabled.store(ini.GetBoolValue("Highlight Categories", "misc", true));
		alchemyItemEnabled.store(ini.GetBoolValue("Highlight Categories", "alchemyItem", true));
		scrollEnabled.store(ini.GetBoolValue("Highlight Categories", "scroll", true));
		soulGemEnabled.store(ini.GetBoolValue("Highlight Categories", "soulGem", true));

		ignoreUnnamed.store(ini.GetBoolValue("Filter", "ignoreUnnamedObjects", true));
		ignoreEmptyContainers.store(ini.GetBoolValue("Filter", "ignoreEmptyContainers", false));

		if (debug.load())
		{
			spdlog::set_level(spdlog::level::trace);
		}

		Save();

		logger::info("Config loaded: debug={} radiusOnly={} kb={} gp={} radius={} crossDur={} scanDur={}",
			debug.load(), radiusHighlightOnly.load(), scanHotkeyKeyboard.load(), scanHotkeyGamepad.load(), scanRadius.load(),
			crosshairDuration.load(), scanDuration.load());
		if (debug.load())
		{
			logger::info("Categories: Act={} TalkAct={} Cont={} Door={} Flora={} Furn={} Arm={} Wep={} Ammo={} Book={} Ing={} Key={} Misc={} Alch={} Scr={} SG={}",
				activatorEnabled.load(), talkingActivatorEnabled.load(), containerEnabled.load(), doorEnabled.load(),
				floraEnabled.load(), furnitureEnabled.load(), armorEnabled.load(), weaponEnabled.load(),
				ammoEnabled.load(), bookEnabled.load(), ingredientEnabled.load(), keyMasterEnabled.load(),
				miscEnabled.load(), alchemyItemEnabled.load(), scrollEnabled.load(), soulGemEnabled.load());
		}
	}

	void Config::Save()
	{
		constexpr auto path = L"Data/SKSE/Plugins/HighlightObjects.ini";

		CSimpleIniA ini;
		ini.SetUnicode();
		ini.LoadFile(path);

		ini.SetBoolValue("General", "debug", debug.load(), ";Enable detailed logging. Set to true if you are troubleshooting issues.");

		ini.SetBoolValue("Radius Highlight", "radiusHighlightOnly", radiusHighlightOnly.load(),
			";When set to true, only the hotkey radius highlight system works.\n"
			";Objects you look at normally will no longer glow.\n"
			";Default: false");
		ini.SetLongValue("Radius Highlight", "scanHotkeyKeyboard", scanHotkeyKeyboard.load(),
			";Keyboard or mouse button scan code that highlights all nearby interactive objects.\n"
			";Set to -1 to disable this hotkey. Default: -1");
		ini.SetLongValue("Radius Highlight", "scanHotkeyGamepad", scanHotkeyGamepad.load(),
			";Gamepad button code that highlights all nearby interactive objects.\n"
			";Set to -1 to disable this hotkey. Default: -1");
		ini.SetDoubleValue("Radius Highlight", "scanRadius", scanRadius.load(),
			";Radius in game units around your character to scan for interactive objects.\n"
			";Default: 1500.0 (approximately 23 meters)");

		ini.SetDoubleValue("Highlight Duration", "crosshairDuration", crosshairDuration.load(),
			";Duration in seconds for the crosshair highlight effect.\n"
			";Range: 0.1 to 60.0 seconds. Default: 0.8");
		ini.SetDoubleValue("Highlight Duration", "scanDuration", scanDuration.load(),
			";Duration in seconds for the area scan highlight effect.\n"
			";Range: 0.1 to 60.0 seconds. Default: 3.0");

		ini.SetBoolValue("Highlight Categories", "activator", activatorEnabled.load(),
			";Highlight activator objects (levers, buttons, traps, etc.). Default: true");
		ini.SetBoolValue("Highlight Categories", "talkingActivator", talkingActivatorEnabled.load(),
			";Highlight talking activator objects (shrines, word walls, etc.). Default: true");
		ini.SetBoolValue("Highlight Categories", "container", containerEnabled.load(),
			";Highlight containers (chests, barrels, sacks, etc.). Default: true");
		ini.SetBoolValue("Highlight Categories", "door", doorEnabled.load(),
			";Highlight doors and gates. Default: true");
		ini.SetBoolValue("Highlight Categories", "flora", floraEnabled.load(),
			";Highlight harvestable flora (plants, mushrooms, etc.). Default: true");
		ini.SetBoolValue("Highlight Categories", "furniture", furnitureEnabled.load(),
			";Highlight furniture (chairs, beds, crafting stations, etc.). Default: true");
		ini.SetBoolValue("Highlight Categories", "armor", armorEnabled.load(),
			";Highlight loose armor on the ground. Default: true");
		ini.SetBoolValue("Highlight Categories", "weapon", weaponEnabled.load(),
			";Highlight loose weapons on the ground. Default: true");
		ini.SetBoolValue("Highlight Categories", "ammo", ammoEnabled.load(),
			";Highlight loose ammo/arrows on the ground. Default: true");
		ini.SetBoolValue("Highlight Categories", "book", bookEnabled.load(),
			";Highlight books and notes. Default: true");
		ini.SetBoolValue("Highlight Categories", "ingredient", ingredientEnabled.load(),
			";Highlight loose ingredients on the ground. Default: true");
		ini.SetBoolValue("Highlight Categories", "keyMaster", keyMasterEnabled.load(),
			";Highlight keys. Default: true");
		ini.SetBoolValue("Highlight Categories", "misc", miscEnabled.load(),
			";Highlight miscellaneous objects. Default: true");
		ini.SetBoolValue("Highlight Categories", "alchemyItem", alchemyItemEnabled.load(),
			";Highlight potions and poisons. Default: true");
		ini.SetBoolValue("Highlight Categories", "scroll", scrollEnabled.load(),
			";Highlight scrolls. Default: true");
		ini.SetBoolValue("Highlight Categories", "soulGem", soulGemEnabled.load(),
			";Highlight soul gems. Default: true");

		ini.SetBoolValue("Filter", "ignoreUnnamedObjects", ignoreUnnamed.load(),
			";When true, objects without a display name are never highlighted.\n"
			";This filters out unnamed activators, furniture, doors, etc.\n"
			";Default: true");

		ini.SetBoolValue("Filter", "ignoreEmptyContainers", ignoreEmptyContainers.load(),
			";When true, containers you have already emptied are not highlighted.\n"
			";Containers you have never opened are always highlighted.\n"
			";Default: false");

		SI_Error rc = ini.SaveFile(path);
		if (rc < 0)
		{
			logger::warn("Config: failed to save INI, rc={}", rc);
		}
	}
}
