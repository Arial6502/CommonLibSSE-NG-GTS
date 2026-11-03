#include "RE/B/BGSDefaultObjectManager.h"

#include "SKSE/Version.h"

using namespace REL;

namespace RE
{
	namespace
	{
		constexpr auto kInvalid = (std::numeric_limits<std::size_t>::max)();

		// 1.7.99 inserts HelpManualNX at 188 and four HelpMotion objects plus HelpAmiibo at
		// 264-268, so SE and 1.6 indices from 188 move +1 and from 263 move +6.
		inline std::size_t Remap(std::size_t a_idx) noexcept
		{
			if (!Module::IsAtLeast(SKSE::RUNTIME_SSE_1_7_99)) {
				return a_idx;
			}
			return a_idx >= 263 ? a_idx + 6 : a_idx >= 188 ? a_idx + 1 : a_idx;
		}

		inline std::size_t MapIndex(std::underlying_type_t<DefaultObjectID> a_idx) noexcept
		{
			if (a_idx <= stl::to_underlying(DefaultObjectID::kKeywordActivatorFurnitureNoPlayer)) {
				return Remap(a_idx);
			}
			std::size_t result;
			if SKYRIM_REL_CONSTEXPR (Module::IsVR()) {
				result = (0xFFFF0000 & a_idx) >> 16;
			} else {
				result = 0x0000FFFF & a_idx;
			}
			return result ? Remap(result) : kInvalid;
		}
	}

	TESForm** BGSDefaultObjectManager::GetObject(DefaultObjectID a_object) noexcept
	{
		const auto idx = MapIndex(stl::to_underlying(a_object));
		if (idx == kInvalid || !IsObjectInitialized(idx)) {
			return nullptr;
		}
		return &objects[idx];
	}

	std::size_t BGSDefaultObjectManager::ObjectCount() noexcept
	{
		// Read off the initialized flags loop in InitializeAfterAllFormsRead, which starts
		// right after the form array: SE 0xB80, AE 1.6 0xB90, AE 1.7 0xBC0.
		if SKYRIM_REL_VR_CONSTEXPR (Module::IsVR()) {
			return 369;
		}
		if (Module::IsAtLeast(SKSE::RUNTIME_SSE_1_7_99)) {
			return 372;
		}
		return Module::IsAE() ? 366 : 364;
	}

	bool BGSDefaultObjectManager::IsObjectInitialized(DefaultObjectID a_object) const noexcept
	{
		const auto idx = MapIndex(stl::to_underlying(a_object));
		return idx != kInvalid && IsObjectInitialized(idx);
	}

	bool BGSDefaultObjectManager::IsObjectInitialized(std::size_t a_idx) const noexcept
	{
		const auto count = ObjectCount();
		if (a_idx >= count) {
			return false;
		}
		const auto flags = reinterpret_cast<const bool*>(reinterpret_cast<std::uintptr_t>(this) + 0x20 + count * sizeof(TESForm*));
		return flags[a_idx];
	}

	bool BGSDefaultObjectManager::SupportsVR(DefaultObjectID a_object) noexcept
	{
		auto idx = stl::to_underlying(a_object);
		return idx <= stl::to_underlying(DefaultObjectID::kKeywordActivatorFurnitureNoPlayer) || idx & 0xFFFF0000;
	}

	bool BGSDefaultObjectManager::SupportsSE(DefaultObjectID a_object) noexcept
	{
		return (stl::to_underlying(a_object) & 0x0000FFFF) || a_object != DefaultObjectID::kWerewolfSpell;
	}

	bool BGSDefaultObjectManager::SupportsCurrentRuntime(DefaultObjectID a_object) noexcept
	{
		return MapIndex(stl::to_underlying(a_object)) != kInvalid;
	}
}
