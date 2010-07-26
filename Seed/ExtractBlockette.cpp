

#include "Judge.h"
#include "SwapTimeStrMath.h"
#include "ExtractBlockette.h"
#include "HeaderBlockStruct.h"

int IsValidIndexBlockCode( int code )
{
	if(code == 5 || code == 8 || code == 10 || code == 11 || code == 12)
		return 1;
	else
		return 0;
}

int IsValidDictionaryBlockCode( int code )
{
	if(code >= 30 && code <= 35 )
		return 1;
	if(code >= 41 && code <= 49)
		return 1;
	return 0;
}

int IsValidStationBlockCode( int code )
{
	if(code >= 50 && code <= 62)
		return 1;
	else
		return 0;
}

int IsValidTimespanBlockCode( int code )
{
	if(code >= 70 && code <= 74)
		return 1;
	else
		return 0;
}

int IsValidBlockCode( int code )
{
	if(IsValidIndexBlockCode(code))
		return 1;
	if(IsValidDictionaryBlockCode(code))
		return 1;
	if(IsValidStationBlockCode(code ))
		return 1;
	if(IsValidTimespanBlockCode(code ))
		return 1;
	return 0;
}

void* LocateHeaderAddr( char* lpBase, int iStep, int iSize, int iType )
{
	int iLeft;
	if(iType != INDEX_SEED_HEADER_CODE && iType != DICTIONARY_SEED_HEADER_CODE &&
		iType != STATION_SEED_HEADER_CODE && iType != TIMESPAN_SEED_HEADER_CODE)
		return 0;
	iLeft = iSize;
	while(iLeft >= iStep)
	{
		if( lpBase[6] == iType)
			return (void*)lpBase;
		lpBase += iStep;
		iLeft -= iStep;
	}
	return 0;
}

void* LocateFirstDataRecord( char* lpBase, int iStep, int iSize )
{
	int iLeft;
	iLeft = iSize;
	while(iLeft >= iStep)
	{
		if( IS_DATA_RECORD_CHAR(lpBase[6]))
			return (void*)lpBase;
		lpBase += iStep;
		iLeft -= iStep;
	}
	return 0;

}

void* FindBlockInOneRecord( char* lpRecord, int iStep, int ID, int iGnore, int* iLeft )
{
	int aleft;
	int iLen, itype;
	char* lpTmp;
	//set the initial operate value 
	aleft = iStep - 8;
	if(iGnore >= aleft)
	{
		*iLeft = iGnore - aleft;
		return 0;
	}
	aleft -= iGnore;
	lpTmp = lpRecord + 8 + iGnore;
	while (aleft >= 3+4)
	{
		itype = STR3TOINT(lpTmp);
		if(itype == ID)
			return lpTmp;
		lpTmp += 3;
		iLen = STR4TOINT(lpTmp);
		if(iLen >= aleft)
		{
			*iLeft = iLen - aleft;
			return 0;
		}
		aleft -= iLen;
		lpTmp += iLen - 3;
	}
	*iLeft = 0;
	return 0;
}

void* FindBlockInControlHeader(char* lpBaseH, int iStep, int iSize, int ID)
{
	int ignore, icnt;
	char* lpDest;
	char headerType;
	ignore = 0;
	icnt = iSize/iStep;
	headerType = lpBaseH[6];//remember the first record type 
	do {
		lpDest = (char*)FindBlockInOneRecord(lpBaseH, iStep, ID, ignore, &ignore);
		if(lpDest != 0)
			return lpDest;
		lpBaseH += iStep;
		icnt --;
		if(icnt <= 0)
			break;
	}while( lpBaseH[6] == headerType);

	return 0;
}

void* LocateBlockette( char* lpBase, int iStep, int iSize, int ID )
{
	char* lpTmp;
	if (IsValidIndexBlockCode(ID))
		lpTmp = (char*)LocateHeaderAddr(lpBase, iStep, iSize, INDEX_SEED_HEADER_CODE);
	else if (IsValidDictionaryBlockCode(ID))
		lpTmp = (char*)LocateHeaderAddr(lpBase, iStep, iSize, DICTIONARY_SEED_HEADER_CODE);
	else if (IsValidStationBlockCode(ID))
		lpTmp = (char*)LocateHeaderAddr(lpBase, iStep, iSize, STATION_SEED_HEADER_CODE);
	else if (IsValidTimespanBlockCode(ID))
		lpTmp = (char*)LocateHeaderAddr(lpBase, iStep, iSize, TIMESPAN_SEED_HEADER_CODE);
	else
		lpTmp = 0;
	if(lpTmp == 0)
		return 0;
	return FindBlockInControlHeader(lpTmp, iStep, (iSize - (lpTmp-lpBase)), ID);
}
