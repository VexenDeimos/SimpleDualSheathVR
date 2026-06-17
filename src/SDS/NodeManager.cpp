#include "PCH.h"

#include "SDS/NodeManager.h"

namespace
{
	std::vector<RE::NiPointer<RE::NiNode>> g_createdNodes;
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
}