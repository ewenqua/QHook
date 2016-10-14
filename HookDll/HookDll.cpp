#include <windows.h>
#include <fstream>
#include "HookDll.h"
#include "../Shared.h"
#include <string.h>

const UINT MAXSIZE = 1000;
UINT HTW_UNDOCK;
UINT HTW_UNSUBCLASS;
UINT HTW_APPBARMESSAGE;
UINT HTW_DOCK;
UINT HTW_ACTION;
UINT HTW_RECORD;

using namespace std;

//Initialized Data to be shared with all instance of the dll
#pragma data_seg("Shared")
HWND g_hWndHTW = NULL;
HINSTANCE g_hInstance = NULL;
UINT g_iCount = 0;
bool g_bRecordStatus = false;
#pragma data_seg()
// Initialised data End of data share

//Uninitialized data
#pragma bss_seg("SharedBss")
HWND	g_hWndWindow[MAXSIZE];		//Window handle
long	g_OldWindowProc[MAXSIZE];	//Old window proc
bool	g_bDocked[MAXSIZE];			//If docked
UINT	g_uEdge[MAXSIZE];			//If so, its edge
HMENU	g_hMenu[MAXSIZE];			//Our integrated menu
#pragma bss_seg()
//Uninitialized data

BOOL WINAPI DllMain(HANDLE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	switch(fdwReason)
	{	
		case DLL_PROCESS_ATTACH:
		{
			HTW_UNDOCK = RegisterWindowMessage(L"HTW_UNDOCK");
			HTW_APPBARMESSAGE = RegisterWindowMessage(L"HTW_APPBARMESSAGE");
			HTW_DOCK = RegisterWindowMessage(L"HTW_DOCK");
			HTW_ACTION = RegisterWindowMessage(L"HTW_ACTION");
			HTW_UNSUBCLASS = RegisterWindowMessage(L"HTW_UNSUBCLASS");
			HTW_RECORD = RegisterWindowMessage(L"HTW_RECORD");

			if(g_hInstance == NULL)
				g_hInstance=(HINSTANCE)hinstDLL;
		}
		break;
		case DLL_PROCESS_DETACH:
		{
			//UnSubclass();	Do it from HTW! instead...
		}
		break;
	}
	return TRUE;
}

void WINAPI InitDll(HWND hWndMain)
{
	g_hWndHTW = hWndMain;
}

int GetWindowId(HWND hWnd)
{
	for(UINT i=0;i<g_iCount;i++)
	{
		if(g_hWndWindow[i] == hWnd)
			return i;
	}
	return -1;
}

void RemoveFromList(int i)
{
	for(UINT j = i; j < g_iCount - 1; j++)
	{
		g_hWndWindow[j]	= g_hWndWindow[j+1];
		g_OldWindowProc[j]= g_OldWindowProc[j+1];
		g_uEdge[j] = g_uEdge[j+1];
	}
	g_iCount--;
	g_hWndWindow[g_iCount] = NULL;
	g_OldWindowProc[g_iCount] = NULL;
	g_uEdge[g_iCount] = 0;
}

void RemoveFromList(HWND hWnd)
{
	RemoveFromList(GetWindowId(hWnd));
}

//Returns the number of docked windows to the main window
int WINAPI GetDockedCount()
{
	int c = 0;
	for(UINT i=0;i<g_iCount;i++)
	{
		if(g_bDocked[i])
			c++;
	}
	return c;
}

void WINAPI SetRecordStatus(bool bStatus)
{
	g_bRecordStatus = bStatus;
}


