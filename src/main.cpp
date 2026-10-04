#include "pch.h"
#include "HighlightManager.h"
#include "Config.h"
#include "Strings.h"
#include "UI.h"

namespace
{
	void MessageHandler(SKSE::MessagingInterface::Message* msg)
	{
		if (!msg)
		{
			return;
		}

		switch (msg->type)
		{
		case SKSE::MessagingInterface::kDataLoaded:
			{
				HLO::HighlightManager::Get().Init();
				HLO::HighlightManager::Get().StartPolling();
				Strings::Load();
				UI::Register();
				break;
			}
		case SKSE::MessagingInterface::kPreLoadGame:
			{
				HLO::HighlightManager::Get().StopPolling();
				break;
			}
		case SKSE::MessagingInterface::kPostLoadGame:
			{
				HLO::HighlightManager::Get().StartPolling();
				break;
			}
		default:
			break;
		}
	}
}

SKSEPluginLoad(const SKSE::LoadInterface* skse)
{
	SKSE::Init(skse);

	HLO::Config::Get().Load();

	auto* messaging = SKSE::GetMessagingInterface();
	if (messaging)
	{
		messaging->RegisterListener(MessageHandler);
	}

	logger::info("HighlightObjects plugin loaded");

	return true;
}
