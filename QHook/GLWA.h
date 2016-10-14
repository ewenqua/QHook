#ifndef __GLWA_H__
#define __GLWA_H__

#include <windows.h>

//To make GetLayeredWindowAttributes work on XP and disable on w2k
/*BOOL GetLayeredWindowAttributes(          HWND hwnd,
    COLORREF *pcrKey,
    BYTE *pbAlpha,
    DWORD *pdwFlags
);*/
typedef BOOL (CALLBACK* glwa)(HWND, COLORREF*, BYTE*, DWORD*);
extern glwa GetLayeredWindowAttributesEx;

void LoadGLWA();

#endif //__GLWA_H__