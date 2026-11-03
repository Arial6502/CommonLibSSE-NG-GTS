#include "RE/C/ControlMap.h"

#include "RE/U/UserEventEnabled.h"

namespace RE
{
	ControlMap* ControlMap::GetSingleton()
	{
		REL::Relocation<ControlMap**> singleton{ Offset::ControlMap::Singleton };
		return *singleton;
	}

	std::int8_t ControlMap::AllowTextInput(bool a_allow)
	{
		auto& count = GetRuntimeData().textEntryCount;
		if (a_allow) {
			if (count != -1) {
				++count;
			}
		} else {
			if (count != 0) {
				--count;
			}
		}

		return count;
	}

	std::uint32_t ControlMap::GetMappedKey(std::string_view a_eventID, INPUT_DEVICE a_device, InputContextID a_context) const
	{
		assert(a_device < INPUT_DEVICE::kTotal);
		assert(a_context < InputContextID::kTotal);

		if (controlMap[a_context]) {
			const auto&   mappings = controlMap[a_context]->deviceMappings[a_device];
			BSFixedString eventID(a_eventID);
			for (auto& mapping : mappings) {
				if (mapping.eventID == eventID) {
					return mapping.inputKey;
				}
			}
		}

		return kInvalid;
	}

	std::string_view ControlMap::GetUserEventName(std::uint32_t a_buttonID, INPUT_DEVICE a_device, InputContextID a_context) const
	{
		assert(a_device < INPUT_DEVICE::kTotal);
		assert(a_context < InputContextID::kTotal);

		if (controlMap[a_context]) {
			const auto&      mappings = controlMap[a_context]->deviceMappings[a_device];
			UserEventMapping tmp{};
			tmp.inputKey = static_cast<std::uint16_t>(a_buttonID);
			auto range = std::equal_range(
				mappings.begin(),
				mappings.end(),
				tmp,
				[](auto&& a_lhs, auto&& a_rhs) {
					return a_lhs.inputKey < a_rhs.inputKey;
				});

			if (std::distance(range.first, range.second) == 1) {
				return range.first->eventID;
			}
		}

		return ""sv;
	}

	void ControlMap::ToggleControls(UEFlag a_flags, bool a_enable)
	{
		auto& data = GetRuntimeData();
		auto  oldState = data.enabledControls;

		if (a_enable) {
			data.enabledControls.set(a_flags);
			if (data.unk11C != UEFlag::kInvalid) {
				data.unk11C.set(a_flags);
			}
		} else {
			data.enabledControls.reset(a_flags);
			if (data.unk11C != UEFlag::kInvalid) {
				data.unk11C.reset(a_flags);
			}
		}

		UserEventEnabled event{ data.enabledControls, oldState };
		SendEvent(std::addressof(event));
	}
}
