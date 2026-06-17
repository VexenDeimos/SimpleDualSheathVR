#include "PCH.h"

#include "SDS/Config.h"
#include "SDS/Controller.h"
#include "SDS/Data.h"
#include "SDS/StringHolder.h"

void RunPlayerNodeProbe(const char* a_reason);
void StartDelayedPlayerNodeProbe(const char* a_reason);

namespace ProbeState
{
	std::vector<RE::NiPointer<RE::NiNode>> g_createdNodes;
}

void StartDelayedPlayerNodeProbe(const char* a_reason)
{
	std::thread([reason = std::string(a_reason)]() {
		logger::info("Starting delayed player node probe timer: {}", reason);

		std::this_thread::sleep_for(std::chrono::seconds(10));

		const auto taskInterface = SKSE::GetTaskInterface();
		if (!taskInterface) {
			logger::warn("Delayed player node probe failed: SKSE task interface unavailable");
			return;
		}

		taskInterface->AddTask([reason]() {
			logger::info("Running delayed player node probe on SKSE task: {}", reason);
			RunPlayerNodeProbe(reason.c_str());
		});
	}).detach();
}

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

void LogWeaponDataEntry(const char* a_name, const SDS::Data::Weapon& a_weapon)
{
	logger::info("{} weapon data: leftNode={}, rightNode={}, firstPerson={}",
		a_name,
		a_weapon.GetNodeName(true).c_str(),
		a_weapon.GetNodeName(false).c_str(),
		a_weapon.FirstPerson());
}

void LoadAndLogWeaponDataTest(const SDS::Config& a_config)
{
	logger::info("Beginning weapon data sanity test");

	const SDS::Data::Weapon sword(
		SDS::StringHolder::NINODE_SWORD,
		SDS::StringHolder::NINODE_SWORD_LEFT,
		a_config.m_sword);

	const SDS::Data::Weapon axe(
		SDS::StringHolder::NINODE_AXE,
		SDS::StringHolder::NINODE_AXE_LEFT,
		a_config.m_axe);

	const SDS::Data::Weapon mace(
		SDS::StringHolder::NINODE_MACE,
		SDS::StringHolder::NINODE_MACE_LEFT,
		a_config.m_mace);

	const SDS::Data::Weapon dagger(
		SDS::StringHolder::NINODE_DAGGER,
		SDS::StringHolder::NINODE_DAGGER_LEFT,
		a_config.m_dagger);

	const SDS::Data::Weapon staff(
		SDS::StringHolder::NINODE_STAFF,
		SDS::StringHolder::NINODE_STAFF_LEFT,
		a_config.m_staff);

	const SDS::Data::Weapon twoHandSword(
		SDS::StringHolder::NINODE_WEAPON_BACK,
		SDS::StringHolder::NINODE_SWORD_ON_BACK_LEFT,
		a_config.m_2hSword);

	const SDS::Data::Weapon twoHandAxe(
		SDS::StringHolder::NINODE_WEAPON_BACK,
		SDS::StringHolder::NINODE_AXE_ON_BACK_LEFT,
		a_config.m_2hAxe);

	LogWeaponDataEntry("Sword", sword);
	LogWeaponDataEntry("Axe", axe);
	LogWeaponDataEntry("Mace", mace);
	LogWeaponDataEntry("Dagger", dagger);
	LogWeaponDataEntry("Staff", staff);
	LogWeaponDataEntry("2HSword", twoHandSword);
	LogWeaponDataEntry("2HAxe", twoHandAxe);

	logger::info("Weapon data sanity test complete");
}

