#include "PCH.h"

#include "SDS/Controller.h"
#include "SDS/NodeManager.h"

namespace SDS
{
	Controller::Controller(const Config& a_config) :
		m_config(a_config),
		m_shieldOnBackSwitch(1)
	{}

	void Controller::InitializeData()
	{
		m_strings = std::make_unique<StringHolder>();
		m_data = std::make_unique<Data::WeaponData>();

		m_data->Create(RE::WEAPON_TYPE::kOneHandSword, StringHolder::NINODE_SWORD, StringHolder::NINODE_SWORD_LEFT, m_config.m_sword);
		m_data->Create(RE::WEAPON_TYPE::kOneHandAxe, StringHolder::NINODE_AXE, StringHolder::NINODE_AXE_LEFT, m_config.m_axe);
		m_data->Create(RE::WEAPON_TYPE::kOneHandMace, StringHolder::NINODE_MACE, StringHolder::NINODE_MACE_LEFT, m_config.m_mace);
		m_data->Create(RE::WEAPON_TYPE::kOneHandDagger, StringHolder::NINODE_DAGGER, StringHolder::NINODE_DAGGER_LEFT, m_config.m_dagger);
		m_data->Create(RE::WEAPON_TYPE::kStaff, StringHolder::NINODE_STAFF, StringHolder::NINODE_STAFF_LEFT, m_config.m_staff);
		m_data->Create(RE::WEAPON_TYPE::kTwoHandSword, StringHolder::NINODE_WEAPON_BACK, StringHolder::NINODE_SWORD_ON_BACK_LEFT, m_config.m_2hSword);
		m_data->Create(RE::WEAPON_TYPE::kTwoHandAxe, StringHolder::NINODE_WEAPON_BACK, StringHolder::NINODE_AXE_ON_BACK_LEFT, m_config.m_2hAxe);

		if (!m_config.m_shield.m_sheathNode.empty()) {
			m_strings->m_shieldSheathNode = m_config.m_shield.m_sheathNode.c_str();
		}

		logger::info("Controller data initialized");
		logger::info("Shield sheath node: {}", m_strings->m_shieldSheathNode.c_str());
		logger::info("Shield on back switch default: {}", GetShieldOnBackSwitch());
	}

	void Controller::ConfigureRuntimeLogging(bool a_logWeaponMoves)
	{
		m_logWeaponMoves = a_logWeaponMoves;

		logger::info("Weapon move logging configured: enabled={}", m_logWeaponMoves);
	}

	bool Controller::GetIsDrawn(RE::Actor* a_actor, DrawnState a_state)
	{
		switch (a_state) {
		case DrawnState::Drawn:
			return true;
		case DrawnState::Sheathed:
			return false;
		default:
			return a_actor ? a_actor->IsWeaponDrawn() : false;
		}
	}

	void Controller::LogEquippedWeaponTest(RE::Actor* a_actor) const
	{
		if (!a_actor) {
			logger::warn("Equipped weapon test skipped: actor is null");
			return;
		}

		if (!m_data) {
			logger::warn("Equipped weapon test skipped: weapon data is not initialized");
			return;
		}

		logger::info("Beginning equipped weapon test");
		logger::info("Actor IsWeaponDrawn: {}", a_actor->IsWeaponDrawn());

		const auto logHand = [&](bool a_leftHand) {
			const auto handName = a_leftHand ? "left" : "right";
			const auto form = a_actor->GetEquippedObject(a_leftHand);

			if (!form) {
				logger::info("Equipped weapon test [{}]: no equipped object", handName);
				return;
			}

			const auto weapon = form->As<RE::TESObjectWEAP>();
			if (!weapon) {
				logger::info("Equipped weapon test [{}]: equipped object is not a weapon, formType={}",
					handName,
					static_cast<std::uint32_t>(form->GetFormType()));
				return;
			}

			const auto weaponType = weapon->GetWeaponType();
			const auto entry = m_data->Get(a_actor, weapon, a_leftHand);

			logger::info("Equipped weapon test [{}]: weapon={}, type={}",
				handName,
				static_cast<const void*>(weapon),
				static_cast<std::uint32_t>(weaponType));

			if (!entry) {
				logger::info("Equipped weapon test [{}]: no SDS weapon data entry matched", handName);
				return;
			}

			logger::info("Equipped weapon test [{}]: SDS leftNode={}, rightNode={}, firstPerson={}",
				handName,
				entry->GetNodeName(true).c_str(),
				entry->GetNodeName(false).c_str(),
				entry->FirstPerson());
		};

		logHand(false);
		logHand(true);

		logger::info("Equipped weapon test complete");
	}

