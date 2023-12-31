#include "Common.h"
#include "RemoteServer.h"

#define OBJ_UPDATE_CALL_ADDR 0x54167C
#define Naked __declspec(naked)

const DWORD Get_IOBjRW_ADDR = 0x54BAF0;

// Thanks adoxa
Naked CShip* GetCShip()
{
    __asm {
        call    Get_IOBjRW_ADDR
        test    eax, eax
        jz      noship
        mov     eax, [eax+16]

    noship:
        ret
    }
}

DWORD _;

void Patch(LPVOID vOffset, LPVOID mem, UINT len)
{
    VirtualProtect(vOffset, len, PAGE_EXECUTE_READWRITE, &_);
    memcpy(vOffset, mem, len);
}

void __fastcall SPObjUpdate_Hook(IServerImpl* server, PVOID _edx, SSPObjUpdateInfo &updateInfo, UINT client)
{
    CShip* ship = GetCShip();

    // Get throttle from the ship and set it in the update info, provided the ship isn't NULL
    if (ship)
        updateInfo.fThrottle = ship->get_throttle();

    // Call the original function
    server->SPObjUpdate(_edx, updateInfo, client);
}

void Init()
{
    BYTE callAbsOpcode = 0x15; // Second byte of the absolute dword ptr call opcode
    Patch((PVOID) (OBJ_UPDATE_CALL_ADDR + 1), &callAbsOpcode, sizeof(BYTE)); // Change opcode such that we can hook the function call

    static DWORD hookPtr = (DWORD) SPObjUpdate_Hook;
    DWORD hookPtrRef = (DWORD) &hookPtr;

    // Hook SPObjUpdate function call
    Patch((PVOID) (OBJ_UPDATE_CALL_ADDR + 2), &hookPtrRef, sizeof(DWORD));
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
    UNREFERENCED_PARAMETER(hinstDLL);
    UNREFERENCED_PARAMETER(lpReserved);

    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        Init();
    }

    return TRUE;
}
