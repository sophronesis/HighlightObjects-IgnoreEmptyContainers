#include "pch.h"
#include "HighlightManager.h"
#include "Config.h"

namespace HLO
{
	namespace
	{
		constexpr int EDGE_COLOR_R = 255;
		constexpr int EDGE_COLOR_G = 215;
		constexpr int EDGE_COLOR_B = 0;
		constexpr float EDGE_ALPHA = 1.0f;
		constexpr float EDGE_WIDTH = 1.0f;
		constexpr float FADE_IN = 0.1f;
		constexpr float FADE_OUT = 0.3f;
		constexpr float FILL_ALPHA_RATIO = 0.05f;
		constexpr float FILL_PERSIST_RATIO = 0.01f;
		constexpr float FILL_COLORKEY_ALPHA = 20.0f;
		constexpr float CROSSHAIR_FULL_ALPHA = 0.4f;
		constexpr float SCAN_FULL_ALPHA = 2.6f;
		constexpr int POLL_INTERVAL_MS = 100;
		constexpr int TOGGLE_TICK_MS = 500;
		constexpr float TOGGLE_FULL_ALPHA = 1.0e6f;

		bool IsInteractableFormType(RE::FormType a_type)
		{
			auto& config = Config::Get();
			switch (a_type)
			{
			case RE::FormType::Activator:        return config.activatorEnabled.load();
			case RE::FormType::TalkingActivator: return config.talkingActivatorEnabled.load();
			case RE::FormType::Container:        return config.containerEnabled.load();
			case RE::FormType::Door:             return config.doorEnabled.load();
			case RE::FormType::Flora:            return config.floraEnabled.load();
			case RE::FormType::Furniture:        return config.furnitureEnabled.load();
			case RE::FormType::Armor:            return config.armorEnabled.load();
			case RE::FormType::Weapon:           return config.weaponEnabled.load();
			case RE::FormType::Ammo:             return config.ammoEnabled.load();
			case RE::FormType::Book:             return config.bookEnabled.load();
			case RE::FormType::Ingredient:       return config.ingredientEnabled.load();
			case RE::FormType::KeyMaster:        return config.keyMasterEnabled.load();
			case RE::FormType::Misc:             return config.miscEnabled.load();
			case RE::FormType::AlchemyItem:      return config.alchemyItemEnabled.load();
			case RE::FormType::Scroll:           return config.scrollEnabled.load();
			case RE::FormType::SoulGem:          return config.soulGemEnabled.load();
			default: return false;
			}
		}

		// Containers resolve their leveled loot lazily, so a container without
		// ExtraContainerChanges has never been initialized and counts as non-empty.
		// Once initialized, leveled list entries from the base form are ignored:
		// they were rolled into concrete items in the changes list.
		bool IsEmptyContainer(RE::TESObjectREFR* a_refr)
		{
			if (!a_refr->extraList.HasType(RE::ExtraDataType::kContainerChanges))
			{
				return false;
			}

			auto inventory = a_refr->GetInventory([](RE::TESBoundObject& a_obj) {
				return !a_obj.Is(RE::FormType::LeveledItem);
			});

			for (const auto& [obj, data] : inventory)
			{
				if (data.first > 0)
				{
					return false;
				}
			}
			return true;
		}

		bool ShouldSkipEmptyContainer(RE::TESObjectREFR* a_refr, RE::TESBoundObject* a_base)
		{
			return a_base->Is(RE::FormType::Container) &&
			       Config::Get().ignoreEmptyContainers.load() &&
			       IsEmptyContainer(a_refr);
		}
	}

	HighlightManager& HighlightManager::Get()
	{
		static HighlightManager instance;
		return instance;
	}

