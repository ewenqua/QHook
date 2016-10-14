#include "HTW.h"
#include "Window.h"
#include "AboutDialog.h"
#include "PropertiesDialog.h"
#include "Hook.h"

#include <iostream>
#include <string>  

#include <windows.h>
#include <commctrl.h>
#include <stack>

using namespace std;

#define TIMER_UPDATE	20

HINSTANCE g_hInstance;
HWND hWndTree;
HWND g_hWndHTW;
CWindow g_wndHTW(NULL);

//HWND hWndRoot;
CWindow g_wndRoot(NULL);
CWindow wndSelected(NULL);

enum EViewMode {VM_SIMPLE, VM_ADVANCED};

EViewMode g_viewMode = VM_SIMPLE;

bool g_bUpdatingStatus = false;
bool g_bRecordStatus = false;
UINT uTrayedWindows = 0;

//Proc. called by EnumWindowsEx, fills the list with our windows
BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam)
{
	CWindow wnd(hWnd);
	CWindow* wndParent = (CWindow*)lParam;

	if(wnd.GetParentWindow() != *wndParent) return TRUE;
	
	TVITEM tvi; 
	TVINSERTSTRUCT tvins;

	static HTREEITEM hPrev = TVI_FIRST;
	static HTREEITEM hParent = TVI_ROOT;

	int length = wnd.GetTextLength();

	char className[250];
	char wndText[250];
	char buffer[500];
	wndText[0] = '\0';
	buffer[0] = '\0';
	wnd.GetText(wndText, sizeof(wndText));
	wnd.GetClass(className, sizeof(className));
	sprintf_s(buffer, sizeof(buffer), "%s (%s)", wndText, className);
	
	tvi.mask = TVIF_TEXT | TVIF_PARAM;
	tvi.cchTextMax = sizeof(buffer);
	tvi.lParam = (LPARAM)wnd.m_hWnd;
	if(g_viewMode == VM_SIMPLE)
	{		
		if((wnd.GetParentWindow().m_hWnd == NULL) && (length > 0) && (wnd.IsVisible()))
		{
			tvi.pszText = wndText;

			tvins.item = tvi;
			tvins.hInsertAfter = hPrev;
			tvins.hParent = hParent;
			hPrev = (HTREEITEM)SendMessage(hWndTree, TVM_INSERTITEM, 0, (LPARAM)(LPTVINSERTSTRUCT)&tvins);
			
			HTREEITEM hPrevParent = hParent;
			hParent = hPrev;
			EnumChildWindows(wnd.m_hWnd, EnumWindowsProc, (LPARAM)&wnd);
			hParent = hPrevParent;
		}
	}
	else
	{	
		tvi.pszText = buffer;

		tvins.item = tvi;
		tvins.hInsertAfter = hPrev;
		tvins.hParent = hParent;
		hPrev = (HTREEITEM)SendMessage(hWndTree, TVM_INSERTITEM, 0, (LPARAM)(LPTVINSERTSTRUCT)&tvins);
		
		HTREEITEM hPrevParent = hParent;
		hParent = hPrev;
		EnumChildWindows(wnd.m_hWnd, EnumWindowsProc, (LPARAM)&wnd);
		hParent = hPrevParent;
	}

	return TRUE;
}

//Clears the whole window list
void ClearWindowList()
{
	TreeView_DeleteAllItems(hWndTree);
}

