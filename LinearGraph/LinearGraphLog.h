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
    void Error(PCSTR formatString, ...);
    bool FlushLog();
    bool CloseLog();

    inline void _no_trace(PCSTR, ...){};
}

// To remove log support, just define COMPILE_WITHOUT_LOG_SUPPORT
//
#ifdef COMPILE_WITHOUT_LOG_SUPPORT
#define LG_OPEN_LOG()      false
#define LG_TRACE           LinearGraph::_no_trace
#define LG_ERROR           LinearGraph::_no_trace
#define LG_FLUSH_LOG()     false
#define LG_CLOSE_LOG()     false
#else
#define LG_OPEN_LOG()      LinearGraph::OpenLog()
#define LG_TRACE           LinearGraph::Trace
#define LG_ERROR           LinearGraph::Error
#define LG_FLUSH_LOG()     LinearGraph::FlushLog()
#define LG_CLOSE_LOG()     LinearGraph::CloseLog()
#endif

#define LG_TRACE_FUNCTION()      LG_TRACE(__FUNCTION__)
