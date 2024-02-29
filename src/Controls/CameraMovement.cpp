#include "Controls/CameraMovement.h"
#include <xbyak\xbyak.h>

namespace SkyrimSoulsRE::CameraMovement
{
	bool CameraMove_Hook(bool a_result)
	{
		Settings* settings = Settings::GetSingleton();
		RE::UI* ui = RE::UI::GetSingleton();
		RE::PlayerControls* pc = RE::PlayerControls::GetSingleton();
		RE::ControlMap* controlMap = RE::ControlMap::GetSingleton();
		RE::MenuControls* mc = RE::MenuControls::GetSingleton();

		if (!ui->GameIsPaused() && (!ui->IsMenuOpen(RE::DialogueMenu::MENU_NAME) || settings->isUsingDME) && settings->enableCursorCameraMove &&
			GetUnpausedMenuCount() && !IsFullScreenMenuOpen() && !mc->remapMode &&
			pc->lookHandler->IsInputEventHandlingEnabled() && controlMap->IsLookingControlsEnabled())
		{
			RE::MenuCursor* menuCursor = RE::MenuCursor::GetSingleton();

			float speedX = settings->cursorCameraHorizontalSpeed;
			float speedY = settings->cursorCameraVerticalSpeed;

			if (menuCursor->cursorPosX == 0)
			{
				pc->data.lookInputVec.x = -speedX;
			}
			else if (menuCursor->cursorPosX == menuCursor->screenWidthX)
			{
				pc->data.lookInputVec.x = speedX;
			}

			if (menuCursor->cursorPosY == 0)
			{
				pc->data.lookInputVec.y = speedY;
			}
			else if (menuCursor->cursorPosY == menuCursor->screenWidthY)
			{
				pc->data.lookInputVec.y = -speedY;
			}
		}

		return a_result;
	}

	void InstallHook()
	{
		struct CameraMove_Code : Xbyak::CodeGenerator
		{
			CameraMove_Code(uintptr_t a_hookAddr)
			{
				Xbyak::Label hookAddress;

				pop(r13);
				pop(r12);
				pop(rdi);
				mov(rcx, rax);
				jmp(ptr[rip + hookAddress]);

				L(hookAddress);
				dq(a_hookAddr);
			}
		};

		CameraMove_Code code{ std::uintptr_t(CameraMove_Hook) };
		void* codeLoc = SKSE::GetTrampoline().allocate(code);

		SKSE::GetTrampoline().write_branch<5>(Offsets::Misc::ScreenEdgeCameraMoveHook.address() + 0x241, codeLoc);		//VERIFIED
	}
}
