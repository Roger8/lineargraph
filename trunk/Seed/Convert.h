
#ifndef _CONVERT_H_
#define _CONVERT_H_

#include <Windows.h>
#include "rdseedlib.h"

#define NANO_SECOND			1000000000
#define NANO_SECOND_100		10000000

//时间转换函数
void BTimeToSystemTime(const BTIME* lpBtime, SYSTEMTIME* lpTime);
void SystemTimeToBTime(const SYSTEMTIME* lpTime, BTIME* lpBtime);

void SystemTimeAddSeconds(const SYSTEMTIME* lpSour, double dSecond, SYSTEMTIME* lpDest);
void SystemTimeSubSystemTime(const SYSTEMTIME* A, const SYSTEMTIME* B, double* lpDiff);

void SystemTimeAddFraction(const SYSTEMTIME* lpSour, SYSTEMTIME* lpDest, 
						   __int64 numerator,__int64 denominator);
void SystemTimeSubSystemTimeFraction(const SYSTEMTIME* A, const SYSTEMTIME* B, 
									 __int64* pnumerator, __int64* pdenominator);

void BTimeAddSeconds(const BTIME* lpSour, double dSecond, BTIME* lpDest);
void BTimeSubBTime(const BTIME* A, const BTIME* B, double* lpDiff);

void BTimeAddFreq(const BTIME* lpSour, BTIME* lpDest, 
				  __int64 numerator, __int64 denominator);
void BTimeSubBTimeFraction(const BTIME* A, const BTIME* B, 
						   __int64* pnumerator, __int64* pdenominator);

int CmpSystemTimeF(const SYSTEMTIME* A, const SYSTEMTIME * B);
int CmpSystemTimeS(const SYSTEMTIME* A, const SYSTEMTIME * B);


void SplitNameAndPath(const WCHAR* lpFull, WCHAR* lpName, WCHAR* lpPath);

void WcharNameToCharName(char* pchar, int isize, const WCHAR* pwchar, int ilen);

//分数计算函数
#endif