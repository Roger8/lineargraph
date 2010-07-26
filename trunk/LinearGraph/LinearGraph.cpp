#ifdef _DEBUG

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#endif

#include "LinearGraph.h"
#include "LinearGraphApp.h"

#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' "\
    "version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

//  Application entry

INT WINAPI WinMain(HINSTANCE, HINSTANCE, PSTR, INT)
{
    CLinearGraphApp app;
    if( !app.OnInitApplication() )
    {
        return 0;
    }

    app.DoMessageLoop();
    app.OnExitApplication();

#ifdef _DEBUG
    _CrtDumpMemoryLeaks();
#endif
    return 0;
}