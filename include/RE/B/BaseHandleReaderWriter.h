#pragma once

#include "RE/I/IHandleReaderWriter.h"

namespace RE::SkyrimScript
{
	class BaseHandleReaderWriter : public RE::BSScript::IHandleReaderWriter
	{
	public:
		inline static constexpr auto RTTI = RTTI_SkyrimScript__BaseHandleReaderWriter;
		inline static constexpr auto VTABLE = VTABLE_SkyrimScript__BaseHandleReaderWriter;

		~BaseHandleReaderWriter() override;  // 0

		// add
		virtual bool SaveRefID(SaveStorageWrapper* saveWrapper, uint32_t handleLow) = 0;                       // 4
		virtual bool ParseScriptInstance(BSMemStorage* loadWrapper, uint32_t* scriptInstanceValues) = 0;  // 5
	};
	static_assert(sizeof(BaseHandleReaderWriter) == 0x8);
}
