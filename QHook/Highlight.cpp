#include "Window.h"
#include "HTW.h"

#define TIMER_FLASH	1

CWindow wndFlash(NULL);
int currentFlash = 0;

void CALLBACK FlashTimer(HWND hwnd, UINT message, UINT idTimer, DWORD dwTime)
{
	if(!wndFlash.IsValid()) return;

	if(currentFlash % 2 == 0)
		wndFlash.Highlight();
	else wndFlash.UnHighlight();
	currentFlash++;
	if(currentFlash == 4)	//Number of flashes times 2 (2=1 flash, 6=3 flashes)
	{
		currentFlash = 0;
		KillTimer(g_wndHTW.m_hWnd, TIMER_FLASH);
		wndFlash = CWindow(NULL);
	}
}

void CWindow::FlashBlink()
{
	if(!IsValid()) return;

	if(wndFlash.m_hWnd != NULL)
	{
		currentFlash = 0;
		KillTimer(g_wndHTW.m_hWnd, TIMER_FLASH);
		wndFlash.UnHighlight();
	}

	wndFlash = CWindow(m_hWnd);
	SetTimer(g_wndHTW.m_hWnd, TIMER_FLASH, 175, (TIMERPROC)FlashTimer);
} 

void CWindow::UnHighlight()
{
	if(!IsValid()) return;

	InvalidateRect(m_hWnd, NULL, TRUE);
	UpdateWindow(m_hWnd);
	RedrawWindow(m_hWnd, NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
}

void CWindow::Highlight()
{
	if(!IsValid()) return;

	HDC hDc = GetWindowDC(m_hWnd);
	HPEN hPen, hPenOld;
	hPen = CreatePen(PS_SOLID, 3, RGB(255, 0, 0));
	hPenOld = (HPEN)SelectObject(hDc, hPen);
	RECT rect;
	GetRect(&rect);
	int right = rect.right - rect.left - 2;
	int bottom = rect.bottom - rect.top - 2;
	MoveToEx(hDc, 0, 0, NULL);
	LineTo(hDc, right, 0);
	MoveToEx(hDc, 0, 0, NULL);
	LineTo(hDc, 0, bottom);
	MoveToEx(hDc, right, 0, NULL);
	LineTo(hDc, right, bottom);
	MoveToEx(hDc, 0, bottom, NULL);
	LineTo(hDc, right, bottom);
	SelectObject(hDc, hPenOld);
	DeleteObject(hPen);
}