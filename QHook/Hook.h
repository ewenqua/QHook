#ifndef __HOOK_H__
#define __HOOK_H__

#include <windows.h>
#include "resource.h"

#define _HOOK	//Comment out to disable hooking

void HookWindow(HWND hWnd, UINT uEdge);

extern HINSTANCE hLib;

typedef LRESULT (WINAPI* dllprc)(int,WPARAM,LPARAM);
extern dllprc ShellProc;
extern dllprc KeyboardProc;
extern dllprc MouseProc;
typedef int (WINAPI* getdocked)(VOID);
extern getdocked GetDockedCount;
typedef void (WINAPI* initdllprc)(HWND);
extern initdllprc InitDll;
typedef int (WINAPI* unsubclassprc)(VOID);
extern unsubclassprc UnSubclass;

typedef void (WINAPI* recstatus)(bool);
extern recstatus SetRecordStatus;

//Cross process windows messages
extern UINT HTW_UNDOCK;
extern UINT HTW_APPBARMESSAGE;
extern UINT HTW_ACTION;
extern UINT HTW_RECORD;

#endif //__HOOK_H__