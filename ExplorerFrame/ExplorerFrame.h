#pragma once

#include "resource.h"
#include <unordered_map>

// Global Variables:
HINSTANCE hInst;                                // current instance
std::unordered_map<DWORD, HHOOK> hookingMap = {};
HMODULE hUxtheme;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
void                WinEventProc(
    HWINEVENTHOOK hWinEventHook,
    DWORD         event,
    HWND          hwnd,
    LONG          idObject,
    LONG          idChild,
    DWORD         idEventThread,
    DWORD         dwmsEventTime);
void HandleWindow(HWND hwnd);

// Define SetPreferredAppMode
typedef enum PREFERRED_APP_MODE
{
	Default,
	AllowDark,
	ForceDark,
	ForceLight,
	Max
} PREFERRED_APP_MODE;

using fnSetPreferredAppMode = PREFERRED_APP_MODE(WINAPI*)(PREFERRED_APP_MODE appMode);