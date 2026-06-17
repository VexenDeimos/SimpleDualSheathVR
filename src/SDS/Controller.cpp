#include "PCH.h"

#include "SDS/Controller.h"

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

		logHand(false); // right hand
		logHand(true);  // left hand

		logger::info("Equipped weapon test complete");
	}
}