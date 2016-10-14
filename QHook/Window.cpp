#include "Window.h"
#include "GLWA.h"
#include "../Shared.h"

bool CWindow::IsValid()
{
	return ((m_hWnd != NULL) && (IsWindow(m_hWnd)));
}

CWindow CWindow::GetParentWindow()
{
	return CWindow(GetParent(m_hWnd));
}

int CWindow::GetText(LPTSTR lpString, int nMaxCount)
{
	return GetWindowText(m_hWnd, lpString, nMaxCount);
}

int CWindow::GetClass(LPTSTR lpString, int nMaxCount)
{
	return GetClassName(m_hWnd, lpString, nMaxCount);
}

int CWindow::GetTextLength()
{
	return GetWindowTextLength(m_hWnd);
}

bool CWindow::IsVisible()
{
	return IsWindowVisible(m_hWnd);
}

void CWindow::Close()
{
	if(!IsValid()) return;

	PostMessage(m_hWnd, WM_CLOSE, NULL, NULL);
}

void CWindow::Resize(int X, int Y, int W, int H)
{
	if(!IsValid()) return;

	SetWindowPos(m_hWnd, NULL, X, Y, W, H, SWP_ASYNCWINDOWPOS | SWP_NOZORDER);
}

void CWindow::UnDock()
{
	if(!IsValid()) return;
	SendMessage(m_hWnd, RegisterWindowMessage("HTW_UNDOCK"), 0, 0);
	//UnHookWindow(hWnd);
}

void CWindow::Record(bool bStatus)
{
	//if (!IsValid()) return;

	SendMessage(m_hWnd, RegisterWindowMessage("HTW_UNDOCK"), (WPARAM)bStatus, 0);
}

void CWindow::Dock(DockEdge edge)
{
	if(!IsValid()) return;
	//Remove from taskbar (make toolwindow) and make top-most
	ShowInTaskBar(false);
	MakeTopMost(true);
	//Create appbar
	APPBARDATA abd;
	RECT rc;
	GetWindowRect(m_hWnd, &rc);
	int iWidth  = rc.right  - rc.left;
	int iHeight	= rc.bottom - rc.top;
	abd.cbSize	= sizeof(APPBARDATA);
	abd.hWnd	= m_hWnd;
	abd.uCallbackMessage = RegisterWindowMessage("HTW_APPBARMESSAGE");
	SHAppBarMessage(ABM_NEW, &abd);
	switch(edge)
	{
	case deRight:
		abd.uEdge = ABE_RIGHT;
		rc.right = GetSystemMetrics(SM_CXSCREEN);
		rc.left = rc.right - iWidth;
		rc.top = 0;
		rc.bottom = GetSystemMetrics(SM_CYSCREEN);
	break;
	case deLeft:
		abd.uEdge = ABE_LEFT;
		rc.left = 0;
		rc.right = rc.left + iWidth;
		rc.top = 0;
		rc.bottom = GetSystemMetrics(SM_CYSCREEN);
	break;
	case deBottom:
		abd.uEdge = ABE_BOTTOM;
		rc.left = 0;
		rc.right = GetSystemMetrics(SM_CXSCREEN);
		rc.bottom = GetSystemMetrics(SM_CYSCREEN);
		rc.top = rc.bottom - iHeight;
	break;
	case deTop:
		abd.uEdge = ABE_TOP;
		rc.left = 0;
		rc.right = GetSystemMetrics(SM_CXSCREEN);
		rc.top = 0;
		rc.bottom = rc.top + iHeight;
	break;
	}
	abd.rc = rc;
	SHAppBarMessage(ABM_QUERYPOS, &abd);
	switch(edge)
	{
	case deRight:
		abd.rc.left = abd.rc.right - iWidth;
	break;
	case deLeft:
		abd.rc.right = abd.rc.left + iWidth;
	break;
	case deBottom:
		abd.rc.top = abd.rc.bottom - iHeight;
	break;
	case deTop:
		abd.rc.bottom = abd.rc.top + iHeight;
	break;
	}
	SHAppBarMessage(ABM_SETPOS, &abd);
	//Dock the window, and set our proc to check for resizing
/*Dock(hWnd,abd.uEdge);*/
	SendMessage(m_hWnd, RegisterWindowMessage("HTW_DOCK"), (WPARAM)abd.uEdge, NULL);
	MoveWindow(m_hWnd, abd.rc.left, abd.rc.top ,abd.rc.right - abd.rc.left, abd.rc.bottom - abd.rc.top, true);
}

void CWindow::Rename(char* buffer)
{
	if(!IsValid()) return;

	SetWindowText(m_hWnd, buffer);
}

void CWindow::Hide()
{
	if(!IsValid()) return;
	ShowWindow(m_hWnd, SW_HIDE);
}

void CWindow::BringToFront()
{
	if(!IsValid()) return;
	BringWindowToTop(m_hWnd);
}

void CWindow::Show()
{
	if(!IsValid()) return;
	ShowWindow(m_hWnd,SW_SHOW);
	//BringWindowToTop(m_hWnd);
}

void CWindow::Minimize()
{
	if(!IsValid()) return;
	ShowWindow(m_hWnd, SW_MINIMIZE);
}

void CWindow::Maximize()
{
	if(!IsValid()) return;
	ShowWindow(m_hWnd, SW_MAXIMIZE);
	//BringWindowToTop(m_hWnd);
}

