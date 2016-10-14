#include "PropertiesDialog.h"
#include "Window.h"

BYTE g_uPropsAlpha;
HWND g_hPropsWnd;

int WINAPI PropsProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
	case WM_INITDIALOG:
	{
		SendMessage(GetDlgItem(hDlg, IDC_SLIDER_TRANS), TBM_SETRANGE, TRUE, MAKELONG(0, 255));
		SendMessage(GetDlgItem(hDlg, IDC_SLIDER_TRANS), TBM_SETPOS, TRUE, g_uPropsAlpha);
		return 1;
	}
	case WM_HSCROLL:
	{
		int pos = (int)SendMessage(GetDlgItem(hDlg, IDC_SLIDER_TRANS), TBM_GETPOS, 0, 0);
		CWindow window(g_hPropsWnd);
		window.MakeTransparent(255 - pos);
		return 1;
	}
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
		case IDCANCEL:
			EndDialog(hDlg, LOWORD(wParam));
			return 1;
		}
		break;
	}
	return 0;
}
