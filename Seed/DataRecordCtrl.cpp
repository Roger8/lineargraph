
#include <memory.h>
#include <string.h>
#include <math.h>
#include "AnalyzeTools.h"
#include "SwapTimeStrMath.h"
#include "Judge.h"


char* GetDataRecordAddr(char* lpBase, int iSize, int iStep, char* lpStation, char* lpChannel)
{
	int istationLen, ichannelLen;
	FIX_DATA_HEADER* lpFDH;

	//取得data record的地址
	lpFDH = (FIX_DATA_HEADER*)GetFirstDataRecordAddr(lpBase, iSize, iStep);
	if(lpFDH == 0)
		return 0;
	iSize = iSize - ((char*)lpFDH - lpBase);

	istationLen = strlen(lpStation);
	ichannelLen = strlen(lpChannel);

	while(iSize >= iStep)
	{
		if(memcmp(lpFDH->szStationID, lpStation, istationLen) == 0)
		{
			if(memcmp(lpFDH->szChannelID, lpChannel, ichannelLen) == 0)
				return (char*)lpFDH;
		}
		iSize -= iStep;
		lpFDH = (FIX_DATA_HEADER*)((char*)lpFDH + iStep);
	}
	//没有找到
	return 0;
}
//从当前位置，寻找下一个台站分量的数据
char* GetNextDataRecordAddr(char* lpBase, int iSize, int iStep)
{
	return GetDataRecordLen(lpBase, iSize, iStep, 0, 0, 0);
}

//从指定的数据地址开始计算当前同台站分量数据记录的长度
//默认认为开始地址就是数据地址，不做检查
char* GetDataRecordLen(char* lpStart, int iSize, int iStep, int* lpLen, \
					 int iSwap, int* lpTotalCnt)
{
	char szStation[6];
	char szChannel[4];
	FIX_DATA_HEADER* lpFDH;
	int stationLen, channelLen;
	int iSampleCnt, iLen;

	lpFDH = (FIX_DATA_HEADER*) lpStart;
	strcpy_ULN(szStation, lpFDH->szStationID, 5);
	strcpy_ULN(szChannel, lpFDH->szChannelID, 3);

	if(iSwap)
		iSampleCnt = SWAP2(lpFDH->usSampleNum);
	else
		iSampleCnt = lpFDH->usSampleNum;
	
	stationLen = strlen(szStation);
	channelLen = strlen(szChannel);
	iSize -= iStep;
	lpFDH = (FIX_DATA_HEADER*)((char*)lpFDH + iStep);

	while( iSize >= iStep)
	{
		if(memcmp(lpFDH->szStationID, szStation, stationLen) != 0)
			break;
		if(memcmp(lpFDH->szChannelID, szChannel, channelLen) != 0)
			break;
		
		//count the samples
		if(iSwap)
			iSampleCnt += SWAP2(lpFDH->usSampleNum);
		else
			iSampleCnt += lpFDH->usSampleNum;

		iSize -= iStep;
		lpFDH = (FIX_DATA_HEADER*)((char*)lpFDH + iStep);
	}

	if(lpTotalCnt != 0)
		*lpTotalCnt = iSampleCnt;

	iLen = ((char*)lpFDH - lpStart);
	//返回数据记录的长度
	if(lpLen != 0)
		*lpLen = iLen;
	//如果剩下的数据不足一个STEP步长，认为没有数据量
	if(iLen + iStep > iSize)
		return NULL;
	return lpStart + iLen;
}

char* LocateRecordByTime(char* lpBase, int iSize, int iStep, int iSwap, BTIME* lptime)
{
	int nSample, iLen;
	double freq;
	FIX_DATA_HEADER* lpFDH;
	char* lpMax;
	char* lpPre;
	BTIME time;

	lpFDH = (FIX_DATA_HEADER*)lpBase;
	lpPre = 0;
	GetDataRecordLen(lpBase, iSize, iStep, &iLen, 0, 0);
	lpMax = lpBase + iLen - iStep;
	
	while((char*)lpFDH <= lpMax)
	{
		memcpy(&time, &lpFDH->tStart, sizeof(BTIME));
		nSample = lpFDH->usSampleNum;
		if(iSwap)
		{
			time.usYear = SWAP2(time.usYear);
			time.usDay = SWAP2(time.usDay);
			time.usMiniSec = SWAP2(time.usMiniSec);
			nSample = SWAP2(nSample);
		}
		if(CmpBTIMEAS(lptime, &time) >= 0)
		{
			int numerator, denominator;
			if(0 == GetRecordRate(lpFDH, iSwap, &numerator, &denominator))
				return 0;
			freq = ((float)numerator)/((float)denominator);
			//计算最后一个点的时间
			BTimeAddLen_L(&time, (long long)((10000.0/freq) * (nSample-1)));
			if(CmpBTIMEAS(lptime, &time) <= 0 )
				return (char*)lpFDH;
		}
		else
		{
			return lpPre;
// 			if(lpPre == 0)//起始时间在数据开始时间之前
// 				return (char*)lpFDH;
// 			return lpPre;
		}
		//计算下一块
		lpPre = (char*)lpFDH;
		lpFDH = (FIX_DATA_HEADER*)((char*)lpFDH + iStep);
	}
	return 0;
// 	//比数据时间还要晚，返回最后一块
// 	return (char*)lpMax;
}


