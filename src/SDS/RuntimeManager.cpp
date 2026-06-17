#include "PCH.h"

#include "SDS/Controller.h"
#include "SDS/RuntimeManager.h"

namespace SDS
{
	std::atomic<std::uint32_t> RuntimeManager::s_pollingGeneration{ 0 };
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

	void RuntimeManager::StartPolling(const char* a_reason)
	{
		const auto generation = ++s_pollingGeneration;

		std::thread([reason = std::string(a_reason), generation]() {
			logger::info("Starting SDS runtime polling: {}, generation={}", reason, generation);

			while (s_pollingGeneration == generation) {
				std::this_thread::sleep_for(std::chrono::milliseconds(500));

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