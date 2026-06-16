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
}