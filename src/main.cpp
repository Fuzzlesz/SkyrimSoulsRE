#include "EngineFixesChecker.h"
#undef MessageBox

#include "SkyrimSoulsRE.h"

constexpr auto MESSAGEBOX_ERROR = 0x00001010L;    // MB_OK | MB_ICONERROR | MB_SYSTEMMODAL
constexpr auto MESSAGEBOX_WARNING = 0x00001030L;  // MB_OK | MB_ICONWARNING | MB_SYSTEMMODAL
void MessageHandler(SKSE::MessagingInterface::Message* a_msg)
{
	switch (a_msg->type) {
	case SKSE::MessagingInterface::kPostLoad:
		{
			SkyrimSoulsRE::Settings* settings = SkyrimSoulsRE::Settings::GetSingleton();

			bool engineFixesPresent = false;
			if (REX::W32::GetModuleHandle("EngineFixes.dll")) {
				if (SkyrimSoulsRE::EngineFixesConfig::load_config("Data/SKSE/Plugins/EngineFixes.toml")) {
					if (SkyrimSoulsRE::EngineFixesConfig::patchMemoryManager && SkyrimSoulsRE::EngineFixesConfig::fixGlobalTime) {
						SKSE::log::info("SSE Engine Fixes detected.");
						engineFixesPresent = true;
					}
				}
			}

			if (!engineFixesPresent) {
				SKSE::log::warn("SSE Engine Fixes not detected, or certain features are not enabled.");
				SKSE::log::warn("To ensure best functionality, the following Engine Fixes features must be enabled : Memory Manager patch, Global Time Fix");
				if (!settings->hideEngineFixesWarning) {
					REX::W32::MessageBoxA(nullptr, "SSE Engine Fixes not detected, or certain features are not enabled. This will not prevent Skyrim Souls RE from running, but to ensure best functionality, the following Engine Fixes features must be enabled:\n\n- Memory Manager patch\n- Global Time Fix\n\nThe Memory Manager patch prevents the false save corruption bug that tends to happen with this mod, and the Global Time fix fixes the behaviour of some menus when using the slow-motion feature.\n\nIf you can't install SSE Engine Fixes for some reason (you're using Skyrim Together for example), you can disable this warning in the .ini.", "Skyrim Souls RE - Warning", MESSAGEBOX_WARNING);
				}
			}

			if (REX::W32::GetModuleHandle("DialogueMovementEnabler.dll")) {
				SKSE::log::info("Dialogue Movement Enabler detected. Enabling compatibility.");
				settings->isUsingDME = true;
			} else {
				SKSE::log::info("Dialogue Movement Enabler not detected. Disabling compatibility.");
				settings->isUsingDME = false;
			}
		}
		break;
	case SKSE::MessagingInterface::kDataLoaded:
		SkyrimSoulsRE::InstallMenuHooks();
		break;
	}
}

	void InitializeLog()
{
#ifndef NDEBUG
	auto sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
#else
	auto path = logger::log_directory();
	if (!path) {
		util::report_and_fail("Failed to find standard logging directory"sv);
	}

	*path /= fmt::format("{}.log"sv, Plugin::NAME);
	auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
#endif

#ifndef NDEBUG
	const auto level = spdlog::level::trace;
#else
	const auto level = spdlog::level::info;
#endif

	auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));
	log->set_level(level);
	log->flush_on(level);

	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("%s(%#): [%^%l%$] %v"s);
}

extern "C" DLLEXPORT bool SKSEAPI
	SKSEPlugin_Query(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info)
{
	a_info->infoVersion = SKSE::PluginInfo::kVersion;
	a_info->name = Plugin::NAME.data();
	a_info->version = Plugin::VERSION[0];

	if (a_skse->IsEditor()) {
		logger::critical("Loaded in editor, marking as incompatible"sv);
		return false;
	}

	const auto ver = a_skse->RuntimeVersion();
	if (ver < SKSE::RUNTIME_1_5_39) {
		logger::critical(FMT_STRING("Unsupported runtime version {}"), ver.string());
		return false;
	}

	return true;
}

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(SKSE::LoadInterface* a_skse)
{
		InitializeLog();
		SKSE::AllocTrampoline(1 << 9, true);
		SKSE::Init(a_skse);

		auto messaging = SKSE::GetMessagingInterface();
		if (messaging->RegisterListener("SKSE", MessageHandler))
		{
			SKSE::log::info("Messaging interface registration successful.");
		}
		else
		{
			SKSE::log::critical("Messaging interface registration failed.");
			return false;
		}

		SkyrimSoulsRE::LoadSettings();
		SKSE::log::info("Settings loaded.");

		SkyrimSoulsRE::InstallHooks();
		SKSE::log::info("Hooks installed.");

		SKSE::log::info("Skyrim Souls RE loaded.");

		return true;
	};
