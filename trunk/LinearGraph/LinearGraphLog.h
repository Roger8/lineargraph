// File LinearGraphLog.h
// Author: X.R.
//
// Last Modified: 2010-06-22
//
// Description:
//    This file contains declaration of functions that implements log support.
//    Functions are implemented in LinearGraphLog.cpp
//

#pragma once
#include <Windows.h>

namespace LinearGraph
{
    bool OpenLog();
    void Trace(PCSTR formatString, ...);
    bool FlushLog();
    bool CloseLog();

    inline void _no_trace(PCSTR, ...){};
}

// To remove log support, just define COMPILE_WITHOUT_LOG_SUPPORT
//
#ifdef COMPILE_WITHOUT_LOG_SUPPORT
#define LGTRACE_OPEN()      false
#define LGTRACE             LinearGraph::_no_trace
#define LGFLUSH()           false
#define LGTRACE_CLOSE()     false
#else
#define LGTRACE_OPEN()      LinearGraph::OpenLog()
#define LGTRACE             LinearGraph::Trace
#define LGFLUSH()           LinearGraph::FlushLog()
#define LGTRACE_CLOSE()     LinearGraph::CloseLog()
#endif

#define LGTRACE_OUTOFMEMORY()   LGTRACE("Out of memory")
#define LGTRACE_FUNCTION()      LGTRACE(__FUNCTION__)