	void HighlightManager::Init()
	{
		auto* primary = RE::TESForm::LookupByEditorID<RE::TESEffectShader>("HLOEdgeHighlightFX");
		if (primary)
		{
			edgeShaderPool.push_back(primary);
		}

		for (int i = 2; i <= 128; ++i)
		{
			char buf[64];
			snprintf(buf, sizeof(buf), "HLOEdgeHighlightFX%02d", i);
			auto* shader = RE::TESForm::LookupByEditorID<RE::TESEffectShader>(buf);
			if (shader)
			{
				edgeShaderPool.push_back(shader);
			}
		}

		if (edgeShaderPool.empty())
		{
			logger::error("No TESEffectShader forms found");
			return;
		}

		ModifyShaderData();

		RE::BSInputDeviceManager::GetSingleton()->AddEventSink(InputEventSink::GetSingleton());

		const size_t shaderCount = edgeShaderPool.size();
		if (shaderCount == 128)
		{
			logger::info("HighlightObjects initialized, all 128 shaders loaded");
		}
		else
		{
			logger::warn("HighlightObjects initialized, only {}/128 shaders loaded", shaderCount);
			for (size_t si = 0; si < shaderCount; ++si)
			{
				auto* s = edgeShaderPool[si];
				logger::info("  shader[{}] FormID=0x{:08x}", si, s->GetFormID());
			}
			logger::warn("Missing shader EditorIDs (expected 128, got {})", shaderCount);
		}
		initialized.store(true);
	}

	void HighlightManager::ModifyShaderData()
	{
		for (size_t i = 0; i < edgeShaderPool.size(); ++i)
		{
			auto* shader = edgeShaderPool[i];
			if (!shader)
			{
				continue;
			}

			auto& data = shader->data;

			data.flags.reset(RE::EffectShaderData::Flags::kDisableTextureShader);

			data.fillTextureEffectFullAlphaRatio = FILL_ALPHA_RATIO;
			data.fillTextureEffectPersistentAlphaRatio = FILL_PERSIST_RATIO;
			data.fillTextureEffectColorKey1.red = static_cast<std::uint8_t>(EDGE_COLOR_R);
			data.fillTextureEffectColorKey1.green = static_cast<std::uint8_t>(EDGE_COLOR_G);
			data.fillTextureEffectColorKey1.blue = static_cast<std::uint8_t>(EDGE_COLOR_B);
			data.fillTextureEffectColorKey1.alpha = static_cast<std::uint8_t>(EDGE_ALPHA * FILL_COLORKEY_ALPHA);

			data.edgeEffectColor.red = static_cast<std::uint8_t>(EDGE_COLOR_R);
			data.edgeEffectColor.green = static_cast<std::uint8_t>(EDGE_COLOR_G);
			data.edgeEffectColor.blue = static_cast<std::uint8_t>(EDGE_COLOR_B);
			data.edgeEffectColor.alpha = static_cast<std::uint8_t>(EDGE_ALPHA * 255.0f);

			data.edgeColor.red = static_cast<std::uint8_t>(EDGE_COLOR_R);
			data.edgeColor.green = static_cast<std::uint8_t>(EDGE_COLOR_G);
			data.edgeColor.blue = static_cast<std::uint8_t>(EDGE_COLOR_B);
			data.edgeColor.alpha = static_cast<std::uint8_t>(EDGE_ALPHA * 255.0f);

			data.edgeWidthAlphaUnits = EDGE_WIDTH;
			data.edgeEffectAlphaFadeInTime = FADE_IN;
			data.edgeEffectAlphaFadeOutTime = FADE_OUT;

			data.edgeEffectFullAlphaRatio = 1.0f;
			data.edgeEffectPersistentAlphaRatio = 0.0f;

			data.edgeEffectFallOff = 1.0f;

			data.edgeEffectFullAlphaTime = (i < 4) ? CROSSHAIR_FULL_ALPHA : SCAN_FULL_ALPHA;
		}
	}

