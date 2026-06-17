#include "PCH.h"

#include "SDS/Config.h"
#include "SDS/StringHolder.h"

namespace
{
	std::string Trim(std::string a_value)
	{
		auto notSpace = [](unsigned char ch) {
			return !std::isspace(ch);
		};

		a_value.erase(a_value.begin(), std::find_if(a_value.begin(), a_value.end(), notSpace));
		a_value.erase(std::find_if(a_value.rbegin(), a_value.rend(), notSpace).base(), a_value.end());

		return a_value;
	}

	std::string ToLower(std::string a_value)
	{
		std::transform(
			a_value.begin(),
			a_value.end(),
			a_value.begin(),
			[](unsigned char ch) {
				return static_cast<char>(std::tolower(ch));
			});

		return a_value;
	}

	std::vector<std::string> Split(const std::string& a_input, char a_delimiter)
	{
		std::vector<std::string> result;
		std::stringstream stream(a_input);
		std::string item;

		while (std::getline(stream, item, a_delimiter)) {
			auto trimmed = Trim(item);
			if (!trimmed.empty()) {
				result.push_back(trimmed);
			}
		}

		return result;
	}

	bool ParseUInt32(const std::string& a_input, std::uint32_t& a_out)
	{
		try {
			const auto value = std::stoul(Trim(a_input), nullptr, 0);
			if (value > std::numeric_limits<std::uint32_t>::max()) {
				return false;
			}

			a_out = static_cast<std::uint32_t>(value);
			return true;
		} catch (...) {
			return false;
		}
	}

	class IniFile
	{
	public:
		explicit IniFile(const std::string& a_path)
		{
			Load(a_path);
		}

		[[nodiscard]] bool IsLoaded() const noexcept
		{
			return m_loaded;
		}

		[[nodiscard]] std::string GetValue(
			const std::string& a_section,
			const std::string& a_key,
			const std::string& a_default) const
		{
			const auto lookupKey = MakeLookupKey(a_section, a_key);

			if (const auto it = m_values.find(lookupKey); it != m_values.end()) {
				return it->second;
			}

			return a_default;
		}

		[[nodiscard]] bool GetBoolValue(
			const std::string& a_section,
			const std::string& a_key,
			bool a_default) const
		{
			const auto value = ToLower(Trim(GetValue(a_section, a_key, a_default ? "true" : "false")));

			if (value == "1" || value == "true" || value == "yes" || value == "on") {
				return true;
			}

			if (value == "0" || value == "false" || value == "no" || value == "off") {
				return false;
			}

			return a_default;
		}
		
		[[nodiscard]] std::uint32_t GetUInt32Value(
			const std::string& a_section,
			const std::string& a_key,
			std::uint32_t a_default) const
		{
			std::uint32_t out = a_default;

			if (!ParseUInt32(GetValue(a_section, a_key, std::to_string(a_default)), out)) {
				return a_default;
			}

			return out;
		}

	private:
		bool Load(const std::string& a_path)
		{
			std::ifstream file(a_path);
			if (!file.is_open()) {
				m_loaded = false;
				return false;
			}

			std::string section;
			std::string line;

			while (std::getline(file, line)) {
				line = Trim(line);

				if (line.empty() || line.starts_with(';') || line.starts_with('#')) {
					continue;
				}

				if (line.front() == '[' && line.back() == ']') {
					section = Trim(line.substr(1, line.size() - 2));
					continue;
				}

				const auto equals = line.find('=');
				if (equals == std::string::npos) {
					continue;
				}

				auto key = Trim(line.substr(0, equals));
				auto value = Trim(line.substr(equals + 1));

				if (key.empty()) {
					continue;
				}

				m_values[MakeLookupKey(section, key)] = value;
			}

			m_loaded = true;
			return true;
		}

		[[nodiscard]] static std::string MakeLookupKey(const std::string& a_section, const std::string& a_key)
		{
			return ToLower(Trim(a_section)) + "." + ToLower(Trim(a_key));
		}

		bool m_loaded{ false };
		std::unordered_map<std::string, std::string> m_values;
	};
}

namespace SDS
{
	using namespace Data;

	Flags FlagParser::Parse(const std::string& a_in, bool a_internal)
	{
		auto out = Flags::kNone;

		for (auto entry : Split(a_in, '|')) {
			entry = ToLower(entry);

			if (entry == "npc") {
				out |= Flags::kNPC;
			} else if (entry == "player") {
				out |= Flags::kPlayer;
			} else if (entry == "firstperson") {
				out |= Flags::kFirstPerson;
			} else if (entry == "mountonly") {
				out |= Flags::kMountOnly;
			} else if (a_internal && entry == "right") {
				out |= Flags::kRight;
			} else if (a_internal && entry == "swap") {
				out |= Flags::kSwap;
			}
		}

		return out;
	}

