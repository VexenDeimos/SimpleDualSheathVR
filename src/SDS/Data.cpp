#include "PCH.h"

#include "SDS/Data.h"
#include "SDS/Util/Node.h"

namespace SDS::Data
{
	Weapon::Weapon(
		const char* a_nodeName,
		const char* a_nodeNameLeft,
		const Config::ConfigEntry& a_config) :
		m_nodeName(a_nodeName),
		m_nodeNameLeft(
			a_config.m_sheathNode.empty() ?
				a_nodeNameLeft :
				a_config.m_sheathNode.c_str()),
		m_flags(a_config.m_flags)
	{}

	const RE::BSFixedString& Weapon::GetNodeName(bool a_left) const
	{
		const bool swap = HasFlag(m_flags, Flags::kSwap);

		if (a_left) {
			return swap ? m_nodeName : m_nodeNameLeft;
		}

		return swap ? m_nodeNameLeft : m_nodeName;
	}

	RE::NiNode* Weapon::GetNode(RE::NiNode* a_root, bool a_left) const
	{
		const auto object = SDS::Util::Node::GetNiObject(a_root, GetNodeName(a_left));
		return object ? object->AsNode() : nullptr;
	}

	void WeaponData::SetStrings(
		std::uint32_t a_type,
		const char* a_nodeName,
		const char* a_nodeNameLeft)
	{
		if (a_type >= m_entries.size()) {
			return;
		}

		auto& entry = m_entries[a_type];
		if (!entry) {
			return;
		}

		if (a_nodeName) {
			entry->m_nodeName = a_nodeName;
		}

		if (a_nodeNameLeft) {
			entry->m_nodeNameLeft = a_nodeNameLeft;
		}
	}

	const Weapon* WeaponData::Get(
		RE::Actor* a_actor,
		const RE::TESObjectWEAP* a_weapon,
		bool a_left) const
	{
		if (!a_weapon) {
			return nullptr;
		}

		const auto type = a_weapon->GetWeaponType();
		const auto index = static_cast<std::size_t>(type);

		if (index >= m_entries.size()) {
			return nullptr;
		}

		const auto& entry = m_entries[index];
		if (!entry) {
			return nullptr;
		}

		const auto player = RE::PlayerCharacter::GetSingleton();

		if (a_actor && a_actor == player) {
			if (!HasFlag(entry->m_flags, Flags::kPlayer)) {
				return nullptr;
			}
		} else {
			if (!HasFlag(entry->m_flags, Flags::kNPC)) {
				return nullptr;
			}
		}

		if (!a_left && !HasFlag(entry->m_flags, Flags::kRight)) {
			return nullptr;
		}

		return entry.get();
	}

	const RE::BSFixedString* WeaponData::GetNodeName(
		const RE::TESObjectWEAP* a_weapon,
		bool a_left) const
	{
		if (!a_weapon) {
			return nullptr;
		}

		const auto type = a_weapon->GetWeaponType();
		const auto index = static_cast<std::size_t>(type);

		if (index >= m_entries.size()) {
			return nullptr;
		}

		const auto& entry = m_entries[index];
		if (!entry) {
			return nullptr;
		}

		return std::addressof(entry->GetNodeName(a_left));
	}
}