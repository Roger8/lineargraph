#pragma once
// Window library implemented by WinGdiClass project
//
#include "WinGdiClass.h"
#pragma comment(lib, "WinGdiClass.lib")

// SEED library implemented by Seed project
//
#include "OperateStruct.h"
#include "SeedFile.h"
#pragma comment(lib, "Seed.lib")

#include "LinearGraphLog.h"

#include <Ole2.h>
#include <shlobj.h>
#pragma comment(lib, "shell32.lib")

// Using GDI+ library
//
#include <GdiPlus.h>
#pragma comment(lib, "gdiplus.lib")

// Application resources
//
#include "Resource/resource.h"

#include <math.h>

#include <vector>

typedef std::vector<bool>   CMaskVect;

enum Messages
{
    // WPARAM, reinterpreted as size_t, length of document path
    // LPARAM, reinterpreted as PCWSTR, pointer to document path
    //
    LinearGraphOpenDocument = WM_USER+1,

    // WPARAM, not used
    // LPARAM, reinterpreted as CLinearGraphView*, pointer to the view
    //that sends this message
    //
    LinearGraphViewActivate,

    // WPARAM, reinterpreted as UINT, ID of the tab control
    // LPARAM, reinterpreted as CWindow*, pointer to the window that
    //invokes this notification
    //
    LinearGraphTabSelectChange,
};