//Updates the stats of a window (text,transp,top-most,focusable,position and size)
void ShowWindowStatus(CWindow wnd)
{
	if(!wnd.IsValid()) return;

	g_bUpdatingStatus = true;
	
	SendMessage(GetDlgItem(g_hWndHTW, IDC_SLIDER_TRANS), TBM_SETPOS, TRUE, wnd.GetTransparency());
	SendMessage(GetDlgItem(g_hWndHTW, IDC_CHECKAOT), BM_SETCHECK, wnd.IsTopMost() ? BST_CHECKED : BST_UNCHECKED, NULL);
	SendMessage(GetDlgItem(g_hWndHTW, IDC_CHECKFOCUS), BM_SETCHECK, wnd.IsFocusable() ? BST_CHECKED : BST_UNCHECKED, NULL);
	SendMessage(GetDlgItem(g_hWndHTW, IDC_CHECK_INTASKBAR), BM_SETCHECK, wnd.IsInTaskBar() ? BST_CHECKED : BST_UNCHECKED, NULL);
	
	char buffer[250];
	wnd.GetText(buffer, sizeof(buffer));
	SetWindowText(GetDlgItem(g_hWndHTW, IDC_TEXT), buffer);

	char className[250];
	wnd.GetClass(className, sizeof(className));
	SetWindowText(GetDlgItem(g_hWndHTW, IDC_WCLASS), className);
	
	//X,Y,W,H
	RECT rect;
	wnd.GetRect(&rect);
	POINT lefttop, bottomright;
	lefttop.x = rect.left;
	lefttop.y = rect.top;
	bottomright.x = rect.right;
	bottomright.y = rect.bottom;

	ScreenToClient(wnd.GetParentWindow().m_hWnd, &lefttop);
	ScreenToClient(wnd.GetParentWindow().m_hWnd, &bottomright);
	
	char buf[8];
	_itoa_s(lefttop.x, buf, 8, 10);
	SetWindowText(GetDlgItem(g_hWndHTW, IDC_X), buf);
	_itoa_s(lefttop.y, buf, 8, 10);
	SetWindowText(GetDlgItem(g_hWndHTW, IDC_Y), buf);
	_itoa_s(bottomright.x - lefttop.x, buf, 8, 10);
	SetWindowText(GetDlgItem(g_hWndHTW, IDC_W), buf);
	_itoa_s(bottomright.y - lefttop.y, buf, 8, 10);
	SetWindowText(GetDlgItem(g_hWndHTW, IDC_H), buf);

	if(wnd.IsVisible())
		SetWindowText(GetDlgItem(g_hWndHTW, IDC_BUTTON_SHOWHIDE), "Hide");
	else SetWindowText(GetDlgItem(g_hWndHTW, IDC_BUTTON_SHOWHIDE), "Show");

	if(wnd.IsMaximized())
		SetWindowText(GetDlgItem(g_hWndHTW, IDC_BUTTON_MAXIMIZE), "Restore");
	else SetWindowText(GetDlgItem(g_hWndHTW, IDC_BUTTON_MAXIMIZE), "Maximize");

	if(wnd.IsMinimized())
		SetWindowText(GetDlgItem(g_hWndHTW, IDC_BUTTON_MINIMIZE), "Restore");
	else SetWindowText(GetDlgItem(g_hWndHTW, IDC_BUTTON_MINIMIZE), "Minimize");

	g_bUpdatingStatus = false;
}

//Updates the window list with the root's children
void UpdateWindowList(CWindow wndRoot)
{
	g_wndRoot = wndRoot;
	//Update child window list
	g_bUpdatingStatus = true;
	SendMessage(hWndTree, WM_SETREDRAW, 0, 0); //Do not redraw until done!

	ClearWindowList();
	EnumChildWindows(g_wndRoot.m_hWnd, EnumWindowsProc, (LPARAM)&g_wndRoot);

	TreeView_SortChildren(hWndTree, TVI_ROOT, TRUE);

	SendMessage(hWndTree, WM_SETREDRAW, 1, 0);
	ShowWindowStatus(wndSelected);
	g_bUpdatingStatus = false;
}

bool SelectWindow(CWindow wnd)
{
	static bool updating = false;

	wndSelected = wnd;
	HTREEITEM hItem;

	stack<CWindow> parents;
	parents.push(wnd);
	
	CWindow window = wnd;
	while((window = window.GetParentWindow()).m_hWnd != NULL)
	{
		parents.push(window);
	}

	hItem = TreeView_GetRoot(hWndTree);
	bool found = false;

	TVITEM tvi;
	tvi.mask = TVIF_PARAM;

	while((window = parents.top()).m_hWnd != NULL)
	{
		parents.pop();
		do
		{
			tvi.hItem = hItem;
			TreeView_GetItem(hWndTree, &tvi);
			if((HWND)tvi.lParam == window.m_hWnd)
			{
				break;
			}
		}while((hItem = TreeView_GetNextSibling(hWndTree, hItem)) != NULL);

		if((HWND)tvi.lParam == wndSelected.m_hWnd)
		{
			found = true;
			break;
		}
		if(parents.empty())
		{
			if(updating) return false;

			updating = true;
			//If the stack is empty and found still is false, our window is not in the list
			UpdateWindowList(g_wndRoot);
			found = SelectWindow(wndSelected);
			updating = false;

			return found;
		}
		hItem = TreeView_GetChild(hWndTree, hItem);
	}

	if(!found)
		return false;

	g_bUpdatingStatus = true;
	TreeView_SelectItem(hWndTree, hItem);
	g_bUpdatingStatus = false;

	ShowWindowStatus(wnd);
	return true;
}