	void HighlightManager::ApplyShaderToRefr(RE::TESObjectREFR* refr)
	{
		if (!refr || edgeShaderPool.size() < 4)
		{
			return;
		}

		if (refr->IsDisabled())
		{
			return;
		}

		if (!refr->Is3DLoaded())
		{
			return;
		}

		if (refr->As<RE::Actor>())
		{
			return;
		}

		if ((refr->GetFormFlags() & RE::TESObjectREFR::RecordFlags::kHarvested) != 0)
		{
			return;
		}

		auto* baseObj = refr->GetBaseObject();
		if (!baseObj || !IsInteractableFormType(baseObj->GetFormType()))
		{
			return;
		}

		if (Config::Get().ignoreUnnamed.load())
		{
			const char* name = refr->GetName();
			if (!name || name[0] == '\0')
			{
				return;
			}
		}

		if (ShouldSkipEmptyContainer(refr, baseObj))
		{
			return;
		}

		uint32_t idx = crosshairShaderIdx.fetch_add(1) & 3;
		auto* shader = edgeShaderPool[idx];
		float duration = Config::Get().crosshairDuration.load();
		float fullAlpha = duration - 0.4f;
		if (fullAlpha < 0.01f) fullAlpha = 0.01f;
		shader->data.edgeEffectFullAlphaTime = fullAlpha;
		refr->ApplyEffectShader(shader, duration + 1.0f);

		if (Config::Get().debug.load())
		{
			logger::info("Applied edge shader to 0x{:08x}", refr->GetFormID());
		}
	}

	void HighlightManager::PollThreadFunc(std::uint32_t generation)
	{
		int toggleTickElapsed = 0;
		while (pollGeneration.load() == generation)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(POLL_INTERVAL_MS));

			if (pollGeneration.load() != generation || !initialized.load())
			{
				return;
			}

			toggleTickElapsed += POLL_INTERVAL_MS;
			if (toggleActive.load() && toggleTickElapsed >= TOGGLE_TICK_MS)
			{
				toggleTickElapsed = 0;
				if (auto* taskInt = SKSE::GetTaskInterface())
				{
					taskInt->AddTask([this, generation]() {
						if (pollGeneration.load() == generation)
						{
							ToggleTick();
						}
					});
				}
			}

			if (HLO::Config::Get().radiusHighlightOnly.load())
			{
				continue;
			}

			auto* crosshair = RE::CrosshairPickData::GetSingleton();
			if (!crosshair)
			{
				continue;
			}

			std::uint32_t rawHandle = crosshair->GetActiveTarget().native_handle();

			if (rawHandle == 0 || rawHandle == 0xFFFFFFFF)
			{
				currentRawHandle.store(0);
				continue;
			}

			if (rawHandle == currentRawHandle.load())
			{
				continue;
			}

			currentRawHandle.store(rawHandle);

			auto* taskInt = SKSE::GetTaskInterface();
			if (!taskInt)
			{
				continue;
			}

