#include "PCH.h"

#include "SDS/Controller.h"
#include "SDS/RuntimeManager.h"

namespace SDS
{
	std::atomic<std::uint32_t> RuntimeManager::s_pollingGeneration{ 0 };
	std::atomic_bool RuntimeManager::s_pollingEnabled{ true };
	std::atomic<std::uint32_t> RuntimeManager::s_pollingIntervalMS{ 500 };
	Controller* RuntimeManager::s_controller = nullptr;

	void RuntimeManager::SetController(Controller* a_controller)
	{
		s_controller = a_controller;
	}

	void RuntimeManager::Run(const char*)
	{
		if (!s_controller) {
			logger::warn("SDS runtime process skipped: controller is not initialized");
			return;
		}

		const auto player = RE::PlayerCharacter::GetSingleton();
		if (!player) {
			logger::warn("SDS runtime process skipped: PlayerCharacter::GetSingleton returned null");
			return;
		}

		s_controller->ProcessEquippedLeftWeapon(player);
	}
	
	void RuntimeManager::Configure(bool a_enablePolling, std::uint32_t a_intervalMS)
	{
		s_pollingEnabled = a_enablePolling;
		s_pollingIntervalMS = a_intervalMS;

		logger::info("Runtime polling configured: enabled={}, intervalMS={}",
			s_pollingEnabled.load(),
			s_pollingIntervalMS.load());
	}

	void RuntimeManager::StartPolling(const char* a_reason)
	{
		if (!s_pollingEnabled) {
			logger::info("SDS runtime polling disabled: {}", a_reason);
			return;
		}
		const auto generation = ++s_pollingGeneration;
		const auto intervalMS = s_pollingIntervalMS.load();

		std::thread([reason = std::string(a_reason), generation, intervalMS]() {
			logger::info("Starting SDS runtime polling: {}, generation={}", reason, generation);

			while (s_pollingGeneration == generation) {
				std::this_thread::sleep_for(std::chrono::milliseconds(intervalMS));

				const auto taskInterface = SKSE::GetTaskInterface();
				if (!taskInterface) {
					logger::warn("SDS runtime polling stopped: SKSE task interface unavailable");
					return;
				}

				taskInterface->AddTask([reason, generation]() {
					if (s_pollingGeneration != generation) {
						return;
					}

					RuntimeManager::Run(reason.c_str());
				});
			}

			logger::info("Stopping SDS runtime polling: {}, generation={}", reason, generation);
		}).detach();
	}

	void RuntimeManager::StopPolling()
	{
		++s_pollingGeneration;
	}
}