void CALLBACK UpdateTimer(HWND hwnd, UINT msg, UINT idTimer, DWORD dwTime)
{
	UpdateWindowList(g_wndRoot);
	SelectWindow(wndSelected);	
}

//Main Window Procedure
int WINAPI WndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static bool bCaptured = false;
	CWindow window(message == HTW_ACTION ? (HWND)lParam : wndSelected);
	if(message == HTW_ACTION)
	{
		switch(LOWORD(wParam))
		{
		case HTW_HIDE:
			window.Hide();
			break;
		case HTW_TASKBAR:
			window.ShowInTaskBar(HIWORD(wParam) == 1);
			break;
		case HTW_TRAY:
			window.SendToTray();
			uTrayedWindows++;
			break;
		case HTW_DOCKLEFT:
			window.Dock(deLeft);
			break;
		case HTW_DOCKTOP:
			window.Dock(deTop);
			break;
		case HTW_DOCKRIGHT:
			window.Dock(deRight);
			break;
		case HTW_DOCKBOTTOM:
			window.Dock(deBottom);
			break;
		case HTW_DOCK_UNDOCK:
			window.UnDock();
			break;
		case HTW_TOPMOST:
			window.MakeTopMost(HIWORD(wParam) == 1);
			break;
		case HTW_FOCUSABLE:
			window.SetFocusable(HIWORD(wParam) == 1);
			break;
		case HTW_TRANS:
			g_hPropsWnd = window.m_hWnd;
			g_uPropsAlpha = window.GetTransparency();

			if(DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_PROPS), window.m_hWnd, PropsProc) == IDCANCEL)
			{
				window.MakeTransparent(255 - g_uPropsAlpha);
			}
			break;
		}
		return 1;
	}
	else switch(message)
	{
	case WM_SYSCOMMAND:
		switch(wParam)
		{
		case SC_MINIMIZE:
			g_wndHTW.SendToTray();
			return 1;
		}
		break;
	case WM_CLOSE:
		g_hWndHTW = NULL;	//Causes the messageloop to terminate
		return 0;
	case WM_TRAYMESSAGE:
		switch(lParam)
		{
			case WM_LBUTTONDBLCLK:
			{
				//Delete the icon
				NOTIFYICONDATA nid;
				ZeroMemory(&nid, sizeof(nid));
				nid.cbSize = sizeof(NOTIFYICONDATA);
				nid.hWnd = g_hWndHTW;
				nid.uID = (UINT)wParam; 
				Shell_NotifyIcon(NIM_DELETE, &nid); 
				uTrayedWindows--;
				//Show the window
				if(!IsWindow((HWND)wParam)) return 0;
				
				ShowWindow((HWND)wParam,SW_SHOW);
				return 1;
			}
			case WM_RBUTTONUP:
			{
				//TODO: Make work!
				HMENU windowMenu = GetSystemMenu((HWND)wParam, false);
				//Get mouse position
				POINT pt;
				GetCursorPos(&pt);
				//Show the menu and set our window as the message handler
				SetForegroundWindow((HWND)wParam);
				TrackPopupMenu(windowMenu, TPM_LEFTALIGN, pt.x, pt.y, 0, (HWND)wParam, NULL);
				PostMessage((HWND)wParam, WM_NULL, 0, 0);
				//DestroyMenu(windowMenu);
				return 1;
			}
			break;
		}
		break;
	case WM_LBUTTONUP:
		if(bCaptured)
		{
			bCaptured = false;
			ReleaseCapture();
			SetCursor(LoadCursor(NULL,IDC_ARROW));
			SendMessage(GetDlgItem(g_hWndHTW,IDC_FIND),STM_SETIMAGE,IMAGE_ICON,(WPARAM)LoadIcon(g_hInstance,MAKEINTRESOURCE(IDI_FINDER)));
			wndSelected.UnHighlight();
			return 1;
		}
		break;
	case WM_LBUTTONDOWN:
	{
		POINT pt;
		RECT rect;
		GetCursorPos(&pt);
		GetWindowRect(GetDlgItem(g_hWndHTW,IDC_FIND),&rect);	
		//See if the drag icon was clicked
		if((pt.x >= rect.left) && (pt.x <= rect.right) &&
			(pt.y >= rect.top) && (pt.y <= rect.bottom))
		{
			bCaptured = true;
			SetCursor(LoadCursor(g_hInstance,MAKEINTRESOURCE(IDC_CFINDER)));
			SendMessage(GetDlgItem(g_hWndHTW, IDC_FIND), STM_SETIMAGE, IMAGE_ICON, NULL);
			SetCapture(g_hWndHTW);
			return 1;
		}
		break;
	}
	case WM_MOUSEMOVE:
	{
		if(bCaptured)
		{
			POINT pt;
			GetCursorPos(&pt);
			CWindow wnd(WindowFromPoint(pt));
			if(wnd.m_hWnd != wndSelected.m_hWnd)
			{
				//Redraw old window (Remove Highlight)
				wndSelected.UnHighlight();
				//Highlight new window
				wnd.Highlight();
				/*if(!SelectWindow(wnd))
					MessageBox(g_hWndHTW, "Window not found!", "HTW! - Error", MB_ICONERROR | MB_OK);*/
				SelectWindow(wnd);
				return 1;
			}
		}
		break;
	}
	case WM_INITDIALOG:
	{
		g_hWndHTW = hDlg;
		g_wndHTW = CWindow(g_hWndHTW);
#ifdef _HOOK
		InitDll(g_hWndHTW);
#endif
		hWndTree = GetDlgItem(hDlg, IDC_TREE);
		SendMessage(g_hWndHTW,WM_SETICON,ICON_BIG,(LPARAM)LoadImage(g_hInstance,MAKEINTRESOURCE(IDI_ICON),IMAGE_ICON,32,32,LR_DEFAULTCOLOR));
		SendMessage(g_hWndHTW,WM_SETICON,ICON_SMALL,(LPARAM)LoadImage(g_hInstance,MAKEINTRESOURCE(IDI_ICON),IMAGE_ICON,16,16,LR_DEFAULTCOLOR));
		//Set finder icon
		SendMessage(GetDlgItem(g_hWndHTW,IDC_FIND),STM_SETIMAGE,IMAGE_ICON,(WPARAM)LoadIcon(g_hInstance,MAKEINTRESOURCE(IDI_FINDER)));
		//Set slider range
		SendMessage(GetDlgItem(g_hWndHTW,IDC_SLIDER_TRANS),TBM_SETRANGE,TRUE,MAKELONG(0,255));
		//Advanced mode = default
		SendMessage(GetDlgItem(g_hWndHTW,IDC_RADIOSIMPLE), BM_SETCHECK, BST_CHECKED, 0);
		UpdateWindowList(CWindow(NULL));
		//SetTimer(g_hWndHTW, TIMER_UPDATE, 10000, UpdateTimer);	//TODO: Needs some work

		return 1;
	}
	case WM_HSCROLL:
	{
		int pos = (int)SendMessage(GetDlgItem(g_hWndHTW, IDC_SLIDER_TRANS), TBM_GETPOS, 0, 0);
		window.MakeTransparent(255 - pos);
		return 1;
	}
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_RADIOSIMPLE:
			g_viewMode = VM_SIMPLE;
			UpdateWindowList(g_wndRoot);
			return 1;
		case IDC_RADIOADV:
			g_viewMode = VM_ADVANCED;
			UpdateWindowList(g_wndRoot);
			return 1;
		case IDC_CHECKAOT:
		{		
			if(SendMessage(GetDlgItem(g_hWndHTW,IDC_CHECKAOT),BM_GETCHECK,0,NULL) == BST_CHECKED)
				window.MakeTopMost(true);
			else window.MakeTopMost(false);

			return 1;
		}
		case IDC_CHECKFOCUS:
		{
			if(SendMessage(GetDlgItem(g_hWndHTW, IDC_CHECKFOCUS), BM_GETCHECK, 0, NULL) == BST_CHECKED)
				window.SetFocusable(true);
			else
			{
				//A little warning message here...
				if((window.m_hWnd != g_hWndHTW) | ((window.m_hWnd == g_hWndHTW) && (MessageBox(g_hWndHTW,
					"You are about to delete the HideThatWindow! main window's focus. Restoring it will be impossible, if you don't start another instance of HideThatWindow! and enable its focus again. Are you sure you want to do this?",
					"Are you sure?",
					MB_YESNO | MB_ICONQUESTION) == IDYES)))
					window.SetFocusable(false);
				else
					SendMessage(GetDlgItem(g_hWndHTW, IDC_CHECKFOCUS), BM_SETCHECK, BST_CHECKED, 0);
			}
			return 1;
		}
		case IDC_CHECK_INTASKBAR:
		{
			if(SendMessage(GetDlgItem(g_hWndHTW, IDC_CHECK_INTASKBAR), BM_GETCHECK, 0, NULL) == BST_CHECKED)
				window.ShowInTaskBar(true);
			else window.ShowInTaskBar(false);
			return 1;
		}
		case IDCANCEL:
		{
			//Version 0.35 - Warning messages when closing
			int dc = GetDockedCount();
			if(dc > 0)
			{
				if(MessageBox(g_hWndHTW, "You are trying to close HideThatWindow! with docked windows open. If you do this, the automatic undocking and sizechanging procedures won't work. Are you sure you want to do this?",
					"Are you sure?",
					MB_YESNO | MB_ICONQUESTION) == IDNO)
				{
					return 0;
				}
			}
			if(uTrayedWindows > 0)
			{
				if(MessageBox(g_hWndHTW, "You are trying to close HideThatWindow! with window icons in tray. If you do this, the icons will be deleted and the window won't be restored. Are you sure you want to do this?",
					"Are you sure?",
					MB_YESNO | MB_ICONQUESTION) == IDNO)
				{
					return 0;
				}
			}
			EndDialog(hDlg, LOWORD(wParam));
			return 1;
		}
		case IDC_BUTTON_BRINGUP:
			window.BringToFront();
			return 1;
		case IDC_BUTTON_UPDATE:
			//Flash the window
			//window.FlashBlink();
			UpdateWindowList(g_wndRoot);
			SelectWindow(wndSelected);
			return 1;
		case IDC_BUTTON_SHOWHIDE:
		{
			if(window.IsVisible())
			{
				//Hiding HideThatWindow! is no good...
				if((window.m_hWnd != g_hWndHTW) | ((window.m_hWnd == g_hWndHTW) && (MessageBox(g_hWndHTW,
						"You are about to hide the HideThatWindow! main window. Showing it again will be impossible, if you don't start another instance and show the hidden window. Are you sure you want to hide this window?",
						"Are you sure?",
						MB_YESNO | MB_ICONQUESTION) == IDYES)))
				{
						window.Hide();
				}
			}
			else
				window.Show();
			ShowWindowStatus(window);
			return 1;
		}
		case IDC_BUTTON_CLOSE:
		{
			window.Close();
			UpdateWindowList(g_wndRoot.m_hWnd);
			return 1;
		}
		case IDC_BUTTON_MINIMIZE:
			if(window.IsMinimized())
				window.Restore();
			else window.Minimize();
			g_wndHTW.BringToFront();
			ShowWindowStatus(window);
			return 1;
		case IDC_BUTTON_MAXIMIZE:
			if(window.IsMaximized())
				window.Restore();
			else window.Maximize();
			g_wndHTW.BringToFront();
			ShowWindowStatus(window);
			return 1;
		case IDC_BUTTON_TOTRAY:
			window.SendToTray();
			uTrayedWindows++;
			return 1;
		case IDC_BUTTON_ABOUT:
			DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_ABOUT), g_hWndHTW, AboutProc);
			return 1;

		case IDC_BUTTON_RECORD:	
			if (g_bRecordStatus == false)
			{
				SetWindowText(GetDlgItem(g_hWndHTW, IDC_BUTTON_RECORD), "Stop REC");
				g_bRecordStatus = true;
				//wndSelected.Record(true);
				//SendMessage(g_hWndHTW, RegisterWindowMessage("HTW_UNDOCK"), (WPARAM)true, 0);
			}
			else
			{
				SetWindowText(GetDlgItem(g_hWndHTW, IDC_BUTTON_RECORD), "Start REC");
				g_bRecordStatus = false;
				//wndSelected.Record(false);
				//SendMessage(g_hWndHTW, RegisterWindowMessage("HTW_UNDOCK"), (WPARAM)false, 0);
			}
			SetRecordStatus(g_bRecordStatus);
			return 1;
		case IDC_BUTTON_REPEAT:
		{
			//DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_ABOUT), g_hWndHTW, AboutProc);
			window.BringToFront();
			DWORD threadID;
			HANDLE hThread = CreateThread(NULL, 0, ThreadProc, NULL, 0, &threadID);
			//SetWindowText(GetDlgItem(g_hWndHTW, IDC_BUTTON_SHOWHIDE), "Hide");
			return 1;
		}
		case IDC_BUTTON_DOCK:
			{
				HMENU hMenu, hMenuPopup;
				POINT pt;
				GetCursorPos(&pt);
				hMenu = LoadMenu(g_hInstance,MAKEINTRESOURCE(IDR_MENUDOCK));
				hMenuPopup = GetSubMenu(hMenu,0);
				TrackPopupMenu(hMenuPopup,
					TPM_LEFTALIGN | TPM_RIGHTBUTTON,
					pt.x,pt.y,0,g_hWndHTW,NULL);
				DestroyMenu(hMenu);
			}
			return 1;
		case ID_DOCK_LEFT:
			window.Dock(deLeft);
			return 1;
		case ID_DOCK_RIGHT:
			window.Dock(deRight);
			return 1;
		case ID_DOCK_TOP:
			window.Dock(deTop);
			return 1;
		case ID_DOCK_BOTTOM:
			window.Dock(deBottom);
			return 1;
		case ID_DOCK_UNDOCK:
			window.UnDock();
			return 1;
		case IDC_X:
		case IDC_Y:
		case IDC_W:
		case IDC_H:
			if((HIWORD(wParam) == EN_CHANGE)&&(!g_bUpdatingStatus))
			{
				char buffer[5];
				GetWindowText(GetDlgItem(g_hWndHTW,IDC_X),buffer,5);
				int x= atoi(buffer);
				GetWindowText(GetDlgItem(g_hWndHTW,IDC_Y),buffer,5);
				int y = atoi(buffer);
				GetWindowText(GetDlgItem(g_hWndHTW,IDC_W),buffer,5);
				int w = atoi(buffer);
				GetWindowText(GetDlgItem(g_hWndHTW,IDC_H),buffer,5);
				int h = atoi(buffer);
				window.Resize(x, y, w, h);
				return 1;
			}
			break;
		case IDC_TEXT:
			if((HIWORD(wParam) == EN_CHANGE) && (!g_bUpdatingStatus))
			{
				int length = GetWindowTextLength(GetDlgItem(g_hWndHTW, IDC_TEXT)) + 1;
				char* buffer = new char[length];
				GetWindowText(GetDlgItem(g_hWndHTW, IDC_TEXT), buffer, length);
				window.Rename(buffer);
				return 1;
			}
			break;
		}
		break;
	case WM_NOTIFY:
	{
		NMHDR nmhdr = *((LPNMHDR)lParam);
		if((nmhdr.hwndFrom == hWndTree) && (nmhdr.code == TVN_SELCHANGED))
		{
			if(g_bUpdatingStatus) break;
			LPNMTREEVIEW pnmtv = (LPNMTREEVIEW)lParam;
			TVITEM item = pnmtv->itemNew;
			wndSelected = CWindow((HWND)item.lParam);
			wndSelected.FlashBlink();
			ShowWindowStatus(wndSelected);
		}
		break;
	}
	}
	return 0;
}