			taskInt->AddTask([this, generation]() {
				if (pollGeneration.load() != generation)
				{
					return;
				}

				auto* crosshair2 = RE::CrosshairPickData::GetSingleton();
				if (!crosshair2)
				{
					return;
				}

				auto refrPtr = crosshair2->GetActiveTarget().get();
				auto* refr = refrPtr.get();

				if (refr && refr != RE::PlayerCharacter::GetSingleton())
				{
					ApplyShaderToRefr(refr);
				}
			});
		}
	}

	std::vector<RE::TESObjectREFR*> HighlightManager::CollectScanTargets(float a_radius)
	{
		auto* player = RE::PlayerCharacter::GetSingleton();
		if (!player)
		{
			return {};
		}

		auto* cell = player->GetParentCell();
		if (!cell || !cell->IsAttached())
		{
			return {};
		}

		const RE::NiPoint3 playerPos = player->GetPosition();

		std::vector<RE::TESObjectREFR*> targets;
		int totalVisited = 0;
		int skippedNull = 0;
		int skippedPlayer = 0;
		int skippedNoBaseObj = 0;
		int skippedFormType = 0;
		int skippedDisabled = 0;
		int skippedNo3D = 0;
		int skippedActor = 0;
		int skippedHarvested = 0;
		int skippedUnnamed = 0;
		int skippedEmpty = 0;

		RE::TES::GetSingleton()->ForEachReferenceInRange(player, a_radius,
			[&](RE::TESObjectREFR* refr) -> RE::BSContainer::ForEachResult {
				++totalVisited;

				if (!refr)
				{
					++skippedNull;
					return RE::BSContainer::ForEachResult::kContinue;
				}

				if (refr == player)
				{
					++skippedPlayer;
					return RE::BSContainer::ForEachResult::kContinue;
				}

				auto* baseObj = refr->GetBaseObject();
				if (!baseObj)
				{
					++skippedNoBaseObj;
					return RE::BSContainer::ForEachResult::kContinue;
				}

				if (!IsInteractableFormType(baseObj->GetFormType()))
				{
					++skippedFormType;
					return RE::BSContainer::ForEachResult::kContinue;
				}

				if (Config::Get().ignoreUnnamed.load())
				{
					const char* name = refr->GetName();
					if (!name || name[0] == '\0')
					{
						++skippedUnnamed;
						return RE::BSContainer::ForEachResult::kContinue;
					}
				}

				if (refr->IsDisabled())
				{
					++skippedDisabled;
					return RE::BSContainer::ForEachResult::kContinue;
				}

				if (!refr->Is3DLoaded())
				{
					++skippedNo3D;
					return RE::BSContainer::ForEachResult::kContinue;
				}

				if (refr->As<RE::Actor>())
				{
					++skippedActor;
					return RE::BSContainer::ForEachResult::kContinue;
				}

				if ((refr->GetFormFlags() & RE::TESObjectREFR::RecordFlags::kHarvested) != 0)
				{
					++skippedHarvested;
					return RE::BSContainer::ForEachResult::kContinue;
				}

				if (ShouldSkipEmptyContainer(refr, baseObj))
				{
					++skippedEmpty;
					return RE::BSContainer::ForEachResult::kContinue;
				}

				targets.push_back(refr);
				return RE::BSContainer::ForEachResult::kContinue;
			});

		if (HLO::Config::Get().debug.load())
		{
			logger::info("Area scan: visited={} accepted={} null={} player={} noBase={} wrongType={} disabled={} no3D={} actor={} harvested={} unnamed={} empty={}",
				totalVisited, targets.size(), skippedNull, skippedPlayer, skippedNoBaseObj,
				skippedFormType, skippedDisabled, skippedNo3D, skippedActor, skippedHarvested, skippedUnnamed, skippedEmpty);
		}

		std::sort(targets.begin(), targets.end(),
			[playerPos](RE::TESObjectREFR* a, RE::TESObjectREFR* b) {
				float distA = (a->GetPosition() - playerPos).SqrLength();
				float distB = (b->GetPosition() - playerPos).SqrLength();
				return distA < distB;
			});

		return targets;
	}

	// Scan shaders normally fade out after their full-alpha time. In toggle mode the
	// effects run until stopped, so keep them at full alpha for their whole lifetime.
	void HighlightManager::SetScanShadersPersistent(bool a_persistent)
	{
		for (size_t i = 4; i < edgeShaderPool.size(); ++i)
		{
			auto& data = edgeShaderPool[i]->data;
			data.edgeEffectPersistentAlphaRatio = a_persistent ? 1.0f : 0.0f;
			data.fillTextureEffectPersistentAlphaRatio = a_persistent ? FILL_ALPHA_RATIO : FILL_PERSIST_RATIO;
		}
	}

	void HighlightManager::ScanAndHighlight(float a_radius)
	{
		if (edgeShaderPool.empty() || !initialized.load())
		{
			return;
		}

		auto targets = CollectScanTargets(a_radius);
		if (targets.empty())
		{
			return;
		}

		const size_t totalPool = edgeShaderPool.size();
		if (totalPool < 5)
		{
			logger::warn("ScanAndHighlight: need at least 5 shaders, have {}", totalPool);
			return;
		}
		const size_t poolSize = totalPool - 4;

		SetScanShadersPersistent(false);

		float scanDuration = Config::Get().scanDuration.load();
		float scanFullAlpha = scanDuration - 0.4f;
		if (scanFullAlpha < 0.01f) scanFullAlpha = 0.01f;

		for (size_t i = 0; i < targets.size(); ++i)
		{
			auto* shader = edgeShaderPool[(i % poolSize) + 4];
			shader->data.edgeEffectFullAlphaTime = scanFullAlpha;
			targets[i]->ApplyEffectShader(shader, scanDuration + 1.0f);
		}

		if (HLO::Config::Get().debug.load())
		{
			logger::info("Area scan applied shaders to {} objects (scan pool={}, total={})", targets.size(), poolSize, totalPool);
		}
	}

	// Runs on the main thread every TOGGLE_TICK_MS while toggle mode is on: keeps a
	// persistent scan shader on every eligible object in range and stops the ones on
	// objects that left the radius or stopped qualifying (looted, harvested, ...).
	void HighlightManager::ToggleTick()
	{
		if (!toggleActive.load() || !initialized.load())
		{
			return;
		}

		if (!Config::Get().scanToggle.load())
		{
			StopToggle();
			return;
		}

		const size_t totalPool = edgeShaderPool.size();
		if (totalPool < 5)
		{
			return;
		}
		const size_t poolSize = totalPool - 4;

		auto targets = CollectScanTargets(Config::Get().scanRadius.load());
		std::unordered_set<RE::TESObjectREFR*> wanted(targets.begin(), targets.end());
		std::unordered_set<RE::TESEffectShader*> scanShaders(edgeShaderPool.begin() + 4, edgeShaderPool.end());
		std::unordered_set<RE::TESObjectREFR*> covered;
		int stopped = 0;

		if (auto* processLists = RE::ProcessLists::GetSingleton())
		{
			processLists->ForEachShaderEffect([&](RE::ShaderReferenceEffect* a_effect) {
				if (!a_effect || a_effect->finished || !scanShaders.contains(a_effect->effectData))
				{
					return RE::BSContainer::ForEachResult::kContinue;
				}

				auto refPtr = a_effect->target.get();
				auto* ref = refPtr.get();
				if (ref && wanted.contains(ref) && !covered.contains(ref))
				{
					covered.insert(ref);
				}
				else
				{
					a_effect->finished = true;
					++stopped;
				}
				return RE::BSContainer::ForEachResult::kContinue;
			});
		}

		SetScanShadersPersistent(true);

		int applied = 0;
		for (auto* refr : targets)
		{
			if (covered.contains(refr))
			{
				continue;
			}
			auto* shader = edgeShaderPool[(toggleShaderIdx++ % poolSize) + 4];
			shader->data.edgeEffectFullAlphaTime = TOGGLE_FULL_ALPHA;
			refr->ApplyEffectShader(shader, -1.0f);
			++applied;
		}

		if (Config::Get().debug.load() && (applied || stopped))
		{
			logger::info("Toggle tick: targets={} applied={} stopped={}", targets.size(), applied, stopped);
		}
	}

	// Main thread only.
	void HighlightManager::StopToggle()
	{
		toggleActive.store(false);

		std::unordered_set<RE::TESEffectShader*> scanShaders;
		if (edgeShaderPool.size() > 4)
		{
			scanShaders.insert(edgeShaderPool.begin() + 4, edgeShaderPool.end());
		}

		if (auto* processLists = RE::ProcessLists::GetSingleton())
		{
			processLists->ForEachShaderEffect([&](RE::ShaderReferenceEffect* a_effect) {
				if (a_effect && scanShaders.contains(a_effect->effectData))
				{
					a_effect->finished = true;
				}
				return RE::BSContainer::ForEachResult::kContinue;
			});
		}

		SetScanShadersPersistent(false);

		if (Config::Get().debug.load())
		{
			logger::info("Toggle highlight off");
		}
	}

	void HighlightManager::StartPolling()
	{
		pollGeneration.fetch_add(1);

		if (!initialized.load() || edgeShaderPool.empty())
		{
			logger::warn("StartPolling: not initialized or shader missing");
			return;
		}

		pollThread = std::make_unique<std::thread>(&HighlightManager::PollThreadFunc, this, pollGeneration.load());

		logger::info("Crosshair polling started (gen={})", pollGeneration.load());
	}

	void HighlightManager::StopPolling()
	{
		pollGeneration.fetch_add(1);

		if (pollThread && pollThread->joinable())
		{
			pollThread->join();
		}
		pollThread.reset();

		currentRawHandle.store(0);
		toggleActive.store(false);
	}

	HighlightManager::InputEventSink* HighlightManager::InputEventSink::GetSingleton()
	{
		static InputEventSink instance;
		return &instance;
	}

	RE::BSEventNotifyControl HighlightManager::InputEventSink::ProcessEvent(
		RE::InputEvent* const* a_event, RE::BSTEventSource<RE::InputEvent*>*)
	{
		if (!a_event)
		{
			return RE::BSEventNotifyControl::kContinue;
		}

		auto& config = HLO::Config::Get();
		const int32_t kbKey = config.scanHotkeyKeyboard.load();
		const int32_t gpKey = config.scanHotkeyGamepad.load();

		for (auto* event = *a_event; event; event = event->next)
		{
			auto* buttonEvent = event->AsButtonEvent();
			if (!buttonEvent || !buttonEvent->IsDown())
			{
				continue;
			}

			const auto device = buttonEvent->GetDevice();
			uint32_t rawKey = buttonEvent->GetIDCode();
			if (device == RE::INPUT_DEVICE::kMouse) {
				rawKey += 256;
			} else if (device == RE::INPUT_DEVICE::kGamepad) {
				rawKey = SKSE::InputMap::GamepadMaskToKeycode(rawKey);
			}
			const auto keyCode = static_cast<int32_t>(rawKey);

			const bool isKbMatch = (kbKey >= 0) &&
				(device == RE::INPUT_DEVICE::kKeyboard || device == RE::INPUT_DEVICE::kMouse) &&
				(keyCode == kbKey);

			const bool isGpMatch = (gpKey >= 0) &&
				(device == RE::INPUT_DEVICE::kGamepad) &&
				(keyCode == gpKey);

			if (!isKbMatch && !isGpMatch)
			{
				continue;
			}

			auto* ui = RE::UI::GetSingleton();
			if (ui && (ui->IsMenuOpen(RE::Console::MENU_NAME) || ui->GameIsPaused()))
			{
				continue;
			}

			const float radius = config.scanRadius.load();

			auto* taskInt = SKSE::GetTaskInterface();
			if (!taskInt)
			{
				continue;
			}

			if (config.scanToggle.load())
			{
				auto& manager = HLO::HighlightManager::Get();
				const bool enable = !manager.toggleActive.load();
				manager.toggleActive.store(enable);
				taskInt->AddTask([enable]() {
					auto& mgr = HLO::HighlightManager::Get();
					if (enable)
					{
						mgr.ToggleTick();
					}
					else
					{
						mgr.StopToggle();
					}
				});
				continue;
			}

			taskInt->AddTask([radius]() {
				HLO::HighlightManager::Get().ScanAndHighlight(radius);
			});

			if (config.debug.load())
			{
				logger::info("Area scan hotkey pressed (key={}, radius={})", keyCode, radius);
			}
		}

		return RE::BSEventNotifyControl::kContinue;
	}
}
