#pragma once

#include "PCH.h"

namespace SDS::Data
{
	enum class Flags : std::uint32_t
	{
		kNone = 0,

		kNPC         = 1u << 0,
		kPlayer      = 1u << 1,
		kRight       = 1u << 2,
		kSwap        = 1u << 3,
		kFirstPerson = 1u << 4,
		kMountOnly   = 1u << 7,

		kEnabled = kPlayer | kNPC
	};

	[[nodiscard]] constexpr Flags operator|(Flags a_lhs, Flags a_rhs) noexcept
	{
		return static_cast<Flags>(
			static_cast<std::uint32_t>(a_lhs) |
			static_cast<std::uint32_t>(a_rhs));
	}

	[[nodiscard]] constexpr Flags operator&(Flags a_lhs, Flags a_rhs) noexcept
	{
		return static_cast<Flags>(
			static_cast<std::uint32_t>(a_lhs) &
			static_cast<std::uint32_t>(a_rhs));
	}

	constexpr Flags& operator|=(Flags& a_lhs, Flags a_rhs) noexcept
	{
		a_lhs = a_lhs | a_rhs;
		return a_lhs;
	}

	[[nodiscard]] constexpr bool HasFlag(Flags a_value, Flags a_flag) noexcept
	{
		return static_cast<std::uint32_t>(a_value & a_flag) != 0;
	}

	[[nodiscard]] constexpr bool HasAnyFlag(Flags a_value, Flags a_flags) noexcept
	{
		return static_cast<std::uint32_t>(a_value & a_flags) != 0;
	}
}