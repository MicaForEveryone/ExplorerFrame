// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "detours/detours.h"
#include <dwmapi.h>

static HRESULT(WINAPI* TrueDwmExtendFrameIntoClientArea)(HWND hWnd, _In_ const MARGINS* pMarInset) = DwmExtendFrameIntoClientArea;

bool IsWindowOfInterest(HWND hWnd)
{
	if (!IsWindow(hWnd) || !IsWindowVisible(hWnd))
		return false;

	WCHAR lpClassName[MAX_PATH];

	GetClassName(hWnd, lpClassName, MAX_PATH);

	auto styleEx = GetWindowLongPtrW(hWnd, GWL_EXSTYLE);

	if (styleEx & WS_EX_NOACTIVATE || styleEx & WS_EX_LAYERED)
		return false;

	if (styleEx & WS_EX_APPWINDOW)
		return true;

	auto style = GetWindowLongPtrW(hWnd, GWL_STYLE);

	if (!style)
		return false;

	auto hasTitleBar = style & WS_BORDER && style & WS_DLGFRAME;

	if ((styleEx & WS_EX_TOOLWINDOW || style & WS_POPUP) && !hasTitleBar)
		return false;

	return true;
}

HRESULT WINAPI HookedDwmExtendFrameIntoClientArea(HWND hWnd, _In_ const MARGINS* pMarInset)
{
	if (IsWindowOfInterest(hWnd))
	{
		MARGINS predefinedMargins;
		predefinedMargins.cxLeftWidth =
			predefinedMargins.cxRightWidth =
			predefinedMargins.cyBottomHeight =
			predefinedMargins.cyTopHeight =
			-1;
		return TrueDwmExtendFrameIntoClientArea(hWnd, &predefinedMargins);
	}
	return TrueDwmExtendFrameIntoClientArea(hWnd, pMarInset);
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    if (DetourIsHelperProcess()) {
        return TRUE;
    }
	
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
		DetourRestoreAfterWith();
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)TrueDwmExtendFrameIntoClientArea, HookedDwmExtendFrameIntoClientArea);
        LONG error = DetourTransactionCommit();
        break;
    }
	case DLL_PROCESS_DETACH:
	{
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourDetach(&(PVOID&)TrueDwmExtendFrameIntoClientArea, HookedDwmExtendFrameIntoClientArea);
		DetourTransactionCommit();
	}
    }
    return TRUE;
}