void AppBarQuerySetPos(UINT uEdge, LPRECT lprc, PAPPBARDATA pabd) 
{ 
    int iHeight = 0; 
    int iWidth = 0; 

    pabd->rc = *lprc; 
    pabd->uEdge = uEdge; 

    if ((uEdge == ABE_LEFT) || (uEdge == ABE_RIGHT))
	{ 
        iWidth = pabd->rc.right - pabd->rc.left; 
	}
	else
	{ 
        iHeight = pabd->rc.bottom - pabd->rc.top; 
    }

    // Query the system for an approved size and position. 
    SHAppBarMessage(ABM_QUERYPOS, pabd); 

    // Adjust the rectangle, depending on the edge to which the 
    // appbar is anchored. 
    switch (uEdge)
	{ 
        case ABE_LEFT: 
            pabd->rc.right = pabd->rc.left + iWidth; 
        break; 
        case ABE_RIGHT: 
            pabd->rc.left = pabd->rc.right - iWidth; 
        break; 
        case ABE_TOP: 
            pabd->rc.bottom = pabd->rc.top + iHeight; 
        break; 
		case ABE_BOTTOM: 
            pabd->rc.top = pabd->rc.bottom - iHeight; 
        break; 
    } 
    // Pass the final bounding rectangle to the system. 
    SHAppBarMessage(ABM_SETPOS, pabd); 
}

void  AppBarPosChanged(PAPPBARDATA pabd) 
{
    RECT rc; 
    RECT rcWindow; 
    int iHeight; 
    int iWidth; 

    rc.top = 0; 
    rc.left = 0; 
    rc.right = GetSystemMetrics(SM_CXSCREEN); 
    rc.bottom = GetSystemMetrics(SM_CYSCREEN); 

    GetWindowRect(pabd->hWnd, &rcWindow); 
    iHeight = rcWindow.bottom - rcWindow.top; 
    iWidth = rcWindow.right - rcWindow.left; 

    for(UINT i=0;i<g_iCount;i++)
	{
		if(g_hWndWindow[i] == pabd->hWnd)
		{
			switch (g_uEdge[i])
			{ 
			case ABE_TOP: 
				rc.bottom = rc.top + iHeight; 
				break; 

			case ABE_BOTTOM: 
				rc.top = rc.bottom - iHeight; 
				break; 

			case ABE_LEFT: 
				rc.right = rc.left + iWidth; 
				break; 

			case ABE_RIGHT: 
				rc.left = rc.right - iWidth; 
				break; 
			} 
			AppBarQuerySetPos(g_uEdge[i], &rc, pabd); 
			break;
		}
	}
}

void InsertDockMenu(HWND hWnd, HMENU hMenu)
{
	//Determine if docked
	bool bDocked = false;
	UINT i=0;
	for(i=0;i<g_iCount;i++)
	{
		if(g_hWndWindow[i] == hWnd)
		{
			bDocked = g_bDocked[i];
			break;
		}
	}
	if(!bDocked)
	{
		HMENU hDock = CreatePopupMenu();
		InsertMenu(hDock, (UINT)-1, MF_BYPOSITION | MF_STRING, HTW_DOCKLEFT, L"Left");	
		InsertMenu(hDock, (UINT)-1, MF_BYPOSITION | MF_STRING, HTW_DOCKTOP, L"Top");	
		InsertMenu(hDock, (UINT)-1, MF_BYPOSITION | MF_STRING, HTW_DOCKRIGHT, L"Right");	
		InsertMenu(hDock, (UINT)-1, MF_BYPOSITION | MF_STRING, HTW_DOCKBOTTOM, L"Bottom");

		ModifyMenu(hMenu, (UINT)5, MF_BYPOSITION | MF_POPUP | MF_STRING, (unsigned int)hDock, L"Dock");	
	}
	else
	{
		HMENU hDock;
		if((hDock = GetSubMenu(hMenu, 5)) != NULL)
			DestroyMenu(hDock);
		ModifyMenu(hMenu, (UINT)5, MF_BYPOSITION | MF_STRING | MF_POPUP, HTW_DOCK_UNDOCK, L"Undock");
	}
}

