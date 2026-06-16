#include "PCH.h"

#include "SDS/Config.h"

namespace Plugin
{
	constexpr auto NAME = "SimpleDualSheathVR";
	constexpr auto INI_PATH = "Data\\SKSE\\Plugins\\SimpleDualSheathVR.ini";
}

void WriteProbeLog(const char* a_message)
{
	if (auto file = std::fopen("SimpleDualSheathVR_load_probe.txt", "a")) {
		std::fprintf(file, "%s\n", a_message);
		std::fclose(file);
	}
}

std::filesystem::path GetExplicitVRLogPath()
{
	const auto* userProfile = std::getenv("USERPROFILE");
	if (!userProfile) {
		WriteProbeLog("GetExplicitVRLogPath: USERPROFILE missing");
		return "SimpleDualSheathVR.log";
	}

	auto path = std::filesystem::path(userProfile);
	path /= "Documents";
	path /= "My Games";
	path /= "Skyrim VR";
	path /= "SKSE";

	std::filesystem::create_directories(path);

	path /= "SimpleDualSheathVR.log";
	return path;
}

extern "C" int __stdcall DllMain(void*, unsigned long a_reason, void*)
{
	if (a_reason == 1) {
		WriteProbeLog("DllMain: DLL_PROCESS_ATTACH");
	}

	return 1;
}

void InitializeLog()
{
	const auto path = GetExplicitVRLogPath();

	auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path.string(), true);
	auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));

	log->set_level(spdlog::level::info);
	log->flush_on(spdlog::level::info);

	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("[%Y-%m-%d %T.%e] [%l] %v"s);

	WriteProbeLog(("InitializeLog: writing to "s + path.string()).c_str());
}

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

void LoadAndLogConfig()
{
	SDS::Config config;
	const auto loaded = config.Load(Plugin::INI_PATH);

	logger::info("Config path: {}", Plugin::INI_PATH);
	logger::info("Config loaded: {}", loaded);

	LogConfigEntry("Sword", config.m_sword);
	LogConfigEntry("Axe", config.m_axe);
	LogConfigEntry("Mace", config.m_mace);
	LogConfigEntry("Dagger", config.m_dagger);
	LogConfigEntry("Staff", config.m_staff);
	LogConfigEntry("2HSword", config.m_2hSword);
	LogConfigEntry("2HAxe", config.m_2hAxe);
	LogConfigEntry("ShieldOnBack", config.m_shield);

	logger::info("DisableAllScabbards: {}", config.m_disableScabbards);
	logger::info("DisableWeaponNodeSharing: {}", config.m_disableWeapNodeSharing);
	logger::info("NPC EquipLeft: {}", config.m_npcEquipLeft);
	logger::info("Shield ClenchedHandWorkaround: {}", config.m_shieldHandWorkaround);
	logger::info("Shield ClenchedHandWorkaroundForceIfDrawn: {}", config.m_shwForceIfDrawn);
	logger::info("Shield DisableHideOnSit flags: {}", FlagsToString(config.m_shieldHideFlags));
	logger::info("Shield ToggleKeys has={}, comboKey={}, key={}",
		config.m_shieldToggleKeys.Has(),
		config.m_shieldToggleKeys.GetComboKey(),
		config.m_shieldToggleKeys.GetKey());
}

extern "C" __declspec(dllexport) bool SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
	WriteProbeLog("SKSEPlugin_Load: entered");

	SKSE::Init(a_skse);
	InitializeLog();

	logger::info("{} loaded successfully", Plugin::NAME);

	LoadAndLogConfig();

	WriteProbeLog("SKSEPlugin_Load: complete");

	return true;
}