int GetRecordRate(FIX_DATA_HEADER* lpFDH, int iSwap, int* lpNumerator, int* lpDenominator)
{
	short sRateFactor, sRateMultiplier;

	//计算采样率
	sRateFactor = lpFDH->sRateFactor;
	sRateMultiplier = lpFDH->sRateMultiplier;
	if(iSwap)
	{
		sRateFactor = SWAP2(sRateFactor);
		sRateMultiplier = SWAP2(sRateMultiplier);
	}
	//return the numerator and the denominator
	if(sRateFactor > 0 && sRateMultiplier > 0)
	{
		*lpNumerator = (sRateFactor*sRateMultiplier);
		*lpDenominator = 1;
	}
	else if(sRateFactor > 0 && sRateMultiplier < 0) 
	{
		*lpNumerator = abs(sRateFactor);
		*lpDenominator = abs(sRateMultiplier);
	}
	else if(sRateFactor < 0 && sRateMultiplier > 0)
	{
		*lpNumerator = abs(sRateMultiplier);
		*lpDenominator = abs(sRateFactor);
	}
	else if(sRateFactor < 0 && sRateMultiplier < 0)
	{
		*lpNumerator = 1;
		*lpDenominator = sRateFactor * sRateMultiplier;
	}
	else /* deal the data when iRateFactor is 0 */
	{
		*lpNumerator = 0;
		*lpDenominator = 0;
		return 0;
	}
	return 1;
}

void GetRecordBTime(FIX_DATA_HEADER* lpFDH, int iSwap, BTIME* lpTime)
{
	memcpy(lpTime, &lpFDH->tStart, sizeof(BTIME));
	if(iSwap)
	{
		lpTime->usYear = SWAP2(lpTime->usYear);
		lpTime->usDay = SWAP2(lpTime->usDay);
		lpTime->usMiniSec = SWAP2(lpTime->usMiniSec);
	}
	lpTime->ucUnused = 0;
}

void GetRecordSampleCnt( FIX_DATA_HEADER* lpFDH, int iSwap, int* lpcnt )
{
	if(iSwap)
		*lpcnt = SWAP2(lpFDH->usSampleNum);
	else
		*lpcnt = lpFDH->usSampleNum;
}

//取得指定record的属性
int GetRecordInfo(FIX_DATA_HEADER* lpFDH, int iStep, int iSwap, int* lpNumerator,
				  int* lpDenominator, BTIME* lpStart, BTIME* lpEnd, int* lpCnt)
{
	int numerator, denominator;
	int ret = 1;

	if( !IsValidFixDataHeader(lpFDH, iStep) )
		return 0;
	//计算采样率
	if(GetRecordRate(lpFDH, iSwap, &numerator, &denominator) == 0)
	{
		numerator = 0;
		denominator = 0;
		ret = 0;
	}
	if(lpNumerator != 0)
		*lpNumerator = numerator;
	if(lpDenominator != 0)
		*lpDenominator = denominator;
	//装填起始时间，计算终止时间，就是最后一个采样点的下一个点对应的时间
	BTIME tStart;
	memcpy(&tStart, &lpFDH->tStart, sizeof(BTIME));
	tStart.ucUnused = 0;
	if(iSwap)
	{
		tStart.usYear = SWAP2(tStart.usYear);
		tStart.usDay = SWAP2(tStart.usDay);
		tStart.usMiniSec = SWAP2(tStart.usMiniSec);
	}
	//装填起始时间
	if(lpStart != 0)
		memcpy(lpStart, &tStart, sizeof(BTIME));
	//计算点数
	unsigned short usSample;
	usSample = lpFDH->usSampleNum;
	if(iSwap)
		usSample = SWAP2(usSample);
	if(lpCnt != 0)
		*lpCnt = usSample;
	//计算终止时间
	if(lpEnd != 0)
	{
		memcpy(lpEnd, &tStart, sizeof(BTIME));
		if(numerator > 0 && denominator > 0)//有错误，不校正时间
		{
			long long llOffset;
			llOffset = (long long)(10000*usSample*denominator/numerator);
			BTimeAddLen_L(lpEnd, llOffset);
		}
	}
	return ret;
}

