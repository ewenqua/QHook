#include "Hook.h"

HINSTANCE hLib;

getdocked GetDockedCount;
dllprc ShellProc;
dllprc KeyboardProc;
dllprc MouseProc;

initdllprc InitDll;
unsubclassprc UnSubclass;
recstatus SetRecordStatus;

UINT HTW_UNDOCK;
UINT HTW_APPBARMESSAGE;
UINT HTW_ACTION;
UINT HTW_RECORD;