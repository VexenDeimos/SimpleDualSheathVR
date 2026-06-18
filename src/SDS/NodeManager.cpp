#include "PCH.h"

#include "SDS/NodeManager.h"

#include <algorithm>
#include <cctype>
#include <string_view>

namespace
{
	std::vector<RE::NiPointer<RE::NiNode>> g_createdNodes;

	bool StringContainsInsensitive(std::string_view a_text, std::string_view a_search)
	{
		if (a_search.empty()) {
			return true;
		}

		const auto it = std::search(
			a_text.begin(),
			a_text.end(),
			a_search.begin(),
			a_search.end(),
			[](char a_lhs, char a_rhs) {
				return std::tolower(static_cast<unsigned char>(a_lhs)) ==
				       std::tolower(static_cast<unsigned char>(a_rhs));
			});

		return it != a_text.end();
	}

	bool NodeNameMatchesDiscoveryFilter(std::string_view a_name)
	{
		constexpr std::string_view filters[] = {
			"weapon",
			"shield",
			"sword",
			"axe",
			"mace",
			"dagger",
			"staff",
			"hand",
			"wrist",
			"forearm",
			"npc l",
			"npc r",
			"vrik",
			"holster",
			"sheath",
			"back",
			"left",
			"right"
		};

		for (const auto filter : filters) {
			if (StringContainsInsensitive(a_name, filter)) {
				return true;
			}
		}

		return false;
	}
}

namespace SDS::NodeManager
{
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
			return true;
		}

		RE::NiPointer<RE::NiNode> node(RE::NiNode::Create(0));
		if (!node) {
			logger::error("EnsureChildNode failed: NiNode::Create returned null for {}", a_nodeName);
			return false;
		}

		node->name = a_nodeName;
		a_parent->AttachChild(node.get(), true);

		g_createdNodes.push_back(node);

		logger::debug("EnsureChildNode created: {}", a_nodeName);
		return true;
	}

	void EnsureFallbackSDSNodes(RE::NiNode* a_root, const char* a_rootLabel)
	{
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

	void RunPlayerNodeDiscovery(const char* a_reason)
	{
		logger::info("Beginning player node discovery: {}", a_reason);

		const auto player = RE::PlayerCharacter::GetSingleton();
		if (!player) {
			logger::warn("Player node discovery failed: PlayerCharacter::GetSingleton returned null");
			return;
		}

		const auto logRootMatches = [](RE::NiNode* a_root, const char* a_rootName) {
			if (!a_root) {
				logger::warn("Node discovery [{}]: root is null", a_rootName);
				return;
			}

			logger::info("Node discovery [{}]: root={}", a_rootName, static_cast<const void*>(a_root));

			std::uint32_t visitedCount = 0;
			std::uint32_t matchedCount = 0;

			const auto walkNode = [&](auto&& a_self, RE::NiAVObject* a_object, std::uint32_t a_depth) -> void {
				if (!a_object) {
					return;
				}

				++visitedCount;

				const auto objectName = a_object->name.c_str();
				if (objectName && objectName[0] != '\0' && NodeNameMatchesDiscoveryFilter(objectName)) {
					++matchedCount;

					logger::info("Node discovery [{}]: depth={}, object={}, node={}, name={}",
						a_rootName,
						a_depth,
						static_cast<const void*>(a_object),
						static_cast<const void*>(a_object->AsNode()),
						objectName);
				}

				const auto node = a_object->AsNode();
				if (!node) {
					return;
				}

				for (const auto& child : node->children) {
					a_self(a_self, child.get(), a_depth + 1);
				}
			};

			walkNode(walkNode, a_root, 0);

			logger::info("Node discovery [{}]: visited={}, matched={}", a_rootName, visitedCount, matchedCount);
		};

		const auto thirdPersonObject = player->Get3D(false);
		const auto firstPersonObject = player->Get3D(true);
		const auto currentObject = player->GetCurrent3D();

		auto thirdPersonRoot = thirdPersonObject ? thirdPersonObject->AsNode() : nullptr;
		auto firstPersonRoot = firstPersonObject ? firstPersonObject->AsNode() : nullptr;
		auto currentRoot = currentObject ? currentObject->AsNode() : nullptr;

		logger::info("Node discovery roots: thirdPerson={}, firstPerson={}, current={}",
			static_cast<const void*>(thirdPersonRoot),
			static_cast<const void*>(firstPersonRoot),
			static_cast<const void*>(currentRoot));

		logRootMatches(thirdPersonRoot, "third-person");
		logRootMatches(firstPersonRoot, "first-person");

		if (currentRoot && currentRoot != thirdPersonRoot && currentRoot != firstPersonRoot) {
			logRootMatches(currentRoot, "current");
		}

		logger::info("Player node discovery complete: {}", a_reason);
	}
}