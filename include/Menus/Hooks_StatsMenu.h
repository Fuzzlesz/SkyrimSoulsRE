#pragma once
#include "SkyrimSoulsRE.h"

namespace SkyrimSoulsRE
{
	// menuDepth = 3
	// flags = kPausesGame | kUsesMenuContext | kDisablePauseMenu | kRequiresUpdate | kCustomRendering
	// kUsesCursor if gamepad disabled
	// context = kStats
	class StatsMenuEx : public RE::StatsMenu
	{
	public:
		void AdvanceMovie_Hook(float a_interval, std::uint32_t a_currentTime);	// 05

		static RE::IMenu* Creator();
		static void InstallHook();

		using AdvanceMovie_t = decltype(&RE::StatsMenu::AdvanceMovie);
		static inline REL::Relocation<AdvanceMovie_t> _AdvanceMovie;
	};
}
