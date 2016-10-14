#include <windows.h>

#define RECORD_FILE_NAME "./record_keybd_mouse.txt"
#define RECORD_KEYBOARD "[keybd]"
#define RECORD_MOUSELEFT "[mouseL]"
#define RECORD_MOUSERIGHT "[mouseR]"

#define WM_TRAYMESSAGE WM_USER+16

extern HWND g_hWndHTW;

//Used for sysmenu integration
#define HTW_MENUBASE	0xBA55
#define HTW_HIDE		HTW_MENUBASE + 1
#define HTW_TASKBAR		HTW_MENUBASE + 2
#define HTW_TRAY		HTW_MENUBASE + 3
#define HTW_DOCKLEFT	HTW_MENUBASE + 4
#define HTW_DOCKTOP		HTW_MENUBASE + 5
#define HTW_DOCKRIGHT	HTW_MENUBASE + 6
#define HTW_DOCKBOTTOM	HTW_MENUBASE + 7
#define HTW_DOCK_UNDOCK	HTW_MENUBASE + 8
#define HTW_TOPMOST		HTW_MENUBASE + 9
#define HTW_FOCUSABLE	HTW_MENUBASE + 10
#define HTW_TRANS		HTW_MENUBASE + 11

inline wchar_t* AnsiToUnicode(const char* szStr)
{
	int nLen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szStr, -1, NULL, 0);
	if (nLen == 0)
	{
		return NULL;
	}
	wchar_t* pResult = new wchar_t[nLen];
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szStr, -1, pResult, nLen);
	return pResult;
}

inline char* UnicodeToAnsi(const wchar_t* szStr)
{
	int nLen = WideCharToMultiByte(CP_ACP, 0, szStr, -1, NULL, 0, NULL, NULL);
	if (nLen == 0)
	{
		return NULL;
	}
	char* pResult = new char[nLen];
	WideCharToMultiByte(CP_ACP, 0, szStr, -1, pResult, nLen, NULL, NULL);
	return pResult;
}