	void Controller::LogEquippedWeaponNodePlanTest(RE::Actor* a_actor) const
	{
		if (!a_actor) {
			logger::warn("Equipped weapon node plan skipped: actor is null");
			return;
		}

		if (!m_data || !m_strings) {
			logger::warn("Equipped weapon node plan skipped: controller data is not initialized");
			return;
		}

		logger::info("Beginning equipped weapon node plan test");

		const auto thirdPersonObject = a_actor->Get3D(false);
		const auto firstPersonObject = a_actor->Get3D(true);

		auto thirdPersonRoot = thirdPersonObject ? thirdPersonObject->AsNode() : nullptr;
		auto firstPersonRoot = firstPersonObject ? firstPersonObject->AsNode() : nullptr;

		logger::info("Equipped weapon node plan: thirdPersonRoot={}, firstPersonRoot={}",
			static_cast<const void*>(thirdPersonRoot),
			static_cast<const void*>(firstPersonRoot));

		SDS::NodeManager::EnsureFallbackSDSNodes(thirdPersonRoot, "third-person node-plan");
		SDS::NodeManager::EnsureFallbackSDSNodes(firstPersonRoot, "first-person node-plan");

		const auto drawn = a_actor->IsWeaponDrawn();
		logger::info("Equipped weapon node plan: actor drawn={}", drawn);

		const auto logHandPlan = [&](bool a_leftHand) {
			const auto handName = a_leftHand ? "left" : "right";
			const auto form = a_actor->GetEquippedObject(a_leftHand);

			if (!form) {
				logger::info("Equipped weapon node plan [{}]: no equipped object", handName);
				return;
			}

			const auto weapon = form->As<RE::TESObjectWEAP>();
			if (!weapon) {
				logger::info("Equipped weapon node plan [{}]: equipped object is not a weapon", handName);
				return;
			}

			const auto entry = m_data->Get(a_actor, weapon, a_leftHand);
			if (!entry) {
				logger::info("Equipped weapon node plan [{}]: no SDS weapon data entry matched", handName);
				return;
			}

			const auto sheathedNodeName = entry->GetNodeName(a_leftHand);
			const auto drawnNodeName = a_leftHand ? m_strings->m_shield : m_strings->m_weapon;

			const auto sourceNodeName = drawn ? sheathedNodeName : drawnNodeName;
			const auto targetNodeName = drawn ? drawnNodeName : sheathedNodeName;

			logger::info("Equipped weapon node plan [{}]: weaponType={}, sheathedNode={}, drawnNode={}",
				handName,
				static_cast<std::uint32_t>(weapon->GetWeaponType()),
				sheathedNodeName.c_str(),
				drawnNodeName.c_str());

			logger::info("Equipped weapon node plan [{}]: source={}, target={}",
				handName,
				sourceNodeName.c_str(),
				targetNodeName.c_str());

			const auto logRootPlan = [&](RE::NiNode* a_root, const char* a_rootName) {
				if (!a_root) {
					logger::info("Equipped weapon node plan [{}][{}]: root is null", handName, a_rootName);
					return;
				}

				const auto sheathedObject = SDS::NodeManager::FindObject(a_root, sheathedNodeName.c_str());
				const auto drawnObject = SDS::NodeManager::FindObject(a_root, drawnNodeName.c_str());
				const auto sourceObject = SDS::NodeManager::FindObject(a_root, sourceNodeName.c_str());
				const auto targetObject = SDS::NodeManager::FindObject(a_root, targetNodeName.c_str());

				logger::info("Equipped weapon node plan [{}][{}]: sheathedNode={}, drawnNode={}, source={}, target={}",
					handName,
					a_rootName,
					sheathedObject ? "FOUND" : "missing",
					drawnObject ? "FOUND" : "missing",
					sourceObject ? "FOUND" : "missing",
					targetObject ? "FOUND" : "missing");
			};

			logRootPlan(thirdPersonRoot, "third-person");
			logRootPlan(firstPersonRoot, "first-person");
		};

		logHandPlan(false);
		logHandPlan(true);

		logger::info("Equipped weapon node plan test complete");
	}

