#include "LinearGraph.h"
#include "LinearGraphLog.h"
#include <fstream>
#include <stdarg.h>
#include <time.h>

static char _timeStringBuff[128] = {0};
const char* getCurrentTime()
{
    SYSTEMTIME stNow = {0};
    ::GetLocalTime(&stNow);

    ::sprintf(_timeStringBuff, "[%04d-%02d-%02d %02d:%02d:%02d.%03d] ",
        (DWORD)stNow.wYear, (DWORD)stNow.wMonth, (DWORD)stNow.wDay,
        (DWORD)stNow.wHour, (DWORD)stNow.wMinute,
        (DWORD)stNow.wSecond, (DWORD)stNow.wMilliseconds);
    return _timeStringBuff;
}

static std::ofstream _logFile;
static HANDLE        _logLock = 0;

static inline void lockLog()
{
    ::WaitForSingleObject(_logLock, INFINITE);
}

static inline void freeLog()
{
    ::ReleaseMutex(_logLock);
}

bool LinearGraph::OpenLog()
{
    if( !(_logLock = ::CreateMutex(0, FALSE, 0)) )
    {
        return false;
    }

    SYSTEMTIME sysTime;
    ::GetLocalTime(&sysTime);

    CString logPath = CApplication::GetAppDirectory();
    logPath.Append(L"log");
    if( !::CreateDirectoryW(logPath, 0) && ERROR_ALREADY_EXISTS != ::GetLastError() )
    {
        return false;
    }

    logPath.AppendFormat(L"\\%04d-%02d-%02d.log",
        (DWORD)sysTime.wYear, (DWORD)sysTime.wMonth, (DWORD)sysTime.wDay);
    _logFile.open(logPath, std::ios::app|std::ios::out);
    if( !_logFile.is_open() )
    {
        CloseHandle(_logLock);
        _logLock = 0;
        return false;
    }
    return true;
}

void LinearGraph::Trace(PCSTR formatString, ...)
{
    static char buff[4096];
    if( _logLock )
    {
        lockLog();

        va_list args;
        va_start(args, formatString);
        vsnprintf_s(buff, 4096, 4094, formatString, args);
        va_end(args);

        _logFile << getCurrentTime() << buff << '\n';
        freeLog();
    }
}

void LinearGraph::Error(PCSTR formatString, ...)
{
    static char buff[4096];
    if( _logLock )
    {
        lockLog();

        va_list args;
        va_start(args, formatString);
        vsnprintf_s(buff, 4096, 4094, formatString, args);
        va_end(args);

        _logFile << getCurrentTime() << "ERROR " << buff << '\n';

        char* pErrorText = 0;
        ::FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
            NULL, ::GetLastError(), LANG_CHINESE_SIMPLIFIED, (LPSTR)&pErrorText, 0, NULL);
        if( pErrorText )
        {
            _logFile << "Last Win32 error message [" << pErrorText << "]\n";
        }

        _logFile << std::flush;
        freeLog();
    }
}

bool LinearGraph::CloseLog()
{
    if( !_logLock ){ return false; }

    lockLog();
    _logFile.close();
    freeLog();
    ::CloseHandle(_logLock);
    _logLock = 0;
    return true;
}

bool LinearGraph::FlushLog()
{
    if( !_logLock ){ return false; }

    lockLog();
    _logFile << std::flush;
    freeLog();
    return true;
}
