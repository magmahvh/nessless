#define NOMINMAX
#include <Windows.h>

#include "valve_sdk/sdk.hpp"
#include "helpers/utils.hpp"
#include "helpers/input.hpp"
#include "features/aimbot.hpp"
#include "features/kit_parser.h"
#include "helpers/logs.hpp"
#include "lua/CLua.h"

#include "hooks.hpp"
#include "menu.hpp"
#include "options.hpp"
#include "render.hpp"
#include "user.hpp"

DWORD WINAPI OnDllAttach(LPVOID base)
{
	while (!GetModuleHandleA("serverbrowser.dll"))
		Sleep(1000);

#ifdef _DEBUG
    Utils::AttachConsole();
#endif

    try {
        Utils::ConsolePrint("Initializing...\n");
        Interfaces::Initialize();
        Interfaces::Dump();

        NetvarSys::Get().Initialize();
        InputSys::Get().Initialize();
		Render::Get().Initialize();
        Menu::Get().Initialize();
       // Lua::Get().Initialize();

        g_Legitbot = new CLegitbot();
        g_Ragebot = new CRagebot();
        g_Options.Initialize();
        initialize_kits();

        Hooks::Initialize();

        // Register some hotkeys.
        // - Note:  The function that is called when the hotkey is pressed
        //          is called from the WndProc thread, not this thread.
        // 

        // Panic button
      /*  InputSys::Get().RegisterHotkey(VK_DELETE, [base]() {
            g_Unload = true;
        });*/

        // Menu Toggle
        InputSys::Get().RegisterHotkey(VK_INSERT, [base]() {
            Menu::Get().Toggle();
        });

        Utils::ConsolePrint("Finished.\n");
		Utils::ConsolePrint("Built on: %s %s\n", __DATE__, __TIME__);

        Logs::Get().Create("Welcome back to NessLess");

        while(!g_Unload)
            Sleep(1000);

        g_CVar->FindVar("crosshair")->SetValue(true);

        FreeLibraryAndExitThread(static_cast<HMODULE>(base), 1);

    } catch(const std::exception& ex) {
        Utils::ConsolePrint("An error occured during initialization:\n");
        Utils::ConsolePrint("%s\n", ex.what());
        Utils::ConsolePrint("Press any key to exit.\n");
        Utils::ConsoleReadKey();
        Utils::DetachConsole();

        FreeLibraryAndExitThread(static_cast<HMODULE>(base), 1);
    }

    // unreachable
    //return TRUE;
}

BOOL WINAPI OnDllDetach()
{
#ifdef _DEBUG
    Utils::DetachConsole();
#endif

    Hooks::Shutdown();
    delete g_Legitbot;
    delete g_Ragebot;

    Menu::Get().Shutdown();
    return TRUE;
}

BOOL WINAPI DllMain(
    _In_      HINSTANCE hinstDll,
    _In_      DWORD     fdwReason,
    _In_opt_  LPVOID    lpvReserved
)
{
    switch(fdwReason) {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls(hinstDll);
            CreateThread(nullptr, 0, OnDllAttach, hinstDll, 0, nullptr);
            return TRUE;
        case DLL_PROCESS_DETACH:
            if(lpvReserved == nullptr)
                return OnDllDetach();
            return TRUE;
        default:
            return TRUE;
    }
}