	void Controller::MoveEquippedLeftWeaponTest(RE::Actor* a_actor) const
	{
		if (!a_actor) {
			logger::warn("Move equipped left weapon test skipped: actor is null");
			return;
		}

		if (!m_data || !m_strings) {
			logger::warn("Move equipped left weapon test skipped: controller data is not initialized");
			return;
		}

		logger::info("Beginning move equipped left weapon test");

		constexpr bool leftHand = true;

		const auto form = a_actor->GetEquippedObject(leftHand);
		if (!form) {
			logger::info("Move equipped left weapon test: no left-hand equipped object");
			return;
		}

		const auto weapon = form->As<RE::TESObjectWEAP>();
		if (!weapon) {
			logger::info("Move equipped left weapon test: left-hand object is not a weapon");
			return;
		}

		const auto entry = m_data->Get(a_actor, weapon, leftHand);
		if (!entry) {
			logger::info("Move equipped left weapon test: no SDS weapon data entry matched");
			return;
		}

		char weaponNodeBuffer[1024]{};
		weapon->GetNodeName(weaponNodeBuffer);

		if (weaponNodeBuffer[0] == '\0') {
			logger::warn("Move equipped left weapon test: weapon node name is empty");
			return;
		}

		const RE::BSFixedString weaponNodeName(weaponNodeBuffer);

		const auto drawn = a_actor->IsWeaponDrawn();

		const auto sheathedNodeName = entry->GetNodeName(leftHand);
		const auto drawnNodeName = m_strings->m_shield;

		const auto sourceNodeName = drawn ? sheathedNodeName : drawnNodeName;
		const auto targetNodeName = drawn ? drawnNodeName : sheathedNodeName;

		logger::info("Move equipped left weapon test: weaponNode={}, drawn={}, source={}, target={}",
			weaponNodeName.c_str(),
			drawn,
			sourceNodeName.c_str(),
			targetNodeName.c_str());

		const auto thirdPersonObject = a_actor->Get3D(false);
		const auto firstPersonObject = a_actor->Get3D(true);

		auto thirdPersonRoot = thirdPersonObject ? thirdPersonObject->AsNode() : nullptr;
		auto firstPersonRoot = firstPersonObject ? firstPersonObject->AsNode() : nullptr;

		const auto runRootMove = [&](RE::NiNode* a_root, const char* a_rootName) {
			if (!a_root) {
				logger::warn("Move equipped left weapon test [{}]: root is null", a_rootName);
				return;
			}

			SDS::NodeManager::EnsureFallbackSDSNodes(a_root, a_rootName);

			const auto sourceNode = SDS::NodeManager::FindNode(a_root, sourceNodeName.c_str());
			const auto targetNode = SDS::NodeManager::FindNode(a_root, targetNodeName.c_str());

			if (!sourceNode || !targetNode) {
				logger::warn("Move equipped left weapon test [{}]: sourceNode={}, targetNode={}",
					a_rootName,
					sourceNode ? "FOUND" : "missing",
					targetNode ? "FOUND" : "missing");
				return;
			}

			const auto sourceWeaponObject = sourceNode->GetObjectByName(weaponNodeName);
			const auto targetWeaponObject = targetNode->GetObjectByName(weaponNodeName);

			logger::info("Move equipped left weapon test [{}]: sourceWeaponObject={}, targetWeaponObject={}",
				a_rootName,
				sourceWeaponObject ? "FOUND" : "missing",
				targetWeaponObject ? "FOUND" : "missing");

			if (sourceWeaponObject) {
				targetNode->AttachChild(sourceWeaponObject, true);

				logger::info("Move equipped left weapon test [{}]: moved {} from {} to {}",
					a_rootName,
					weaponNodeName.c_str(),
					sourceNodeName.c_str(),
					targetNodeName.c_str());
				return;
			}

			if (targetWeaponObject) {
				logger::info("Move equipped left weapon test [{}]: weapon already exists at target {}",
					a_rootName,
					targetNodeName.c_str());
				return;
			}

			logger::info("Move equipped left weapon test [{}]: weapon object was not found under source or target",
				a_rootName);
		};

		runRootMove(thirdPersonRoot, "third-person move-test");
		runRootMove(firstPersonRoot, "first-person move-test");

		logger::info("Move equipped left weapon test complete");
	}

