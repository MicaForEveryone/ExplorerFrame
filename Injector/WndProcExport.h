#pragma once
#include "pch.h"

extern "C"
__declspec(dllexport)
HHOOK InjectExplorerHook(HWND window) noexcept;
static LRESULT CALLBACK CallWndProc(int nCode, WPARAM wParam, LPARAM lParam) noexcept;
using PFN_INJECT_EXPLORER_HOOK = decltype(&InjectExplorerHook);