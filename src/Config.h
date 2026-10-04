#pragma once

#include <atomic>

namespace HLO
{
	struct Config
	{
		static Config& Get();

		void Load();
		void Save();

		std::atomic<bool> debug{ false };
		std::atomic<bool> radiusHighlightOnly{ false };
		std::atomic<int32_t> scanHotkeyKeyboard{ -1 };
		std::atomic<int32_t> scanHotkeyGamepad{ -1 };
		std::atomic<float> scanRadius{ 1500.0f };
		std::atomic<bool> scanToggle{ false };
		std::atomic<float> crosshairDuration{ 0.8f };
		std::atomic<float> scanDuration{ 3.0f };

		std::atomic<bool> activatorEnabled{ true };
		std::atomic<bool> talkingActivatorEnabled{ true };
		std::atomic<bool> containerEnabled{ true };
		std::atomic<bool> doorEnabled{ true };
		std::atomic<bool> floraEnabled{ true };
		std::atomic<bool> furnitureEnabled{ true };
		std::atomic<bool> armorEnabled{ true };
		std::atomic<bool> weaponEnabled{ true };
		std::atomic<bool> ammoEnabled{ true };
		std::atomic<bool> bookEnabled{ true };
		std::atomic<bool> ingredientEnabled{ true };
		std::atomic<bool> keyMasterEnabled{ true };
		std::atomic<bool> miscEnabled{ true };
		std::atomic<bool> alchemyItemEnabled{ true };
		std::atomic<bool> scrollEnabled{ true };
		std::atomic<bool> soulGemEnabled{ true };

		std::atomic<bool> ignoreUnnamed{ true };
		std::atomic<bool> ignoreEmptyContainers{ false };

	private:
		Config() = default;
	};
}