	void ConfigKeyCombo::Parse(const std::string& a_input)
	{
		m_comboKey = 0;
		m_key = 0;

		const auto parts = Split(a_input, '+');

		if (parts.size() > 1) {
			ParseUInt32(parts[0], m_comboKey);
			ParseUInt32(parts[1], m_key);
		} else if (parts.size() == 1) {
			ParseUInt32(parts[0], m_key);
		}
	}

	Config::Config(const std::string& a_path)
	{
		Load(a_path);
	}

	bool Config::Load(const std::string& a_path)
	{
		IniFile reader(a_path);

		m_disableScabbards = reader.GetBoolValue(SECT_GENERAL, "DisableAllScabbards", false);
		m_disableWeapNodeSharing = reader.GetBoolValue(SECT_GENERAL, "DisableWeaponNodeSharing", false);
		m_runtimePollingEnabled = reader.GetBoolValue(SECT_RUNTIME, "EnableRuntimePolling", true);
		m_runtimePollingIntervalMS = reader.GetUInt32Value(SECT_RUNTIME, "PollingIntervalMS", 3000);
		m_runtimeLogActionEvents = reader.GetBoolValue(SECT_RUNTIME, "LogActionEvents", false);

		if (m_runtimePollingIntervalMS < 100) {
			m_runtimePollingIntervalMS = 100;
		}

		if (m_runtimePollingIntervalMS > 5000) {
			m_runtimePollingIntervalMS = 5000;
		}

		m_sword = {
			FlagParser::Parse(reader.GetValue(SECT_SWORD, KW_FLAGS, "Player|NPC")),
			reader.GetValue(SECT_SWORD, KW_SHEATHNODE, StringHolder::NINODE_SWORD_LEFT)
		};

		m_axe = {
			FlagParser::Parse(reader.GetValue(SECT_AXE, KW_FLAGS, "Player|NPC")),
			reader.GetValue(SECT_AXE, KW_SHEATHNODE, StringHolder::NINODE_AXE_LEFT)
		};

		m_mace = {
			FlagParser::Parse(reader.GetValue(SECT_MACE, KW_FLAGS, "Player|NPC")),
			reader.GetValue(SECT_MACE, KW_SHEATHNODE, StringHolder::NINODE_MACE_LEFT)
		};

		m_dagger = {
			FlagParser::Parse(reader.GetValue(SECT_DAGGER, KW_FLAGS, "Player|NPC")),
			reader.GetValue(SECT_DAGGER, KW_SHEATHNODE, StringHolder::NINODE_DAGGER_LEFT)
		};

		m_2hSword = {
			FlagParser::Parse(reader.GetValue(SECT_2HSWORD, KW_FLAGS, "")),
			reader.GetValue(SECT_2HSWORD, KW_SHEATHNODE, StringHolder::NINODE_SWORD_ON_BACK_LEFT)
		};

		m_2hAxe = {
			FlagParser::Parse(reader.GetValue(SECT_2HAXE, KW_FLAGS, "")),
			reader.GetValue(SECT_2HAXE, KW_SHEATHNODE, StringHolder::NINODE_AXE_ON_BACK_LEFT)
		};

		m_staff = {
			FlagParser::Parse(reader.GetValue(SECT_STAFF, KW_FLAGS, "Player|NPC|Right"), true),
			reader.GetValue(SECT_STAFF, KW_SHEATHNODE, StringHolder::NINODE_STAFF_LEFT)
		};

		m_shield = {
			FlagParser::Parse(reader.GetValue(SECT_SHIELD, KW_FLAGS, "")),
			reader.GetValue(SECT_SHIELD, KW_SHEATHNODE, StringHolder::NINODE_SHIELD_BACK)
		};

		m_shieldHandWorkaround = reader.GetBoolValue(SECT_SHIELD, "ClenchedHandWorkaround", false);
		m_shwForceIfDrawn = reader.GetBoolValue(SECT_SHIELD, "ClenchedHandWorkaroundForceIfDrawn", false);
		m_shieldHideFlags = FlagParser::Parse(reader.GetValue(SECT_SHIELD, "DisableHideOnSit", ""));
		m_shieldToggleKeys.Parse(reader.GetValue(SECT_SHIELD, "ToggleKeys", ""));

		m_npcEquipLeft = reader.GetBoolValue(SECT_NPC, "EquipLeft", false);

		m_loaded = reader.IsLoaded();
		return m_loaded;
	}
}