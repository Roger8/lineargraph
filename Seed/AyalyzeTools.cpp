
#include "AnalyzeTools.h"
#include "SwapTimeStrMath.h"
#include "ExtractBlockette.h"
#include "Judge.h"

int IsValidHeaderCode(void* lpBase)
{
	int i;
	char* lpHeader;
	lpHeader = (char*)lpBase;

	for( i = 0; i < 6; i++ )
		if( ! IS_DIGIT(lpHeader[i]) )
			return 0;

	if( ! IS_HEADER_CHAR(lpHeader[6]) )
		return 0;

	if( ! IS_CONTINUE_CODE(lpHeader[7]) )
		return 0;
	return 1;
}

int IsValidFixDataHeader(void* lpBase, int iStep)
{
	char* lpHeader;
	int iError;
	FIX_DATA_HEADER* lpFDH;
	unsigned short usTmp;
	lpFDH = (FIX_DATA_HEADER*) lpBase;
	lpHeader = (char*)lpBase;

	if( ! IsValidHeaderCode(lpBase) )
		return 0;
	//校验数据record的特定字符
	if( ! IS_DATA_RECORD_CHAR(lpHeader[6]) )
		return 0;
	//校验台站名
	if( ! IsUN_NAME(lpFDH->szStationID, sizeof(lpFDH->szStationID) + \
										sizeof(lpFDH->szLocationID) + \
										sizeof(lpFDH->szChannelID) + \
										sizeof(lpFDH->szNetworkCode)) )
		return 0;
	//校验时间
	if( ! IsValidBTime(&lpFDH->tStart, 0) )
		return 0;
	//校验偏移
	iError = 0;

	if(lpFDH->usDataOffset > iStep || lpFDH->usBlocketteOffset > iStep )
		iError = 1;

	if(iError == 1)
	{
		usTmp = SWAP2(lpFDH->usDataOffset);
		if(usTmp > iStep)
			return 0;
		usTmp = SWAP2(lpFDH->usBlocketteOffset);
		if(usTmp > iStep)
			return 0;
	}
	//检查频率
	if(lpFDH->sRateFactor == 0 || lpFDH->sRateMultiplier == 0)
		return 0;
	
	return 1;
}

int IsValidDataRecord(void* lpBase, int iStep)
{
	FIX_DATA_HEADER * lpFDH;
	unsigned short* lpBlock;
	char* lpChar;
	unsigned short usfirst, ussecond;

	lpChar = (char*) lpBase;
	lpBlock = (unsigned short*) lpBase;
	lpFDH = (FIX_DATA_HEADER*) lpBase;

	
	if( lpFDH->usDataOffset < iStep && lpFDH->usBlocketteOffset+4 < iStep )
	{//假设不用调换字节顺序
		usfirst = lpFDH->usBlocketteOffset;
		while(1)
		{
			//如果没有后续blockette，+4保证不会内存访问违规
			if(usfirst == 0 || usfirst+4 > iStep)
				break;
			lpBlock = (unsigned short*)(lpChar + usfirst);
			ussecond = lpBlock[0];
			if(ussecond == 1000)
				return 1;
			usfirst = lpBlock[1];
		}
	}
	//假设交换字节顺序
	usfirst = SWAP2(lpFDH->usBlocketteOffset);
	ussecond = SWAP2(lpFDH->usDataOffset);

	if(usfirst+4 < iStep && ussecond < iStep)
	{
		while(1)
		{
			if(usfirst == 0 || usfirst+4 > iStep)
				break;
			lpBlock = (unsigned short*)(lpChar + usfirst);
			ussecond = SWAP2(lpBlock[0]);
			if( ussecond == 1000 )
				return 1;
			usfirst = SWAP2(lpBlock[1]);
		}
	}
	return 0;
}


int IsValidSeed(void* lpBase, int iSize)
{
	int iStep, iLeft;
	char * lpChar;
	//校验控制头序列
	if( ! IsValidHeaderCode(lpBase))
		return 0;
	//校验步长
	iStep = GetRecordStep(lpBase, iSize);
	if(iStep == 0)
		return 0;
	//校验数据记录块
	lpChar = GetFirstDataRecordAddr((char*)lpBase, iSize, iStep);
	if(lpChar != 0)
	{
		iLeft = iSize - (lpChar - (char*)lpBase);
		if( ! IsMiniSeed(lpChar, iLeft))
			return 0;
	}
	return 1;
}
int IsMiniSeed(void* lpBase, int iSize)
{
	int iStep;
	iStep = GetRecordStep(lpBase, iSize);
	if(iStep == 0)
		return 0;
	if( ! IsValidFixDataHeader(lpBase, iStep))
		return 0;
	//为了解压早期标准的seed文件，放弃使用1000块校验
// 	if( ! IsValidDataRecord(lpBase, iStep))//校验mini-seed中的1000块
// 		return 0;
	return 1;
}

