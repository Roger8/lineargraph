
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

	//ȡ��data record�ĵ�ַ
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
	//û���ҵ�
	return 0;
}
//�ӵ�ǰλ�ã�Ѱ����һ��̨վ����������
char* GetNextDataRecordAddr(char* lpBase, int iSize, int iStep)
{
	return GetDataRecordLen(lpBase, iSize, iStep, 0, 0, 0);
}

//��ָ�������ݵ�ַ��ʼ���㵱ǰ̨ͬվ�������ݼ�¼�ĳ���
//Ĭ����Ϊ��ʼ��ַ�������ݵ�ַ���������
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
	//�������ݼ�¼�ĳ���
	if(lpLen != 0)
		*lpLen = iLen;
	//���ʣ�µ����ݲ���һ��STEP��������Ϊû��������
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
			//�������һ�����ʱ��
			BTimeAddLen_L(&time, (long long)((10000.0/freq) * (nSample-1)));
			if(CmpBTIMEAS(lptime, &time) <= 0 )
				return (char*)lpFDH;
		}
		else
		{
			return lpPre;
// 			if(lpPre == 0)//��ʼʱ�������ݿ�ʼʱ��֮ǰ
// 				return (char*)lpFDH;
// 			return lpPre;
		}
		//������һ��
		lpPre = (char*)lpFDH;
		lpFDH = (FIX_DATA_HEADER*)((char*)lpFDH + iStep);
	}
	return 0;
// 	//������ʱ�仹Ҫ���������һ��
// 	return (char*)lpMax;
}


int GetRecordRate(FIX_DATA_HEADER* lpFDH, int iSwap, int* lpNumerator, int* lpDenominator)
{
	short sRateFactor, sRateMultiplier;

	//���������
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

//ȡ��ָ��record������
int GetRecordInfo(FIX_DATA_HEADER* lpFDH, int iStep, int iSwap, int* lpNumerator,
				  int* lpDenominator, BTIME* lpStart, BTIME* lpEnd, int* lpCnt)
{
	int numerator, denominator;
	int ret = 1;

	if( !IsValidFixDataHeader(lpFDH, iStep) )
		return 0;
	//���������
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
	//װ����ʼʱ�䣬������ֹʱ�䣬�������һ�����������һ�����Ӧ��ʱ��
	BTIME tStart;
	memcpy(&tStart, &lpFDH->tStart, sizeof(BTIME));
	tStart.ucUnused = 0;
	if(iSwap)
	{
		tStart.usYear = SWAP2(tStart.usYear);
		tStart.usDay = SWAP2(tStart.usDay);
		tStart.usMiniSec = SWAP2(tStart.usMiniSec);
	}
	//װ����ʼʱ��
	if(lpStart != 0)
		memcpy(lpStart, &tStart, sizeof(BTIME));
	//�������
	unsigned short usSample;
	usSample = lpFDH->usSampleNum;
	if(iSwap)
		usSample = SWAP2(usSample);
	if(lpCnt != 0)
		*lpCnt = usSample;
	//������ֹʱ��
	if(lpEnd != 0)
	{
		memcpy(lpEnd, &tStart, sizeof(BTIME));
		if(numerator > 0 && denominator > 0)//�д��󣬲�У��ʱ��
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
		//�ҵ�1000��
		if(usType == 1000)
			return (char*)lpBlock;
	}
	return 0;
}

//�õ����ݵ�����
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
		&tstart, &tend, &loccnt))//Ƶ�ʳ����ش����ֱ�ӷ���
		return 0;
	//׼����ʼ����
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
			&tstart, &tend, &loccnt) == 0)//Ƶ�ʳ����ش����ֱ�ӷ���
			return 0;
		//����ȱ�����߶������
		BTimeSubBTime_L(&gend, &tstart, &diff);
		//���պ����Ƶ�ʼ���ȱ�����߶������
		tmp = (int)((diff*locnumerator)/(((long long)locdenominator)*10000));
		lost += tmp;
		cnt += loccnt;
		memcpy(&gend, &tend, sizeof(BTIME));
		if(numerator*locdenominator - denominator*locnumerator < 0)
		{//�������Ƶ��
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
		BTimeAddLen_L(&gend, denominator*(-10000)/numerator);//��ȥһ�����ݵ�
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

