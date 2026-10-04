#pragma once

namespace HLO
{
	class HighlightManager
	{
	public:
		static HighlightManager& Get();

		void Init();
		void StartPolling();
		void StopPolling();

	private:
		HighlightManager() = default;

		void ApplyShaderToRefr(RE::TESObjectREFR* refr);
		void ModifyShaderData();
		void PollThreadFunc(std::uint32_t generation);
		void ScanAndHighlight(float a_radius);
		std::vector<RE::TESObjectREFR*> CollectScanTargets(float a_radius);
		void SetScanShadersPersistent(bool a_persistent);
		void ToggleTick();
		void StopToggle();

		class InputEventSink : public RE::BSTEventSink<RE::InputEvent*>
		{
		public:
			static InputEventSink* GetSingleton();

		private:
			InputEventSink() = default;

			RE::BSEventNotifyControl ProcessEvent(RE::InputEvent* const* a_event, RE::BSTEventSource<RE::InputEvent*>*) override;
		};

		std::vector<RE::TESEffectShader*> edgeShaderPool;
		std::atomic<std::uint32_t> currentRawHandle{ 0 };
		std::atomic<std::uint32_t> crosshairShaderIdx{ 0 };

		std::atomic<bool> initialized{ false };
		std::atomic<std::uint32_t> pollGeneration{ 0 };

		std::atomic<bool> toggleActive{ false };
		std::uint32_t toggleShaderIdx{ 0 };

		std::unique_ptr<std::thread> pollThread;
	};
}