	void Controller::LogEquippedWeaponPlan(RE::Actor* a_actor, bool a_leftHand) const
	{
		if (!a_actor) {
			logger::warn("Equipped weapon plan skipped: actor is null");
			return;
		}

		if (!m_data || !m_strings) {
			logger::warn("Equipped weapon plan skipped: controller data is not initialized");
			return;
		}

		const auto handName = a_leftHand ? "left" : "right";

		const auto form = a_actor->GetEquippedObject(a_leftHand);
		if (!form) {
			logger::info("Equipped weapon plan [{}]: no equipped object", handName);
			return;
		}

		const auto weapon = form->As<RE::TESObjectWEAP>();
		if (!weapon) {
			logger::info("Equipped weapon plan [{}]: equipped object is not a weapon", handName);
			return;
		}

		const auto entry = m_data->Get(a_actor, weapon, a_leftHand);
		if (!entry) {
			logger::info("Equipped weapon plan [{}]: no SDS weapon data entry matched", handName);
			return;
		}

		char weaponNodeBuffer[1024]{};
		weapon->GetNodeName(weaponNodeBuffer);

		if (weaponNodeBuffer[0] == '\0') {
			logger::warn("Equipped weapon plan [{}]: weapon node name is empty", handName);
			return;
		}

		const RE::BSFixedString weaponNodeName(weaponNodeBuffer);

		const auto drawn = a_actor->IsWeaponDrawn();

		const auto sheathedNodeName = entry->GetNodeName(a_leftHand);
		const auto drawnNodeName = a_leftHand ? m_strings->m_shield : m_strings->m_weapon;

		const auto sourceNodeName = drawn ? sheathedNodeName : drawnNodeName;
		const auto targetNodeName = drawn ? drawnNodeName : sheathedNodeName;

		logger::info("Equipped weapon plan [{}]: weaponNode={}, weaponType={}, drawn={}, source={}, target={}",
			handName,
			weaponNodeName.c_str(),
			static_cast<std::uint32_t>(weapon->GetWeaponType()),
			drawn,
			sourceNodeName.c_str(),
			targetNodeName.c_str());

		const auto thirdPersonObject = a_actor->Get3D(false);
		auto thirdPersonRoot = thirdPersonObject ? thirdPersonObject->AsNode() : nullptr;

		if (!thirdPersonRoot) {
			logger::warn("Equipped weapon plan [{}]: third-person root is null", handName);
			return;
		}

		SDS::NodeManager::EnsureFallbackSDSNodes(thirdPersonRoot, "third-person plan");

		const auto sourceNode = SDS::NodeManager::FindNode(thirdPersonRoot, sourceNodeName.c_str());
		const auto targetNode = SDS::NodeManager::FindNode(thirdPersonRoot, targetNodeName.c_str());

		if (!sourceNode || !targetNode) {
			logger::warn("Equipped weapon plan [{}]: sourceNode={}, targetNode={}",
				handName,
				sourceNode ? "FOUND" : "missing",
				targetNode ? "FOUND" : "missing");
			return;
		}

		const auto sourceWeaponObject = sourceNode->GetObjectByName(weaponNodeName);
		const auto targetWeaponObject = targetNode->GetObjectByName(weaponNodeName);

		logger::info("Equipped weapon plan [{}]: sourceWeaponObject={}, targetWeaponObject={}",
			handName,
			sourceWeaponObject ? "FOUND" : "missing",
			targetWeaponObject ? "FOUND" : "missing");
	}

