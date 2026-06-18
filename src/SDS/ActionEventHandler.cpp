#include "PCH.h"

#include "SDS/ActionEventHandler.h"
#include "SDS/RuntimeManager.h"

namespace
{
	const char* ActionTypeToString(SKSE::ActionEvent::Type a_type)
	{
		switch (a_type) {
		case SKSE::ActionEvent::Type::kWeaponSwing:
			return "kWeaponSwing";
		case SKSE::ActionEvent::Type::kSpellCast:
			return "kSpellCast";
		case SKSE::ActionEvent::Type::kSpellFire:
			return "kSpellFire";
		case SKSE::ActionEvent::Type::kVoiceCast:
			return "kVoiceCast";
		case SKSE::ActionEvent::Type::kVoiceFire:
			return "kVoiceFire";
		case SKSE::ActionEvent::Type::kBowDraw:
			return "kBowDraw";
		case SKSE::ActionEvent::Type::kBowRelease:
			return "kBowRelease";
		case SKSE::ActionEvent::Type::kBeginDraw:
			return "kBeginDraw";
		case SKSE::ActionEvent::Type::kEndDraw:
			return "kEndDraw";
		case SKSE::ActionEvent::Type::kBeginSheathe:
			return "kBeginSheathe";
		case SKSE::ActionEvent::Type::kEndSheathe:
			return "kEndSheathe";
		default:
			return "unknown";
		}
	}

	const char* ActionSlotToString(SKSE::ActionEvent::Slot a_slot)
	{
		switch (a_slot) {
		case SKSE::ActionEvent::Slot::kLeft:
			return "left";
		case SKSE::ActionEvent::Slot::kRight:
			return "right";
		case SKSE::ActionEvent::Slot::kVoice:
			return "voice";
		default:
			return "unknown";
		}
	}

	bool ShouldLogActionEvent(SKSE::ActionEvent::Type a_type)
	{
		switch (a_type) {
		case SKSE::ActionEvent::Type::kBeginDraw:
		case SKSE::ActionEvent::Type::kEndDraw:
		case SKSE::ActionEvent::Type::kBeginSheathe:
		case SKSE::ActionEvent::Type::kEndSheathe:
			return true;
		default:
			return false;
		}
	}

	bool ShouldQueueRuntimeProcess(SKSE::ActionEvent::Type a_type)
	{
		switch (a_type) {
		case SKSE::ActionEvent::Type::kBeginDraw:
		case SKSE::ActionEvent::Type::kEndSheathe:
			return true;
		default:
			return false;
		}
	}

	const char* GetRuntimeReason(SKSE::ActionEvent::Type a_type)
	{
		switch (a_type) {
		case SKSE::ActionEvent::Type::kBeginDraw:
			return "ActionEvent kBeginDraw";
		case SKSE::ActionEvent::Type::kEndSheathe:
			return "ActionEvent kEndSheathe";
		default:
			return "ActionEvent";
		}
	}

	void QueueRuntimeProcess(SKSE::ActionEvent::Type a_type)
	{
		const auto taskInterface = SKSE::GetTaskInterface();
		if (!taskInterface) {
			logger::warn("ActionEvent runtime process skipped: SKSE task interface unavailable");
			return;
		}

		switch (a_type) {
		case SKSE::ActionEvent::Type::kBeginDraw:
			taskInterface->AddTask([reason = std::string(GetRuntimeReason(a_type))]() {
				SDS::RuntimeManager::RunDrawn(reason.c_str());
			});
			break;
		case SKSE::ActionEvent::Type::kEndSheathe:
			taskInterface->AddTask([reason = std::string(GetRuntimeReason(a_type))]() {
				SDS::RuntimeManager::RunSheathed(reason.c_str());
			});
			break;
		default:
			break;
		}
	}
}

namespace SDS
{
	std::atomic_bool ActionEventHandler::s_registered{ false };
	std::atomic_bool ActionEventHandler::s_logEvents{ false };

	ActionEventHandler* ActionEventHandler::GetSingleton()
	{
		static ActionEventHandler singleton;
		return std::addressof(singleton);
	}

	void ActionEventHandler::Configure(bool a_logEvents)
	{
		s_logEvents = a_logEvents;

		logger::info("ActionEvent logging configured: enabled={}", s_logEvents.load());
	}

	void ActionEventHandler::Register()
	{
		if (s_registered.exchange(true)) {
			return;
		}

		const auto messaging = SKSE::GetMessagingInterface();
		if (!messaging) {
			logger::error("Failed to register ActionEvent sink: SKSE messaging interface unavailable");
			s_registered = false;
			return;
		}

		const auto dispatcher = static_cast<RE::BSTEventSource<SKSE::ActionEvent>*>(
			messaging->GetEventDispatcher(SKSE::MessagingInterface::Dispatcher::kActionEvent));

		if (!dispatcher) {
			logger::error("Failed to register ActionEvent sink: dispatcher unavailable");
			s_registered = false;
			return;
		}

		dispatcher->AddEventSink(GetSingleton());

		logger::info("SKSE ActionEvent sink registered");
	}

	RE::BSEventNotifyControl ActionEventHandler::ProcessEvent(
		const SKSE::ActionEvent* a_event,
		RE::BSTEventSource<SKSE::ActionEvent>*)
	{
		if (!a_event) {
			return RE::BSEventNotifyControl::kContinue;
		}

		const auto type = a_event->type.get();

		const auto shouldQueueRuntimeProcess = ShouldQueueRuntimeProcess(type);
		const auto shouldLogEvent = s_logEvents.load() && ShouldLogActionEvent(type);

		if (!shouldQueueRuntimeProcess && !shouldLogEvent) {
			return RE::BSEventNotifyControl::kContinue;
		}

		const auto player = RE::PlayerCharacter::GetSingleton();
		if (!player || a_event->actor != player) {
			return RE::BSEventNotifyControl::kContinue;
		}

		const auto slot = a_event->slot.get();

		if (shouldLogEvent) {
			logger::info("ActionEvent: type={}, slot={}, sourceForm={}",
				ActionTypeToString(type),
				ActionSlotToString(slot),
				static_cast<const void*>(a_event->sourceForm));
		}

		if (shouldQueueRuntimeProcess) {
			QueueRuntimeProcess(type);
		}

		return RE::BSEventNotifyControl::kContinue;
	}
}