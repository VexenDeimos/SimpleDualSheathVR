#include "PCH.h"

namespace Plugin
{
	constexpr auto NAME = "SimpleDualSheathVR";
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

extern "C" __declspec(dllexport) bool SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
	WriteProbeLog("SKSEPlugin_Load: entered");

	SKSE::Init(a_skse);
	InitializeLog();

	logger::info("{} loaded successfully", Plugin::NAME);
	WriteProbeLog("SKSEPlugin_Load: complete");

	return true;
}