#include "HTW.h"
#include "Hook.h"
#include "Window.h"
#include "GLWA.h"

bool LoadDockDll()
{
	//Load our docking (hook) dll and its functions
	if((hLib = LoadLibrary("HookDll.dll")) == NULL)
	{
		MessageBox(NULL,"Couldn't find HookDll.dll, make sure it's in HideThatWindow!'s dir or in path.","Error!",MB_OK | MB_ICONHAND);
		return false;
	}
	//UnHookWindow = (unhkprc)GetProcAddress(hLib,"UnHookWindow");
	GetDockedCount = (getdocked)GetProcAddress(hLib,"GetDockedCount");
	InitDll = (initdllprc)GetProcAddress(hLib,"InitDll");
	//Hook the shell and receive notifications when windows are created, destroyed, ...
	ShellProc = (dllprc)GetProcAddress(hLib, "ShellProc");
	KeyboardProc = (dllprc)GetProcAddress(hLib, "KeyboardProc");
	MouseProc = (dllprc)GetProcAddress(hLib, "MouseProc");

	UnSubclass = (unsubclassprc)GetProcAddress(hLib, "UnSubclass");
	SetRecordStatus = (recstatus)GetProcAddress(hLib, "SetRecordStatus");

	SetWindowsHookEx(WH_SHELL,ShellProc,hLib,NULL);
	SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, hLib, NULL);
	SetWindowsHookEx(WH_MOUSE_LL, MouseProc, hLib, NULL);

	return true;
}

bool Init()
{
#ifdef _HOOK
	if(!LoadDockDll()) return false;
#endif
	LoadGLWA();
	HTW_UNDOCK = RegisterWindowMessage("HTW_UNDOCK");
	HTW_APPBARMESSAGE = RegisterWindowMessage("HTW_APPBARMESSAGE");
	HTW_ACTION = RegisterWindowMessage("HTW_ACTION");
	HTW_RECORD = RegisterWindowMessage("HTW_RECORD");
	return true;
}

void FreeLibraries()
{
	FreeLibrary(hLib);
}

void Release()
{
	FreeLibraries();
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	g_hInstance = hInst;
	if(!Init()) return 1;

	g_hWndHTW = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_HTW), NULL, WndProc);
	g_wndHTW = CWindow(g_hWndHTW);

	if(strstr(lpCmdLine, "/BG"))
	{
		//Start minimized...
		g_wndHTW.SendToTray();
	}
	else
	{
		//Show the dialog...
		g_wndHTW.Show();
	}

	//Message loop
	MSG msg;
	while((GetMessage(&msg, NULL, NULL, NULL)) && (g_hWndHTW != NULL))
	{
		if(!IsDialogMessage(g_hWndHTW, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	
#ifdef _HOOK
	UnSubclass();
#endif
	Release();
	return 0;
}