DWORD WINAPI ThreadProc(LPVOID lpParam)
{

	Sleep(1000);

	ifstream ifs(RECORD_FILE_NAME);
	string line;
	if (ifs)
	{
		while (ifs.good())
		{
			getline(ifs, line);
			char p[64] = "";
			strcpy_s(p, line.c_str());
			vector<string> vec_tokens = split(p, ",:");

			if (line.find(RECORD_KEYBOARD) != string::npos)
			{
				keybd_event(atoi(vec_tokens[3].c_str()), 0, 0, 0);
				Sleep(500);
			}
			else if ((line.find(RECORD_MOUSELEFT) != string::npos))
			{
				SetCursorPos(atoi(vec_tokens[1].c_str()), atoi(vec_tokens[3].c_str()));
				mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
				mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
				Sleep(500);
			}
			else if ((line.find(RECORD_MOUSERIGHT) != string::npos))
			{
				SetCursorPos(atoi(vec_tokens[1].c_str()), atoi(vec_tokens[3].c_str()));
				mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);
				mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
				Sleep(500);
			}
		}
	}
	ifs.close();
	return 0;
}

vector<string> split(char* str, char* delim)
{
	char* saveptr;
	char* token = strtok_s(str, delim, &saveptr);

	vector<string> result;

	while (token != NULL)
	{
		result.push_back(token);
		token = strtok_s(NULL, delim, &saveptr);
	}
	return result;
}
