// ExplorerFrame.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "ExplorerFrame.h"
#include "..\Injector\WndProcExport.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    hUxtheme = LoadLibraryExW(L"uxtheme.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);

    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int) msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_EXPLORERFRAME));
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = L"ExplorerFrame";
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    auto a = RegisterClassExW(&wcex);
    return a;
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   static const fnSetPreferredAppMode SetPreferredAppMode = (fnSetPreferredAppMode)GetProcAddress(hUxtheme, MAKEINTRESOURCEA(135));
   SetPreferredAppMode(ForceDark);

   HWND hWnd = CreateWindowW(L"ExplorerFrame", L"", 0,
      0, 0, 0, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   // Create a new win event hook for every window creation.
   HWINEVENTHOOK hook = SetWinEventHook(
       EVENT_OBJECT_CREATE,
       EVENT_OBJECT_CREATE,
       hInstance,
       WinEventProc,
       0,
       0,
       WINEVENT_OUTOFCONTEXT);

    // Enumerate opened windows.
   EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
      HandleWindow(hwnd);
      return TRUE;
   }, NULL);

   // Create a notification icon, using version 4
   // with Windows' built it tooltip and context menu support.
   NOTIFYICONDATA nid = { sizeof(NOTIFYICONDATA) };
   wcscpy_s(nid.szTip, L"ExplorerFrame");
   nid.hWnd = hWnd;
   nid.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_EXPLORERFRAME));
   // nid.guidItem = notifyIconGuid;
   nid.uID = 1;
   nid.uCallbackMessage = WM_USER + 1;
   nid.uVersion = NOTIFYICON_VERSION_4;
   nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP /*| NIF_GUID*/;

   Shell_NotifyIcon(NIM_ADD, &nid);
   Shell_NotifyIcon(NIM_SETVERSION, &nid);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_DESTROY:
        {
        for (const auto& [ _, value ] : hookingMap) {
            // Try to unhook DLL (wil::unique_hhook::get()), do nothing if failed.
            try {
                UnhookWindowsHookEx(value);
            }
            catch (...) {}
        }
            PostQuitMessage(0);
            break;
        }
	// Create a new case for our notification icon
	case WM_USER + 1:
		switch (LOWORD(lParam))
		{
		case WM_CONTEXTMENU:
		{
			// Get the mouse location using WPARAM.
			// Note: if the mouse is outside the notification icon rect, use the center point of the icon instead.
            POINT pt = { GET_X_LPARAM(wParam), GET_Y_LPARAM(wParam) };
			if (pt.x == -1 && pt.y == -1)
			{
				RECT rc;
				// NotifyIconGetRect
                NOTIFYICONIDENTIFIER id;
                id.cbSize = sizeof(NOTIFYICONIDENTIFIER);
                id.hWnd = hWnd;
                id.uID = 1;
				Shell_NotifyIconGetRect(&id, &rc);
				pt.x = (rc.left + rc.right) / 2;
				pt.y = (rc.top + rc.bottom) / 2;
			}
			HMENU hMenu = CreatePopupMenu();
			AppendMenu(hMenu, MF_STRING, 1, L"Exit");
			SetForegroundWindow(hWnd);
			UINT clicked = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_NONOTIFY, pt.x, pt.y, 0, hWnd, NULL);
			DestroyMenu(hMenu);
			if (clicked == 1)
			{
				DestroyWindow(hWnd);
			}
		}
		break;
		}
		break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void WinEventProc(
    HWINEVENTHOOK hWinEventHook,
    DWORD         event,
    HWND          hwnd,
    LONG          idObject,
    LONG          idChild,
    DWORD         idEventThread,
    DWORD         dwmsEventTime)
{
    HandleWindow(hwnd);
}

void HandleWindow(HWND hwnd)
{
    DWORD processId;
    DWORD tid = GetWindowThreadProcessId(hwnd, &processId);
    HANDLE processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
    TCHAR processName[MAX_PATH];
    DWORD length = MAX_PATH;
    QueryFullProcessImageNameW(processHandle, 0, processName, &length);
    CloseHandle(processHandle);

    // Check if the process is the Windows Explorer
    if (wcscmp(processName, L"C:\\Windows\\explorer.exe") == 0 && hookingMap.find(tid) == hookingMap.end())
    {
        HMODULE dll = LoadLibrary(L"Injector.dll");
        if (!dll)
            return;
        auto procAddr = GetProcAddress(dll, "InjectExplorerHook");
        auto method = reinterpret_cast<PFN_INJECT_EXPLORER_HOOK>(procAddr);
        HHOOK hook = method(hwnd);
        hookingMap.insert(std::pair<DWORD, HHOOK>(tid, hook));
        FreeLibrary(dll);
    }
}