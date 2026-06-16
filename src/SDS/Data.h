#pragma once

#include "PCH.h"

#include "SDS/Config.h"
#include "SDS/Flags.h"

namespace SDS::Data
{
	class Weapon
	{
	public:
		Weapon(
			const char* a_nodeName,
			const char* a_nodeNameLeft,
			const SDS::Config::ConfigEntry& a_config);

		[[nodiscard]] const RE::BSFixedString& GetNodeName(bool a_left) const;
		[[nodiscard]] RE::NiNode* GetNode(RE::NiNode* a_root, bool a_left) const;

		[[nodiscard]] bool FirstPerson() const noexcept
		{
			return HasFlag(m_flags, Flags::kFirstPerson);
		}

		RE::BSFixedString m_nodeName;
		RE::BSFixedString m_nodeNameLeft;
		Flags m_flags{ Flags::kNone };
	};

	class WeaponData
	{
	public:
		WeaponData() = default;

		template <class... Args>
		void Create(RE::WEAPON_TYPE a_type, Args&&... a_args)
		{
			const auto index = static_cast<std::size_t>(a_type);

			if (index < m_entries.size()) {
				m_entries[index] = std::make_unique<Weapon>(std::forward<Args>(a_args)...);
			}
		}

		void SetStrings(std::uint32_t a_type, const char* a_nodeName, const char* a_nodeNameLeft);

		[[nodiscard]] const Weapon* Get(
			RE::Actor* a_actor,
			const RE::TESObjectWEAP* a_weapon,
			bool a_left) const;

		[[nodiscard]] const RE::BSFixedString* GetNodeName(
			const RE::TESObjectWEAP* a_weapon,
			bool a_left) const;

	private:
		std::array<std::unique_ptr<Weapon>, static_cast<std::size_t>(RE::WEAPON_TYPE::kTotal)> m_entries{};
	};
}