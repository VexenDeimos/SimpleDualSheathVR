#include "PCH.h"

#include "SDS/Config.h"
#include "SDS/Controller.h"
#include "SDS/Data.h"
#include "SDS/PluginState.h"
#include "SDS/RuntimeManager.h"
#include "SDS/ActionEventHandler.h"

namespace
{
	std::unique_ptr<SDS::Controller> g_controller;

	std::string FlagsToString(SDS::Data::Flags a_flags)
	{
		std::vector<std::string> parts;

		if (SDS::Data::HasFlag(a_flags, SDS::Data::Flags::kPlayer)) {
			parts.emplace_back("Player");
		}

		if (SDS::Data::HasFlag(a_flags, SDS::Data::Flags::kNPC)) {
			parts.emplace_back("NPC");
		}

		if (SDS::Data::HasFlag(a_flags, SDS::Data::Flags::kRight)) {
			parts.emplace_back("Right");
		}

		if (SDS::Data::HasFlag(a_flags, SDS::Data::Flags::kSwap)) {
			parts.emplace_back("Swap");
		}

		if (SDS::Data::HasFlag(a_flags, SDS::Data::Flags::kFirstPerson)) {
			parts.emplace_back("FirstPerson");
		}

		if (SDS::Data::HasFlag(a_flags, SDS::Data::Flags::kMountOnly)) {
			parts.emplace_back("MountOnly");
		}

		if (parts.empty()) {
			return "None";
		}

		std::string output;
		for (std::size_t i = 0; i < parts.size(); ++i) {
			if (i > 0) {
				output += "|";
			}

			output += parts[i];
		}

		return output;
	}

	void LogConfigEntry(const char* a_name, const SDS::Config::ConfigEntry& a_entry)
	{
		logger::info("{}: enabled={}, flags={}, sheathNode={}",
			a_name,
			a_entry.IsEnabled(),
			FlagsToString(a_entry.m_flags),
			a_entry.m_sheathNode);
	}
}

namespace SDS::PluginState
{
	void LoadConfigAndInitializeController(const char* a_iniPath)
	{
		SDS::Config config;
		const auto loaded = config.Load(a_iniPath);

		logger::info("Config path: {}", a_iniPath);
		logger::info("Config loaded: {}", loaded);

		LogConfigEntry("Sword", config.m_sword);
		LogConfigEntry("Axe", config.m_axe);
		LogConfigEntry("Mace", config.m_mace);
		LogConfigEntry("Dagger", config.m_dagger);
		LogConfigEntry("Staff", config.m_staff);
		LogConfigEntry("2HSword", config.m_2hSword);
		LogConfigEntry("2HAxe", config.m_2hAxe);
		LogConfigEntry("ShieldOnBack", config.m_shield);

		g_controller = std::make_unique<SDS::Controller>(config);
		g_controller->InitializeData();

		SDS::RuntimeManager::SetController(g_controller.get());
		SDS::RuntimeManager::Configure(
			config.m_runtimePollingEnabled,
			config.m_runtimePollingIntervalMS);
		SDS::ActionEventHandler::Configure(config.m_runtimeLogActionEvents);

		logger::info("Controller initialized: strings={}, weaponData={}, shieldSwitch={}",
			g_controller->GetStringHolder() != nullptr,
			g_controller->GetWeaponData() != nullptr,
			g_controller->GetShieldOnBackSwitch());
	}
}