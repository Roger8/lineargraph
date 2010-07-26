
#include "Convert.h"


void BTimeToSystemTime(const BTIME* lpBtime, SYSTEMTIME* lpTime )
{
	unsigned int uiTmp;
	lpTime->wYear = lpBtime->usYear;
	uiTmp = DayToMonth(lpBtime->usYear, lpBtime->usDay);
	lpTime->wMonth = (uiTmp>>16)&0x0000ffff;
	lpTime->wDay = uiTmp&0x0000ffff;
	lpTime->wDayOfWeek = 0;
	lpTime->wHour = lpBtime->ucHours;
	lpTime->wMinute = lpBtime->ucMinutes;
	lpTime->wSecond = lpBtime->ucSeconds;
	lpTime->wMilliseconds = lpBtime->usMiniSec/10;
}

void SystemTimeToBTime(const SYSTEMTIME* lpTime, BTIME* lpBtime)
{
	lpBtime->usMiniSec = lpTime->wMilliseconds*10;
	lpBtime->ucSeconds = (unsigned char)lpTime->wSecond;
	lpBtime->ucMinutes = (unsigned char)lpTime->wMinute;
	lpBtime->ucHours = (unsigned char)lpTime->wHour;
	lpBtime->usDay = MonthToDay(lpTime->wYear, lpTime->wMonth, lpTime->wDay);
	lpBtime->usYear = lpTime->wYear;
	lpBtime->ucUnused = 0;
}

void SystemTimeAddSeconds(const SYSTEMTIME* lpSour, double dSecond, SYSTEMTIME* lpDest)
{
	LARGE_INTEGER int64val;
	FILETIME ft;
	SystemTimeToFileTime(lpSour, &ft);
	int64val.LowPart = ft.dwLowDateTime;
	int64val.HighPart =ft.dwHighDateTime;
	int64val.QuadPart += (__int64)(dSecond * NANO_SECOND_100);
	ft.dwLowDateTime = int64val.LowPart;
	ft.dwHighDateTime = int64val.HighPart;
	FileTimeToSystemTime(&ft, lpDest);
}

void SystemTimeSubSystemTime( const SYSTEMTIME* A, const SYSTEMTIME* B, double* lpDiff )
{
	FILETIME x, y;
	LARGE_INTEGER lx, ly;
	SystemTimeToFileTime(A, &x);
	SystemTimeToFileTime(B, &y);
	lx.LowPart = x.dwLowDateTime;
	lx.HighPart = x.dwHighDateTime;
	ly.LowPart = y.dwLowDateTime;
	ly.HighPart = y.dwHighDateTime;
	*lpDiff = (lx.QuadPart - ly.QuadPart)/(double)NANO_SECOND_100;
}

void BTimeAddSeconds(const BTIME* lpSour, double dSecond, BTIME* lpDest)
{
	SYSTEMTIME time;
	BTimeToSystemTime(lpSour, &time);
	SystemTimeAddSeconds(&time, dSecond, &time);
	SystemTimeToBTime(&time, lpDest);
}

void BTimeSubBTime(const BTIME* A, const BTIME* B, double* lpDiff)
{
	SYSTEMTIME x, y;
	BTimeToSystemTime(A, &x);
	BTimeToSystemTime(B, &y);
	SystemTimeSubSystemTime(&x, &y, lpDiff);
}


//仅对小端格式有效
int CmpSystemTimeF(const SYSTEMTIME* A, const SYSTEMTIME * B)
{
	return memcmp(A, B, sizeof(SYSTEMTIME));
}

int CmpSystemTimeS(const SYSTEMTIME* A, const SYSTEMTIME * B)
{
	int nRet;
	nRet = (int)A->wYear - (int)B->wYear;
	if(nRet == 0)
	{
		nRet = (int)A->wMonth - (int)B->wMonth;
		if(nRet == 0)
		{
			nRet = (int)A->wDay - (int)B->wDay;
			if(nRet == 0)
			{
				nRet = (int)A->wHour - (int)B->wHour;
				if(nRet == 0)
				{
					nRet = (int)A->wMinute - (int)B->wMinute;
					if(nRet == 0)
					{
						nRet = (int)A->wSecond - (int)B->wSecond;
						if(nRet == 0)
						{
							nRet = (int)A->wMilliseconds - (int)B->wMilliseconds;
						}
					}
				}
			}
		}
	}
	return nRet;
}

void SplitNameAndPath( const WCHAR* lpFull, WCHAR* lpName, WCHAR* lpPath )
{
	int n, iLen;

	iLen = wcslen(lpFull);

	for( n = iLen - 1; n >= 0; n--)
	{
		if(lpFull[n] == '\\')
		{
			memcpy(lpPath, lpFull, n*sizeof(WCHAR));
			lpPath[n] = 0;
			wcscpy_s(lpName, iLen-n, &lpFull[n+1]);
			return ;
		}
	}
	lpName = NULL;
	lpPath = NULL;
}

void WcharNameToCharName(char* pchar, int isize, const WCHAR* pwchar, int ilen)
{
	int iwrited;
	iwrited = WideCharToMultiByte(CP_ACP, 0, pwchar, ilen, pchar, isize, NULL, NULL);
	pchar[iwrited] = 0;
}

void SystemTimeAddFraction( const SYSTEMTIME* lpSour, SYSTEMTIME* lpDest, 
						   __int64 numerator,__int64 denominator )
{
	LARGE_INTEGER int64val;
	FILETIME ft;
	SystemTimeToFileTime(lpSour, &ft);
	int64val.LowPart = ft.dwLowDateTime;
	int64val.HighPart =ft.dwHighDateTime;
	int64val.QuadPart += (__int64)(numerator*NANO_SECOND_100)/denominator;
	ft.dwLowDateTime = int64val.LowPart;
	ft.dwHighDateTime = int64val.HighPart;
	FileTimeToSystemTime(&ft, lpDest);
}

void SystemTimeSubSystemTimeFraction( const SYSTEMTIME* A, const SYSTEMTIME* B, 
									 __int64* pnumerator, __int64* pdenominator )
{
	FILETIME x, y;
	LARGE_INTEGER lx, ly;
	SystemTimeToFileTime(A, &x);
	SystemTimeToFileTime(B, &y);
	lx.LowPart = x.dwLowDateTime;
	lx.HighPart = x.dwHighDateTime;
	ly.LowPart = y.dwLowDateTime;
	ly.HighPart = y.dwHighDateTime;
	*pnumerator = (lx.QuadPart - ly.QuadPart)/10000;// *1000/10000000
	*pdenominator = 1000;//返回毫秒值分母
}

void BTimeAddFreq( const BTIME* lpSour, BTIME* lpDest, __int64 numerator, __int64 denominator )
{
	SYSTEMTIME time;
	BTimeToSystemTime(lpSour, &time);
	SystemTimeAddFraction(&time, &time, numerator, denominator);
	SystemTimeToBTime(&time, lpDest);
}

void BTimeSubBTimeFraction( const BTIME* A, const BTIME* B, __int64* pnumerator, __int64* pdenominator )
{
	SYSTEMTIME x, y;
	BTimeToSystemTime(A, &x);
	BTimeToSystemTime(B, &y);
	SystemTimeSubSystemTimeFraction(&x, &y, pnumerator, pdenominator);
}