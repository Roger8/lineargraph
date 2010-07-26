#pragma once
#include <Windows.h>
#include <CommCtrl.h>

#ifdef  WINGDICLASS_IMPL
#define WINGDI_CLASS   __declspec(dllexport)
#else
#define WINGDI_CLASS   __declspec(dllimport)
#endif

#include "WinFoundation.h"
#include "AppFoundation.h"
#include "CommonControl.h"
#include "DialogBase.h"


#pragma comment(lib, "comctl32.lib")

#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' "\
    "version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
