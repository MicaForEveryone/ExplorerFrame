#include "pch.h"
#include "WndProcExport.h"
#include <wil/win32_helpers.h>

HHOOK InjectExplorerHook(HWND window) noexcept {
	DWORD pid = 0;
	const DWORD tid = GetWindowThreadProcessId(window, &pid);

	HANDLE proc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, pid);

	if (!proc)
	{
		return nullptr;
	}

	return SetWindowsHookEx(WH_CALLWNDPROC, CallWndProc, wil::GetModuleInstanceHandle(), tid);
}

LRESULT CALLBACK CallWndProc(int nCode, WPARAM wParam, LPARAM lParam) noexcept {
	return CallNextHookEx(nullptr, nCode, wParam, lParam);
}