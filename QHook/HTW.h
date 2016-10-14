#ifndef __HTW_H__
#define __HTW_H__

#include <windows.h>
#include <fstream>
#include <vector>

using namespace std;

#include "Window.h"
#include "resource.h"
#include "../Shared.h"

extern HINSTANCE g_hInstance;
extern UINT uTrayedWindows;
//extern HWND g_hWndHTW;
extern CWindow g_wndHTW;

int WINAPI WndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
DWORD WINAPI ThreadProc(LPVOID lpParam);
vector<string> split(char* str, char* delim);

#endif //__HTW_H__