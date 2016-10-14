#include <windows.h>
#include <fstream>

using namespace std;

//Initialized Data to be shared with all instance of the dll
#pragma data_seg("Shared")
HINSTANCE hInstance = NULL;
long OldWindowHandle = NULL;
HWND hTarget = NULL;
#pragma data_seg()
// Initialised data End of data share

//TODO: Multiple instances!

BOOL WINAPI DllMain(HANDLE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	switch(fdwReason)
	{	
		case DLL_PROCESS_ATTACH:
		{
			hInstance=(HINSTANCE)hinstDLL;
		}
		break;

		case DLL_PROCESS_DETACH:
		{
			//UnSubclass();
		}
		break;
	}
	return TRUE;
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam	)
{	
	switch(uMsg)
	{
		case WM_ACTIVATE:
		{
			//MessageBox(hWnd,"Activate","Activate",MB_OK);
			APPBARDATA abd;
			abd.cbSize = sizeof(APPBARDATA);
			abd.hWnd = hWnd;
			SHAppBarMessage(ABM_ACTIVATE,&abd);
		}
		break;
		case WM_WINDOWPOSCHANGED:
		{
			//MessageBox(hWnd,"PosChanged","PosChanged",MB_OK);
			APPBARDATA abd;
			abd.cbSize = sizeof(APPBARDATA);
			abd.hWnd = hWnd;
			SHAppBarMessage(ABM_WINDOWPOSCHANGED,&abd);
		}
		break;
	}
	return CallWindowProc((WNDPROC)OldWindowHandle,hWnd,uMsg,wParam,lParam);
}

LRESULT CALLBACK CBTProc(int nCode,WPARAM wParam,LPARAM lParam)
{
	if(hTarget == NULL)
	{
		MessageBox(NULL,"hTarget = NULL 3","hTarget = NULL 3",MB_OK);
		return CallNextHookEx(NULL,nCode,wParam,lParam);
	}
	//static bool bSubClassed = false;
	if (nCode==HCBT_ACTIVATE)  //Called when the application window is activated
	{
		if(/*(!bSubClassed)&&*/((HWND)(wParam)==hTarget))  //check if the window activated is Our Targer App
		{   	
			//MessageBox(NULL,"SubClassed","",MB_OK);
			OldWindowHandle=SetWindowLong(hTarget,GWL_WNDPROC,(long)WindowProc);  //Subclass !!!!
//			bSubClassed = true;
		}		
	}
	else if (nCode==HCBT_DESTROYWND) //Called when the application window is destroyed
	{
		if(((HWND)wParam==hTarget)/*&&(bSubClassed)*/)
		{
			APPBARDATA abd;
			abd.cbSize = sizeof(APPBARDATA);
			abd.hWnd = hTarget;
			SHAppBarMessage(ABM_REMOVE,&abd);
		}
	}
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

int WINAPI Hook(HWND hWnd)
{
	/*MessageBox(NULL,"int WINAPI Hook(HWND hWnd)","Hook",MB_OK);
	if(hTarget == NULL)
	{
		MessageBox(NULL,"hTarget = NULL 1","hTarget = NULL",MB_OK);
		//return 0;
	}*/
	hTarget = hWnd;
	/*if(hTarget == NULL)
	{
		MessageBox(NULL,"hTarget = NULL 2","hTarget = NULL",MB_OK);
		return 0;
	}*/
	SetWindowsHookEx(WH_CBT,(HOOKPROC)CBTProc,hInstance,GetWindowThreadProcessId(hWnd,NULL));
	return 1;
}

// Function to set the original window procedure of each subclassed window
int WINAPI UnSubclass()
{
	return 1;
}