bool CWindow::IsMinimized()
{
	if(!IsValid()) return false;

	WINDOWPLACEMENT wndpl;
	wndpl.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(m_hWnd, &wndpl);

	return ((wndpl.showCmd == SW_MINIMIZE) | (wndpl.showCmd == SW_SHOWMINIMIZED));
}

bool CWindow::IsMaximized()
{
	if(!IsValid()) return false;

	WINDOWPLACEMENT wndpl;
	wndpl.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(m_hWnd, &wndpl);

	return ((wndpl.showCmd == SW_MAXIMIZE) | (wndpl.showCmd == SW_SHOWMAXIMIZED));
}

void CWindow::Restore()
{
	if(!IsValid()) return;
	ShowWindow(m_hWnd, SW_RESTORE);
	//BringWindowToTop(m_hWnd);
}

void CWindow::MakeTopMost(bool bTop)
{
	if(!IsValid()) return;
	if(bTop)
		SetWindowPos(	m_hWnd, HWND_TOPMOST, 0, 0, 0, 0,
						SWP_ASYNCWINDOWPOS | SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);

	else SetWindowPos(	m_hWnd, HWND_NOTOPMOST, 0, 0, 0, 0,
						SWP_ASYNCWINDOWPOS | SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
}

bool CWindow::IsTopMost()
{
	if(!IsValid()) return false;

	return ((GetWindowLong(m_hWnd, GWL_EXSTYLE) & WS_EX_TOPMOST) == WS_EX_TOPMOST);
}

bool CWindow::IsInTaskBar()
{
	if(!IsValid()) return false;

	long style = GetWindowLong(m_hWnd, GWL_EXSTYLE);
	return ((style & WS_EX_TOOLWINDOW) == 0);
}

void CWindow::ShowInTaskBar(bool bShow)
{	
	if(!IsValid()) return;

	ShowWindow(m_hWnd, SW_HIDE);
	if(bShow)
		SetWindowLong(m_hWnd, GWL_EXSTYLE, (GetWindowLong(m_hWnd, GWL_EXSTYLE) &~ WS_EX_TOOLWINDOW) | WS_EX_APPWINDOW);
	else SetWindowLong(m_hWnd, GWL_EXSTYLE, (GetWindowLong(m_hWnd, GWL_EXSTYLE) &~ WS_EX_APPWINDOW) | WS_EX_TOOLWINDOW );
	ShowWindow(m_hWnd,SW_SHOW);
}

void CWindow::SendToTray()
{
	if(!IsValid()) return;

	NOTIFYICONDATA nid;
	ZeroMemory(&nid, sizeof(nid));
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = g_hWndHTW;
	nid.uID = (UINT)m_hWnd;
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	nid.uCallbackMessage = WM_TRAYMESSAGE;
	
	nid.hIcon = (HICON)SendMessage(m_hWnd, WM_GETICON, ICON_SMALL, NULL);
	if(nid.hIcon == NULL)
		nid.hIcon = (HICON)GetClassLong(m_hWnd, GCL_HICONSM);
	if(nid.hIcon == NULL)
		nid.hIcon = LoadIcon(NULL, IDI_INFORMATION);

	GetWindowText(m_hWnd, nid.szTip, sizeof(nid.szTip));
	Shell_NotifyIcon(NIM_ADD, &nid);

	ShowWindow(m_hWnd, SW_HIDE);
}

void CWindow::MakeTransparent(int iAlpha)
{
	if(!IsValid()) return;
	if(iAlpha != 255)
	{
		SetWindowLong(m_hWnd, GWL_EXSTYLE, GetWindowLong(m_hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
		SetLayeredWindowAttributes(m_hWnd, 0, iAlpha, LWA_ALPHA);
	}
	else
	{
		SetWindowLong(m_hWnd, GWL_EXSTYLE, GetWindowLong(m_hWnd,GWL_EXSTYLE) &~ WS_EX_LAYERED &~ WS_EX_TRANSPARENT);
		RedrawWindow(m_hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_FRAME | RDW_ALLCHILDREN);
	}
}

int CWindow::GetTransparency()
{
	if(!IsValid()) return 0;

	byte alpha = 0;  
	if((GetLayeredWindowAttributesEx != NULL)
		&& GetLayeredWindowAttributesEx(m_hWnd, NULL, &alpha, NULL) != 0)
	{
		alpha = 255 - alpha;
	}
	return alpha;
}

void CWindow::SetFocusable(bool bFocus)
{
	if(!IsValid()) return;
	if(bFocus)
		SetWindowLong(m_hWnd, GWL_EXSTYLE, GetWindowLong(m_hWnd,GWL_EXSTYLE) &~ WS_EX_TRANSPARENT);
	else SetWindowLong(m_hWnd, GWL_EXSTYLE, GetWindowLong(m_hWnd,GWL_EXSTYLE) | WS_EX_LAYERED | WS_EX_TRANSPARENT);
}

bool CWindow::IsFocusable()
{
	if(!IsValid()) return true;

	return ((GetWindowLong(m_hWnd, GWL_EXSTYLE) & WS_EX_TRANSPARENT) == 0);
}

bool CWindow::GetRect(LPRECT lpRect)
{
	return GetWindowRect(m_hWnd, lpRect);
}

bool CWindow::operator==(CWindow wnd)
{
	return (m_hWnd == wnd.m_hWnd);
}

bool CWindow::operator!=(CWindow wnd)
{
	return (m_hWnd != wnd.m_hWnd);
}