#pragma once

namespace SDS
{
	class Controller;

	class RuntimeManager
	{
	public:
		static void SetController(Controller* a_controller);
		static void Configure(bool a_enablePolling, std::uint32_t a_intervalMS);

		static void Run(const char* a_reason);
		static void StartPolling(const char* a_reason);
		static void StopPolling();

	private:
		static std::atomic<std::uint32_t> s_pollingGeneration;
		static std::atomic_bool s_pollingEnabled;
		static std::atomic<std::uint32_t> s_pollingIntervalMS;
		static Controller* s_controller;
	};
}