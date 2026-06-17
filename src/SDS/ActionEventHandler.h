#pragma once

#include "SKSE/Events.h"

namespace SDS
{
	class ActionEventHandler :
		public RE::BSTEventSink<SKSE::ActionEvent>
	{
	public:
		static ActionEventHandler* GetSingleton();

		static void Configure(bool a_logEvents);
		static void Register();

		RE::BSEventNotifyControl ProcessEvent(
			const SKSE::ActionEvent* a_event,
			RE::BSTEventSource<SKSE::ActionEvent>* a_eventSource) override;

	private:
		ActionEventHandler() = default;

		static std::atomic_bool s_registered;
		static std::atomic_bool s_logEvents;
	};
}