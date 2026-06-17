#pragma once

#include "PCH.h"

#include "SDS/Flags.h"

namespace SDS
{
	class FlagParser
	{
	public:
		static Data::Flags Parse(const std::string& a_in, bool a_internal = false);
	};

	class ConfigKeyCombo
	{
	public:
		ConfigKeyCombo() = default;

		void Parse(const std::string& a_input);

		[[nodiscard]] bool Has() const noexcept
		{
			return m_key != 0;
		}

		[[nodiscard]] std::uint32_t GetKey() const noexcept
		{
			return m_key;
		}

		[[nodiscard]] std::uint32_t GetComboKey() const noexcept
		{
			return m_comboKey;
		}

	private:
		std::uint32_t m_key{ 0 };
		std::uint32_t m_comboKey{ 0 };
	};

	struct Config
	{
		inline static constexpr auto SECT_GENERAL = "General";
		inline static constexpr auto SECT_NPC     = "NPC";
		inline static constexpr auto SECT_RUNTIME = "Runtime";
		inline static constexpr auto SECT_SWORD   = "Sword";
		inline static constexpr auto SECT_AXE     = "Axe";
		inline static constexpr auto SECT_MACE    = "Mace";
		inline static constexpr auto SECT_DAGGER  = "Dagger";
		inline static constexpr auto SECT_STAFF   = "Staff";
		inline static constexpr auto SECT_SHIELD  = "ShieldOnBack";
		inline static constexpr auto SECT_2HSWORD = "2HSword";
		inline static constexpr auto SECT_2HAXE   = "2HAxe";

		inline static constexpr auto KW_FLAGS      = "Flags";
		inline static constexpr auto KW_SHEATHNODE = "SheathNode";

		struct ConfigEntry
		{
			Data::Flags m_flags{ Data::Flags::kNone };
			std::string m_sheathNode;

			[[nodiscard]] bool IsEnabled() const noexcept
			{
				return Data::HasAnyFlag(m_flags, Data::Flags::kEnabled);
			}

			[[nodiscard]] bool IsPlayerEnabled() const noexcept
			{
				return Data::HasFlag(m_flags, Data::Flags::kPlayer);
			}

			[[nodiscard]] bool FirstPerson() const noexcept
			{
				return Data::HasFlag(m_flags, Data::Flags::kFirstPerson);
			}
		};

		Config() = default;
		explicit Config(const std::string& a_path);

		bool Load(const std::string& a_path);

		[[nodiscard]] bool IsLoaded() const noexcept
		{
			return m_loaded;
		}

		[[nodiscard]] bool HasEnabled2HEntries() const noexcept
		{
			return m_2hSword.IsEnabled() || m_2hAxe.IsEnabled();
		}

		ConfigEntry m_sword;
		ConfigEntry m_axe;
		ConfigEntry m_mace;
		ConfigEntry m_dagger;
		ConfigEntry m_staff;
		ConfigEntry m_2hSword;
		ConfigEntry m_2hAxe;
		ConfigEntry m_shield;

		bool m_disableScabbards{ false };
		bool m_npcEquipLeft{ false };
		bool m_runtimePollingEnabled{ true };
		std::uint32_t m_runtimePollingIntervalMS{ 3000 };
		bool m_runtimeLogActionEvents{ false };
		bool m_shieldHandWorkaround{ false };
		bool m_shwForceIfDrawn{ false };
		bool m_disableWeapNodeSharing{ false };

		ConfigKeyCombo m_shieldToggleKeys;

		Data::Flags m_shieldHideFlags{ Data::Flags::kNone };

	private:
		bool m_loaded{ false };
	};
}