char* Get1000Block(FIX_DATA_HEADER* lpFDH, int iStep, int iSwap)
{
	unsigned short* lpBlock;
	unsigned short usOffset, usType;

	usOffset = lpFDH->usBlocketteOffset;
	if(iSwap)
		usOffset = SWAP2(usOffset);
	while(1)
	{
		if(usOffset == 0 || usOffset > iStep)
			break;
		lpBlock = (unsigned short*)(((char*)lpFDH) + usOffset);
		usType = lpBlock[0];
		usOffset = lpBlock[1];
		if(iSwap)
		{
			usType = SWAP2(usType);
			usOffset = SWAP2(usOffset);
		}
		//找到1000块
		if(usType == 1000)
			return (char*)lpBlock;
	}
	return 0;
}

//得到数据的属性
int GetDataAttrib(FIX_DATA_HEADER* lpFDH, int iStep, int iSwap, int* lpFormat, 
				  int* lpOrder, int* lpLen)
{
	unsigned short * lpBlock;

	lpBlock = (unsigned short*)Get1000Block(lpFDH, iStep, iSwap);
	if(lpBlock != 0)
	{
		char * lpChar;
		lpChar = (char*)lpBlock;
		if(lpFormat != 0)
			*lpFormat = (int)lpChar[4];
		if(lpOrder != 0)
			*lpOrder = (int)lpChar[5];
		if(lpLen != 0)
			*lpLen = (int)lpChar[6];
		return 1;
	}
	else
		return 0;
}



int AnalyzeStationInfo( char* lpBase, int iSize, int iStep, int iSwap, 
					   char** lpNext, int* lpLen, BTIME* lpstart, BTIME* lpend, 
					   int* lpcnt, int* lplost, int* pnumerator, int* pdenominator )
{
	char szStation[6];
	char szChannel[4];
	int ileft, stationlen, channellen;
	BTIME gend;
	int cnt, lost, numerator, denominator;
	BTIME tstart, tend;
	int loccnt, locnumerator, locdenominator;
	FIX_DATA_HEADER* lpFDH;

	ileft = iSize;
	if(ileft < iStep )
		return 0;
	if( !IsValidFixDataHeader(lpBase, iStep) )
		return 0;

	lpFDH = (FIX_DATA_HEADER*)lpBase;
	strcpy_ULN(szStation, lpFDH->szStationID, 5);
	strcpy_ULN(szChannel, lpFDH->szChannelID, 3);
	stationlen = strlen(szStation);
	channellen = strlen(szChannel);
	if(0 == GetRecordInfo(lpFDH, iStep, iSwap, &locnumerator, &locdenominator, 
		&tstart, &tend, &loccnt))//频率出现重大错误，直接返回
		return 0;
	//准备起始数据
	cnt = loccnt;
	lost = 0;
	memcpy(&gend, &tend, sizeof(BTIME));
	numerator = locnumerator;
	denominator = locdenominator;
	if(lpstart != 0)
		memcpy(lpstart, &tstart, sizeof(BTIME));

	long long diff;
	int tmp;
	ileft -= iStep;
	lpFDH = (FIX_DATA_HEADER*)(((char*)lpFDH) + iStep);
	while(ileft >= iStep)
	{
		if(memcmp(lpFDH->szStationID, szStation, stationlen) != 0)
			break;
		if(memcmp(lpFDH->szChannelID, szChannel, channellen) != 0)
			break;
		if(GetRecordInfo(lpFDH, iStep, iSwap, &locnumerator, &locdenominator, 
			&tstart, &tend, &loccnt) == 0)//频率出现重大错误，直接返回
			return 0;
		//计算缺数或者多数情况
		BTimeSubBTime_L(&gend, &tstart, &diff);
		//按照后面的频率计算缺数或者多数情况
		tmp = (int)((diff*locnumerator)/(((long long)locdenominator)*10000));
		lost += tmp;
		cnt += loccnt;
		memcpy(&gend, &tend, sizeof(BTIME));
		if(numerator*locdenominator - denominator*locnumerator < 0)
		{//返回最大频率
			numerator = locnumerator;
			denominator = locdenominator;
		}
		//next parameter 
		ileft -= iStep;
		lpFDH = (FIX_DATA_HEADER*)(((char*)lpFDH) + iStep);
	}
	if(lpNext != 0)
	{
		if(ileft < iStep)
			*lpNext = 0;
		else
			*lpNext = (char*)lpFDH;
	}
	if(lpLen != 0)
	{
		*lpLen = (char*)lpFDH - lpBase;
	}
	if(lpend != 0)
	{
		BTimeAddLen_L(&gend, denominator*(-10000)/numerator);//减去一个数据点
		memcpy(lpend, &gend, sizeof(BTIME));
	}
	if(lpcnt != 0)
		*lpcnt = cnt;
	if(lplost != 0)
		*lplost = lost;
	if(pnumerator != 0)
		*pnumerator = numerator;
	if(pdenominator != 0)
		*pdenominator = denominator;
	return 1;
}

