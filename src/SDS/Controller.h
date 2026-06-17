#pragma once

#include "PCH.h"

#include "SDS/Config.h"
#include "SDS/Data.h"
#include "SDS/StringHolder.h"

namespace SDS
{
	class Controller
	{
	public:
		enum class DrawnState : std::uint8_t
		{
			Determine,
			Sheathed,
			Drawn
		};

		explicit Controller(const Config& a_config);

		Controller(const Controller&) = delete;
		Controller(Controller&&) = delete;
		Controller& operator=(const Controller&) = delete;
		Controller& operator=(Controller&&) = delete;

		void InitializeData();

		[[nodiscard]] const Config& GetConfig() const noexcept
		{
			return m_config;
		}

		[[nodiscard]] const StringHolder* GetStringHolder() const noexcept
		{
			return m_strings.get();
		}

		[[nodiscard]] const Data::WeaponData* GetWeaponData() const noexcept
		{
			return m_data.get();
		}

		[[nodiscard]] bool GetShieldOnBackSwitch() const noexcept
		{
			return m_shieldOnBackSwitch.load(std::memory_order_acquire) != 0;
		}

		[[nodiscard]] static bool GetIsDrawn(RE::Actor* a_actor, DrawnState a_state);

		void LogEquippedWeaponTest(RE::Actor* a_actor) const;

		void LogEquippedWeaponNodePlanTest(RE::Actor* a_actor) const;
		
		void MoveEquippedLeftWeaponTest(RE::Actor* a_actor) const;
		
		void ProcessEquippedWeapon(RE::Actor* a_actor, bool a_leftHand) const;
		void ProcessEquippedLeftWeapon(RE::Actor* a_actor) const;
		void ProcessPlayerWeapons(RE::Actor* a_actor) const;

	private:
		const Config m_config;

		std::unique_ptr<StringHolder> m_strings;
		std::unique_ptr<Data::WeaponData> m_data;

		std::atomic<std::uint8_t> m_shieldOnBackSwitch{ 1 };
	};
}