#ifndef __PROPERTIESDIALOG_H__
#define __PROPERTIESDIALOG_H__

#include <windows.h>
#include <commctrl.h>
#include "resource.h"

extern byte g_uPropsAlpha;
extern HWND g_hPropsWnd;

int WINAPI PropsProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

#endif //__PROPERTIESDIALOG_H__