int GetStepFromSeedBlock(void* lpBase, int iSize)
{
	int n;
	char* lpBlock;
	lpBlock = (char*)FindBlockInControlHeader((char*)lpBase, iSize, iSize, 5);
	if(lpBlock == 0)
	{
		lpBlock = (char*)FindBlockInControlHeader((char*)lpBase, iSize, iSize, 8);
		if(lpBlock == 0)
		{
			lpBlock = (char*)FindBlockInControlHeader((char*)lpBase, iSize, iSize, 11);
			if(lpBlock == 0)
				return 0;
		}
	}
	lpBlock += 11;
	n = STR2TOINT(lpBlock);
	return (1<<n);
}

int GetRecordStep(void* lpBase, int iSize)
{
	int iStep, icnt, iType, iLeft;
	char* lpChar;

	lpChar = (char*)lpBase;
	if( iSize < 256)
		return 0;

	//从文件中提取record的大小
	if(lpChar[6] == 'V')
	{
		lpChar += 8;
		iType = STR3TOINT(lpChar);
		if(iType == 5 || iType == 8 || iType == 10)
		{
			lpChar += 3+4+4;
			iStep = STR2TOINT(lpChar);
			if(iStep >= 8 && iStep <= 15)
				return (1<<iStep);
		}
	}
	//record记录的长度为256-32768，每个step试一下
	for( iStep = 256; iStep <= 32768; iStep<<=1)
	{
		lpChar = (char*)lpBase;
		iLeft = iSize;
		//测试X块record
		for(icnt = 0; icnt < 4; icnt++)
		{
			if(iLeft < iStep)
				return iStep;
			if( ! IsValidHeaderCode((void*)lpChar) )
				break;
			lpChar += iStep;
			iLeft -= iStep;
		}
		if(icnt == 4)
			return iStep;
	}
	return 0;
}

//这里假设文件就是SEED文件，不对文件作校验处理
int AnalyzeSeedFile(void* lpBase, int iSize, int* lpSwap, int* lpStep)
{
	char * lpChar;
	int iStep;

	lpChar = (char*)lpBase;
	if(IS_DATA_RECORD_CHAR(lpChar[6]))
	{//mini-seed文件，首先的到文件大小端
		if(0 == GetSwapFlag(lpBase, iSize, lpSwap))
			return 0;
		lpChar = Get1000Block((FIX_DATA_HEADER*)lpBase, iSize, *lpSwap);
		if(0 == lpChar)
		{//bad seed file, get the step by trying
			*lpStep = GetRecordStep(lpBase, iSize);
			if(*lpStep == 0)
				return 0;
			else
				return 1;
		}
		*lpStep = POWER2(lpChar[6]);
		return 1;
	}
	//else seed file with the control header 
	//取得步长
	iStep = GetRecordStep(lpBase, iSize);
	if(iStep == 0)
		return 0;//出现这样的错误，没有必要在进行下去
	*lpStep = iStep;
	
	*lpSwap = 0;//默认不用交换
	//取得交换字节顺序
	lpChar = GetFirstDataRecordAddr((char*)lpBase, iSize, iStep);
	if(lpChar == 0)//no data record 
		return 0;
	if(0 == GetSwapFlag(lpChar, iStep, lpSwap))
		return 0;
	return 1;

}

char* GetFirstDataRecordAddr(char* lpBase, int iSize, int iStep)
{
	int iLeft;
	iLeft = iSize;
	while(iLeft >= iStep)
	{
		if( IS_DATA_RECORD_CHAR(lpBase[6]))
			return lpBase;
		lpBase += iStep;
		iLeft -= iStep;
	}
	return (char*)0;
}

int GetSwapFlag(void* lpFDH, int isize, int* pSwap)
{
	FIX_DATA_HEADER* lpB;
	unsigned short ustmp;
	int iswap, itmp;
	if(isize < sizeof(FIX_DATA_HEADER))
		return 0;
	iswap = 0;
	lpB = (FIX_DATA_HEADER*)lpFDH;
	ustmp = lpB->usDataOffset;
	if(ustmp >= isize || ustmp >= MAX_SEED_RECORD_LEN || ustmp < sizeof(FIX_DATA_HEADER))
	{
		iswap = 1;
		ustmp = SWAP2(ustmp);
		if(ustmp >= isize || ustmp >= MAX_SEED_RECORD_LEN || ustmp < sizeof(FIX_DATA_HEADER) || 
			(ustmp > 0 && ustmp < sizeof(FIX_DATA_HEADER)) )
			return 0;
	}
	ustmp = lpB->usBlocketteOffset;
	if(ustmp >= isize || ustmp >= MAX_SEED_RECORD_LEN || ustmp < sizeof(FIX_DATA_HEADER))
	{
		iswap = 1;
		ustmp = SWAP2(ustmp);
		if(ustmp >= isize || ustmp >= MAX_SEED_RECORD_LEN || 
			(ustmp > 0 && ustmp < sizeof(FIX_DATA_HEADER)) )
			return 0;
	}
	if(IsValidBTime(&lpB->tStart, &itmp) == 0)
		return 0;//something wrong 
	if(itmp == 1)//时间检测未必正确
		iswap = 1;
	*pSwap = iswap;
	return 1;
}
