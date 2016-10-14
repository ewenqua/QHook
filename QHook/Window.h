#ifndef __WINDOW_H__
#define __WINDOW_H__

#include <windows.h>

enum DockEdge
{
	deRight,
	deLeft,
	deBottom,
	deTop
};

class CWindow
{
public:
	HWND m_hWnd;

	CWindow(HWND hWnd): m_hWnd(hWnd) {}

	CWindow GetParentWindow();
	void Hide();
	void Show();
	void Close();
	void Minimize();
	void Maximize();
	void Restore();
	void MakeTopMost(bool bTop);
	void ShowInTaskBar(bool bShow);
	void SendToTray();
	void MakeTransparent(int iAlpha);
	void SetFocusable(bool bFocus);
	void Resize(int X, int Y, int W, int H);
	void Rename(char* buffer);
	void Dock(DockEdge edge);
	void UnDock();
	void Record(bool bStatus);
	void BringToFront();

	int	 GetText(LPTSTR lpString, int nMaxCount);
	int	 GetClass(LPTSTR lpString, int nMaxCount);
	int  GetTextLength();
	int  GetTransparency();
	bool GetRect(LPRECT lpRect);
	
	bool IsValid();
	bool IsVisible();
	bool IsInTaskBar();	//TODO: Is this really true??
	bool IsTopMost();
	bool IsFocusable();
	bool IsMinimized();
	bool IsMaximized();

	void Highlight();
	void UnHighlight();
	void FlashBlink();

	bool operator==(CWindow wnd);
	bool operator!=(CWindow wnd);
};

#endif //__WINDOW_H__