HMENU SetupMenu(HWND hWnd)
{
	HMENU hSubMenu;
	HMENU hMenu;

	hSubMenu = CreatePopupMenu();
	InsertMenu(hSubMenu, (UINT)-1, MF_BYPOSITION | MF_STRING, HTW_HIDE, L"Hide");	
	InsertMenu(hSubMenu, (UINT)-1, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
	InsertMenu(hSubMenu, (UINT)-1, MF_BYPOSITION | MF_STRING, HTW_TASKBAR, L"Show in taskbar");	
	InsertMenu(hSubMenu, (UINT)-1, MF_BYPOSITION | MF_STRING, HTW_TRAY, L"Send to tray");	
	InsertMenu(hSubMenu, (UINT)-1, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
	InsertMenu(hSubMenu, (UINT)-1, MF_BYPOSITION | MF_STRING, 0, L"");		//This one will be modified (ID=5)

	InsertDockMenu(hWnd, hSubMenu);

	InsertMenu(hSubMenu, (UINT)-1, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
	InsertMenu(hSubMenu, (UINT)-1, MF_BYPOSITION | MF_STRING, HTW_TOPMOST, L"Top-most");	
	InsertMenu(hSubMenu, (UINT)-1, MF_BYPOSITION | MF_STRING, HTW_FOCUSABLE, L"Focusable");
	InsertMenu(hSubMenu, (UINT)-1, MF_BYPOSITION | MF_STRING, HTW_TRANS, L"Transparency");	

	hMenu = GetSystemMenu(hWnd, FALSE);
	InsertMenu(hMenu, (UINT)0, MF_BYPOSITION | MF_POPUP | MF_STRING, (unsigned int)hSubMenu, L"HideThatWindow!");
	InsertMenu(hMenu, (UINT)1, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);

	return hSubMenu;
}

void ReleaseMenu(HWND hWnd)
{
	HMENU hMenu = GetSystemMenu(hWnd, FALSE);
	RemoveMenu(hMenu,0,MF_BYPOSITION);	//SubMenu
	RemoveMenu(hMenu,0,MF_BYPOSITION);	//Separator
}

LRESULT CALLBACK DockWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{	
	if(uMsg == HTW_UNDOCK)	//Using if because case expression not constant
	{
		MessageBox(NULL, L"undock", L"record", MB_OK);

		DoUnDockWindow(hWnd);
	}
	else if(uMsg == HTW_APPBARMESSAGE)
	{
		APPBARDATA abd; 
		abd.cbSize = sizeof(APPBARDATA);
		abd.hWnd = hWnd;
		switch(wParam)
		{
			case ABN_POSCHANGED:
			{
				AppBarPosChanged(&abd);
				MoveWindow(abd.hWnd, abd.rc.left, abd.rc.top, 
					abd.rc.right - abd.rc.left, 
					abd.rc.bottom - abd.rc.top, TRUE);
			}
		}
	}
	else switch(uMsg)
	{
		case WM_ACTIVATE:
		{
			//MessageBox(hWnd,"Activate","Activate",MB_OK);
			APPBARDATA abd;
			abd.cbSize = sizeof(APPBARDATA);
			abd.hWnd = hWnd;
			SHAppBarMessage(ABM_ACTIVATE,&abd);
		}
		break;
		/*case WM_SIZE:
		{
			switch(wParam)
			{
				case SIZE_MINIMIZED:
				{
					APPBARDATA abd;
					abd.cbSize = sizeof(APPBARDATA);
					abd.hWnd = hWnd;
					SHAppBarMessage(ABM_REMOVE,&abd);					
				}
				break;
			}
		}*/
		break;
		case WM_WINDOWPOSCHANGED:
		{
			APPBARDATA abd; 
			abd.cbSize = sizeof(APPBARDATA);
			abd.hWnd = hWnd;
			AppBarPosChanged(&abd);
			SHAppBarMessage(ABM_WINDOWPOSCHANGED,&abd);
		}
		break;
	}
	long hWinProc = NULL;
	for(UINT i=0;i<g_iCount;i++)
	{
		if(g_hWndWindow[i] == hWnd)
		{
			hWinProc = g_OldWindowProc[i];
			break;
		}
	}
	return CallWindowProc((WNDPROC)hWinProc,hWnd,uMsg,wParam,lParam);
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	long hWinProc = NULL;
	bool bDocked = false;
	UINT i=0;
	for(i=0;i<g_iCount;i++)
	{
		if(g_hWndWindow[i] == hWnd)
		{
			hWinProc = g_OldWindowProc[i];
			bDocked = g_bDocked[i];
			break;
		}
	}
	if(uMsg == HTW_DOCK)
	{
		g_bDocked[i] = true;
		g_uEdge[i] = (UINT)wParam;
	}
	else if(uMsg == HTW_UNSUBCLASS)
	{
		int id = GetWindowId(hWnd);
		SetWindowLong(hWnd, GWL_WNDPROC, (long)g_OldWindowProc[id]);  //UnSubclass !!!!
		ReleaseMenu(g_hWndWindow[id]);		
	}
	else switch(uMsg)
	{
		case WM_SYSCOMMAND:	//Check menu clicks
			switch(LOWORD(wParam))
			{
				case HTW_DOCKLEFT:
				case HTW_DOCKTOP:
				case HTW_DOCKRIGHT:
				case HTW_DOCKBOTTOM:
				case HTW_DOCK_UNDOCK:
				case HTW_HIDE:
				case HTW_TRAY:
				case HTW_TRANS:
					PostMessage(g_hWndHTW, HTW_ACTION, (WPARAM)LOWORD(wParam), (LPARAM)hWnd);	//Don't use sendmessage (not at trans at least)
				break;
				case HTW_TASKBAR:
				case HTW_TOPMOST:		//These need to check their states...
				case HTW_FOCUSABLE:
				{
					HMENU menu = GetSystemMenu(hWnd, false);
					UINT state = GetMenuState(menu, LOWORD(wParam), MF_BYCOMMAND);
					//Report to main window
					SendMessage(g_hWndHTW, HTW_ACTION, MAKEWPARAM(LOWORD(wParam), ((state & MF_CHECKED)?0:1)), (LPARAM)hWnd);
				}
				break;
			}
			break;
		case WM_DESTROY:
			WindowDestroyed(hWnd);
			break;
		case WM_INITMENU:
		{
			//TODO: Check if our menu!
			HMENU menu = g_hMenu[i];
			InsertDockMenu(hWnd, g_hMenu[i]);
			CheckMenuItem(menu, HTW_TOPMOST,MF_BYCOMMAND |
				(((GetWindowLong(hWnd, GWL_EXSTYLE) & WS_EX_TOPMOST)) ? MF_CHECKED : MF_UNCHECKED));
			CheckMenuItem(menu, HTW_FOCUSABLE, MF_BYCOMMAND |
				(((GetWindowLong(hWnd, GWL_EXSTYLE) & WS_EX_TRANSPARENT)) ? MF_UNCHECKED : MF_CHECKED));
			CheckMenuItem(menu, HTW_TASKBAR, MF_BYCOMMAND | 
				(((GetWindowLong(hWnd, GWL_EXSTYLE) & WS_EX_TOOLWINDOW)) ? MF_UNCHECKED : MF_CHECKED));
			break;
		}
	}
	//If docked, continue to the dock-proc...
	if(bDocked)
		return DockWindowProc(hWnd,uMsg,wParam,lParam);
	//If not docked, simply return with the windows' own windowproc
	return CallWindowProc((WNDPROC)hWinProc, hWnd, uMsg, wParam, lParam);
}

void WindowCreated(HWND hWnd)
{
	if(GetWindowId(hWnd) != -1)
		return;
	g_hWndWindow[g_iCount] = hWnd;
	g_OldWindowProc[g_iCount] = NULL;
	g_OldWindowProc[g_iCount] = SetWindowLong(hWnd,GWL_WNDPROC,(long)WindowProc);  //Subclass !!!!
	g_hMenu[g_iCount] = SetupMenu(hWnd);
	g_iCount++;
}

void WindowDestroyed(HWND hWnd)
{
	if(g_bDocked[GetWindowId(hWnd)])
	{
		DoUnDockWindow(hWnd);
	}
	RemoveFromList(hWnd);
}

LRESULT CALLBACK ShellProc(int nCode,WPARAM wParam,LPARAM lParam)
{
	switch(nCode)
	{
	//TODO: Fix support for owned windows (a'la Delphi (TApplication))
	case HSHELL_WINDOWACTIVATED:
		WindowCreated((HWND)wParam);
		break;
	case HSHELL_WINDOWCREATED:
		/*	MessageBox(NULL,"HSHELL_WINDOWCREATED","HSHELL_WINDOWCREATED",MB_OK);*/
		WindowCreated((HWND)wParam);
		break;
	}
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	// By returning a non-zero value from the hook procedure, the  
	// message does not get passed to the target window  
	KBDLLHOOKSTRUCT *pkbhs = (KBDLLHOOKSTRUCT *)lParam;
	BOOL bControlKeyDown = 0;

	if (nCode == HC_ACTION)
	{
		if (pkbhs->vkCode == '1' && pkbhs->flags & LLKHF_ALTDOWN)
		{
			g_bRecordStatus = true;
			return CallNextHookEx(NULL, nCode, wParam, lParam);
		}
		if (pkbhs->vkCode == '2' && pkbhs->flags & LLKHF_ALTDOWN)
		{
			g_bRecordStatus = false;
			return CallNextHookEx(NULL, nCode, wParam, lParam);
		}
	}

	if (nCode == HC_ACTION && wParam == WM_KEYDOWN)
	{
		if (g_bRecordStatus == false)
		{
			return CallNextHookEx(NULL, nCode, wParam, lParam);
		}

		wchar_t str[80] = L"";
		// Convert the virtual key code into a scancode (as required by GetKeyNameText).
		UINT scanCode = MapVirtualKeyEx(pkbhs->vkCode, 0, GetKeyboardLayout(0));
		switch (pkbhs->vkCode)
		{
			// Certain keys end up being mapped to the number pad by the above function,
			// as their virtual key can be generated by the number pad too.
			// If it's one of the known number-pad duplicates, set the extended bit:
			case VK_INSERT:
			case VK_DELETE:
			case VK_HOME:
			case VK_END:
			case VK_NEXT:  // Page down
			case VK_PRIOR: // Page up
			case VK_LEFT:
			case VK_RIGHT:
			case VK_UP:
			case VK_DOWN:
				scanCode |= 0x100; // Add extended bit
				break;
		}

		// GetKeyNameText() expects the scan code to be on the same format as WM_KEYDOWN
		GetKeyNameText(scanCode << 16, str, 80);
		
		std::ofstream os;
		os.open(RECORD_FILE_NAME, std::ofstream::out | std::ofstream::app);
		os << RECORD_KEYBOARD << "name:" << (wcslen(str) == 0 ? "unknown" : UnicodeToAnsi(str)) << ",value:" << pkbhs->vkCode << "\r\n";
		os << std::flush;
		os.close();

		/*
		// Check to see if the CTRL key is pressed  
		bControlKeyDown = GetAsyncKeyState(VK_CONTROL) >> ((sizeof(SHORT) * 8) - 1);
		//Disable CTRL+ESC  
		if (pkbhs->vkCode == VK_ESCAPE && bControlKeyDown){
			return 1;
		}
		//Disable ALT+TAB  
		if (pkbhs->vkCode == VK_TAB && pkbhs->flags & LLKHF_ALTDOWN)
			return 1;
		*/
	}
	  
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (g_bRecordStatus == false)
		return CallNextHookEx(NULL, nCode, wParam, lParam);

	if (wParam == WM_LBUTTONUP || wParam == WM_RBUTTONUP)
	{
		MSLLHOOKSTRUCT *pmhs = (MSLLHOOKSTRUCT*)lParam;
		POINT pt = pmhs->pt;

		std::ofstream os;
		os.open(RECORD_FILE_NAME, std::ofstream::out | std::ofstream::app);
		std::string mouseButton = RECORD_MOUSELEFT; // Left or Right
		if (wParam == WM_RBUTTONUP) mouseButton = RECORD_MOUSERIGHT;

		os << mouseButton.c_str() << "x:" << pt.x << ",y:" << pt.y << "\r\n";
		os << std::flush;
		os.close();
	}

	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

int WINAPI DoUnDockWindow(HWND hWnd)
{
	for(UINT i=0;i<g_iCount;i++)
	{
		if(g_hWndWindow[i] == hWnd)
		{
			APPBARDATA abd;
			abd.cbSize = sizeof(APPBARDATA);
			abd.hWnd = hWnd;
			SHAppBarMessage(ABM_REMOVE,&abd);
			g_bDocked[i] = false;
			break;
		}
	}
	return 1;
}
//Called when HTW! terminates, UnHooks all hooked windows...
int WINAPI UnSubclass()
{
	for(UINT i=0;i<g_iCount;i++)
	{
		SendMessage(g_hWndWindow[i], HTW_UNSUBCLASS ,NULL, NULL);
	}
	return 1;
}