void LoadAndLogControllerTest(const SDS::Config& a_config)
{
	logger::info("Beginning controller initialization sanity test");

	SDS::Controller controller(a_config);
	controller.InitializeData();

	logger::info("Controller initialized: strings={}, weaponData={}, shieldSwitch={}",
		controller.GetStringHolder() != nullptr,
		controller.GetWeaponData() != nullptr,
		controller.GetShieldOnBackSwitch());

	logger::info("Controller initialization sanity test complete");
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

	LoadAndLogWeaponDataTest(config);
	LoadAndLogControllerTest(config);
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
		break;
	case SKSE::MessagingInterface::kPostLoadGame:
		logger::info("SKSE message: kPostLoadGame");
		RunPlayerNodeProbe("kPostLoadGame");
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
		RunPlayerNodeProbe("kNewGame");
		break;
	case SKSE::MessagingInterface::kDataLoaded:
		logger::info("SKSE message: kDataLoaded");
		StartDelayedPlayerNodeProbe("kDataLoaded delayed");
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

RE::NiAVObject* FindObject(RE::NiNode* a_root, const char* a_nodeName)
{
	if (!a_root) {
		return nullptr;
	}

	const RE::BSFixedString nodeName(a_nodeName);
	return a_root->GetObjectByName(nodeName);
}

RE::NiNode* FindNode(RE::NiNode* a_root, const char* a_nodeName)
{
	const auto object = FindObject(a_root, a_nodeName);
	return object ? object->AsNode() : nullptr;
}

void ProbeNode(RE::NiNode* a_root, const char* a_rootName, const char* a_nodeName)
{
	if (!a_root) {
		logger::info("Node probe [{}]: root is null, skipped {}", a_rootName, a_nodeName);
		return;
	}

	const auto object = FindObject(a_root, a_nodeName);

	logger::info("Node probe [{}]: {} -> {}",
		a_rootName,
		a_nodeName,
		object ? "FOUND" : "missing");
}

bool EnsureChildNode(RE::NiNode* a_parent, const char* a_nodeName)
{
	if (!a_parent) {
		logger::warn("EnsureChildNode failed: parent is null for {}", a_nodeName);
		return false;
	}

	if (FindObject(a_parent, a_nodeName)) {
		logger::info("EnsureChildNode skipped: {} already exists", a_nodeName);
		return true;
	}

	RE::NiPointer<RE::NiNode> node(RE::NiNode::Create(0));
	if (!node) {
		logger::error("EnsureChildNode failed: NiNode::Create returned null for {}", a_nodeName);
		return false;
	}

	node->name = a_nodeName;
	a_parent->AttachChild(node.get(), true);

	ProbeState::g_createdNodes.push_back(node);

	logger::info("EnsureChildNode created: {}", a_nodeName);
	return true;
}

void EnsureFallbackSDSNodes(RE::NiNode* a_root, const char* a_rootLabel)
{
	logger::info("Beginning fallback SDS node creation test for {}", a_rootLabel);

	if (!a_root) {
		logger::warn("Fallback SDS node creation skipped: {} root is null", a_rootLabel);
		return;
	}

	auto attachRoot = FindNode(a_root, "NPC Root [Root]");
	if (!attachRoot) {
		logger::warn("Fallback SDS node creation: NPC Root [Root] missing for {}, using passed root", a_rootLabel);
		attachRoot = a_root;
	}

	constexpr const char* nodesToCreate[] = {
		"WeaponSwordLeft",
		"WeaponAxeLeft",
		"WeaponMaceLeft",
		"WeaponDaggerLeft",
		"WeaponStaffLeft",
		"ShieldBack"
	};

	for (const auto* nodeName : nodesToCreate) {
		EnsureChildNode(attachRoot, nodeName);
	}

	logger::info("Fallback SDS node creation test complete for {}", a_rootLabel);
}

void RunPlayerNodeProbe(const char* a_reason)
{
	logger::info("Beginning player node probe: {}", a_reason);

	const auto player = RE::PlayerCharacter::GetSingleton();
	if (!player) {
		logger::warn("Player node probe failed: PlayerCharacter::GetSingleton returned null");
		return;
	}

	logger::info("Player Is3DLoaded: {}", player->Is3DLoaded());

	const auto thirdPersonObject = player->Get3D(false);
	const auto firstPersonObject = player->Get3D(true);
	const auto currentObject = player->GetCurrent3D();

	logger::info("Player Get3D(false): {}", static_cast<const void*>(thirdPersonObject));
	logger::info("Player Get3D(true): {}", static_cast<const void*>(firstPersonObject));
	logger::info("Player GetCurrent3D(): {}", static_cast<const void*>(currentObject));

	auto thirdPersonRoot = thirdPersonObject ? thirdPersonObject->AsNode() : nullptr;
	auto firstPersonRoot = firstPersonObject ? firstPersonObject->AsNode() : nullptr;
	auto currentRoot = currentObject ? currentObject->AsNode() : nullptr;

	logger::info("Player third-person root node: {}", static_cast<const void*>(thirdPersonRoot));
	logger::info("Player first-person root node: {}", static_cast<const void*>(firstPersonRoot));
	logger::info("Player current root node: {}", static_cast<const void*>(currentRoot));

	constexpr const char* nodesToCheck[] = {
		"NPC Root [Root]",
		"WeaponSword",
		"WeaponSwordLeft",
		"WeaponAxe",
		"WeaponAxeLeft",
		"WeaponMace",
		"WeaponMaceLeft",
		"WeaponDagger",
		"WeaponDaggerLeft",
		"WeaponStaff",
		"WeaponStaffLeft",
		"WEAPON",
		"SHIELD",
		"ShieldBack"
	};

	for (const auto* nodeName : nodesToCheck) {
		ProbeNode(thirdPersonRoot, "third-person", nodeName);
	}

	for (const auto* nodeName : nodesToCheck) {
		ProbeNode(firstPersonRoot, "first-person", nodeName);
	}

	EnsureFallbackSDSNodes(thirdPersonRoot, "third-person");
	EnsureFallbackSDSNodes(firstPersonRoot, "first-person");

	logger::info("Re-running player node probe after fallback node creation");

	for (const auto* nodeName : nodesToCheck) {
		ProbeNode(thirdPersonRoot, "third-person after-create", nodeName);
	}

	for (const auto* nodeName : nodesToCheck) {
		ProbeNode(firstPersonRoot, "first-person after-create", nodeName);
	}

	logger::info("Player node probe complete: {}", a_reason);
}

extern "C" __declspec(dllexport) bool SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
	WriteProbeLog("SKSEPlugin_Load: entered");

	SKSE::Init(a_skse);
	InitializeLog();
	RegisterSKSEMessaging();

	logger::info("{} loaded successfully", Plugin::NAME);

	LoadAndLogConfig();

	WriteProbeLog("SKSEPlugin_Load: complete");

	return true;
}