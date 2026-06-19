#include "PCH.h"

#include "SDS/ActionEventHandler.h"
#include "SDS/PluginState.h"
#include "SDS/RuntimeManager.h"

namespace Plugin
{
	constexpr auto NAME = "SimpleDualSheathVR";
	constexpr auto INI_PATH = "Data\\SKSE\\Plugins\\SimpleDualSheathVR.ini";
}

std::filesystem::path GetExplicitVRLogPath()
{
	const auto* userProfile = std::getenv("USERPROFILE");
	if (!userProfile) {
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

void InitializeLog()
{
	const auto path = GetExplicitVRLogPath();

	auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path.string(), true);
	auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));

	log->set_level(spdlog::level::info);
	log->flush_on(spdlog::level::info);

	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("[%Y-%m-%d %T.%e] [%l] %v"s);
}

void OnSKSEMessage(SKSE::MessagingInterface::Message* a_message)
{
	if (!a_message) {
		return;
	}

	switch (a_message->type) {
	case SKSE::MessagingInterface::kPostLoad:
		logger::info("SKSE message: kPostLoad");
		break;
	case SKSE::MessagingInterface::kPostPostLoad:
		logger::info("SKSE message: kPostPostLoad");
		break;
	case SKSE::MessagingInterface::kPreLoadGame:
		logger::info("SKSE message: kPreLoadGame");
		SDS::RuntimeManager::StopPolling();
		break;
	case SKSE::MessagingInterface::kPostLoadGame:
		logger::info("SKSE message: kPostLoadGame");
		SDS::RuntimeManager::Run("kPostLoadGame");
		SDS::RuntimeManager::StartPolling("kPostLoadGame");
		break;
	case SKSE::MessagingInterface::kSaveGame:
		logger::info("SKSE message: kSaveGame");
		break;
	case SKSE::MessagingInterface::kDeleteGame:
		logger::info("SKSE message: kDeleteGame");
		break;
	case SKSE::MessagingInterface::kInputLoaded:
		logger::info("SKSE message: kInputLoaded");
		break;
	case SKSE::MessagingInterface::kNewGame:
		logger::info("SKSE message: kNewGame");
		SDS::RuntimeManager::Run("kNewGame");
		SDS::RuntimeManager::StartPolling("kNewGame");
		break;
	case SKSE::MessagingInterface::kDataLoaded:
		logger::info("SKSE message: kDataLoaded");
		SDS::ActionEventHandler::Register();
		break;
	default:
		logger::info("SKSE message: unknown type={}", a_message->type);
		break;
	}
}

void RegisterSKSEMessaging()
{
	const auto messaging = SKSE::GetMessagingInterface();
	if (!messaging) {
		logger::error("Failed to get SKSE messaging interface");
		return;
	}

	if (!messaging->RegisterListener(OnSKSEMessage)) {
		logger::error("Failed to register SKSE messaging listener");
		return;
	}

	logger::info("SKSE messaging listener registered");
}

extern "C" __declspec(dllexport) bool SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
	SKSE::Init(a_skse);
	InitializeLog();
	RegisterSKSEMessaging();

	logger::info("{} loaded successfully", Plugin::NAME);

	SDS::PluginState::LoadConfigAndInitializeController(Plugin::INI_PATH);

	return true;
}