	void Controller::ProcessEquippedWeapon(RE::Actor* a_actor, bool a_leftHand, DrawnState a_state) const
	{
		if (!a_actor) {
			logger::warn("Process equipped weapon skipped: actor is null");
			return;
		}

		if (!m_data || !m_strings) {
			logger::warn("Process equipped weapon skipped: controller data is not initialized");
			return;
		}

		const auto form = a_actor->GetEquippedObject(a_leftHand);
		if (!form) {
			return;
		}

		const auto weapon = form->As<RE::TESObjectWEAP>();
		if (!weapon) {
			return;
		}

		const auto entry = m_data->Get(a_actor, weapon, a_leftHand);
		if (!entry) {
			return;
		}

		char weaponNodeBuffer[1024]{};
		weapon->GetNodeName(weaponNodeBuffer);

		if (weaponNodeBuffer[0] == '\0') {
			logger::warn("Process equipped weapon: weapon node name is empty");
			return;
		}

		const RE::BSFixedString weaponNodeName(weaponNodeBuffer);

		const auto drawn = GetIsDrawn(a_actor, a_state);

		const auto sheathedNodeName = entry->GetNodeName(a_leftHand);
		const auto drawnNodeName = a_leftHand ? m_strings->m_shield : m_strings->m_weapon;

		const auto sourceNodeName = drawn ? sheathedNodeName : drawnNodeName;
		const auto targetNodeName = drawn ? drawnNodeName : sheathedNodeName;

		const auto processRoot = [&](RE::NiNode* a_root, const char* a_rootName) {
			if (!a_root) {
				logger::warn("Process equipped weapon [{}]: root is null", a_rootName);
				return;
			}

			SDS::NodeManager::EnsureFallbackSDSNodes(a_root, a_rootName);

			const auto sourceNode = SDS::NodeManager::FindNode(a_root, sourceNodeName.c_str());
			const auto targetNode = SDS::NodeManager::FindNode(a_root, targetNodeName.c_str());

			if (!sourceNode || !targetNode) {
				logger::warn("Process equipped weapon [{}]: sourceNode={}, targetNode={}",
					a_rootName,
					sourceNode ? "FOUND" : "missing",
					targetNode ? "FOUND" : "missing");
				return;
			}

			const auto sourceWeaponObject = sourceNode->GetObjectByName(weaponNodeName);
			const auto targetWeaponObject = targetNode->GetObjectByName(weaponNodeName);

			if (sourceWeaponObject) {
				targetNode->AttachChild(sourceWeaponObject, true);

				if (m_logWeaponMoves) {
					logger::info("Process equipped weapon [{}][{}]: moved {} from {} to {}",
						a_rootName,
						a_leftHand ? "left" : "right",
						weaponNodeName.c_str(),
						sourceNodeName.c_str(),
						targetNodeName.c_str());
				}

				return;
			}

			if (targetWeaponObject) {
				return;
			}
		};

		const auto thirdPersonObject = a_actor->Get3D(false);
		const auto firstPersonObject = a_actor->Get3D(true);
		const auto currentObject = a_actor->GetCurrent3D();

		auto thirdPersonRoot = thirdPersonObject ? thirdPersonObject->AsNode() : nullptr;
		auto firstPersonRoot = firstPersonObject ? firstPersonObject->AsNode() : nullptr;
		auto currentRoot = currentObject ? currentObject->AsNode() : nullptr;

		processRoot(thirdPersonRoot, "third-person");

		if (firstPersonRoot && firstPersonRoot != thirdPersonRoot) {
			processRoot(firstPersonRoot, "first-person");
		}

		if (currentRoot && currentRoot != thirdPersonRoot && currentRoot != firstPersonRoot) {
			processRoot(currentRoot, "current");
		}
	}

	void Controller::ProcessEquippedLeftWeapon(RE::Actor* a_actor, DrawnState a_state) const
	{
		ProcessEquippedWeapon(a_actor, true, a_state);
	}

	void Controller::ProcessPlayerWeapons(RE::Actor* a_actor, DrawnState a_state) const
	{
		ProcessEquippedLeftWeapon(a_actor, a_state);
	}
}