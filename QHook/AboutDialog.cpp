#include "AboutDialog.h"

int WINAPI AboutProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
	case WM_INITDIALOG:
	{
		//Set about-text
		char buffer[500];
		sprintf_s(buffer, sizeof(buffer), "HideThatWindow! - The Window Magician.\r\n"
			"Version: 1.00 BETA\r\n\r\n"
			" * Hide/Show\r\n"
			" * Close\r\n"
			" * Minimize\r\n"
			" * Maximize\r\n"
			" * Restore\r\n"
			" * Set size\r\n"
			" * Show in/hide from taskbar\r\n"
			" * Send to tray\r\n"
			" * Dock\r\n"
			" * Edit text\r\n"
			" * Make transparent\r\n"
			" * Make top-most\r\n\r\n"
			"Copyright (C) 2007 Emil Andersson\r\nPublished under the GNU General Public License."
		);
		SendMessage(GetDlgItem(hDlg,IDC_ABOUTTEXT),WM_SETTEXT,0,(LPARAM)buffer);
		//Set icon
		SendMessage(GetDlgItem(hDlg,IDC_ABOUTICON),STM_SETIMAGE,IMAGE_ICON,(WPARAM)LoadImage(g_hInstance,MAKEINTRESOURCE(IDI_ICON),IMAGE_ICON,64,64,LR_DEFAULTCOLOR));
		return 1;
	}
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
		case IDCANCEL:
			EndDialog(hDlg,LOWORD(wParam));
			return 1;
		}
		break;
	}
	return 0;
}
