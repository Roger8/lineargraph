
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
	//У������record���ض��ַ�
	if( ! IS_DATA_RECORD_CHAR(lpHeader[6]) )
		return 0;
	//У��̨վ��
	if( ! IsUN_NAME(lpFDH->szStationID, sizeof(lpFDH->szStationID) + \
										sizeof(lpFDH->szLocationID) + \
										sizeof(lpFDH->szChannelID) + \
										sizeof(lpFDH->szNetworkCode)) )
		return 0;
	//У��ʱ��
	if( ! IsValidBTime(&lpFDH->tStart, 0) )
		return 0;
	//У��ƫ��
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
	//���Ƶ��
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
	{//���費�õ����ֽ�˳��
		usfirst = lpFDH->usBlocketteOffset;
		while(1)
		{
			//���û�к���blockette��+4��֤�����ڴ����Υ��
			if(usfirst == 0 || usfirst+4 > iStep)
				break;
			lpBlock = (unsigned short*)(lpChar + usfirst);
			ussecond = lpBlock[0];
			if(ussecond == 1000)
				return 1;
			usfirst = lpBlock[1];
		}
	}
	//���轻���ֽ�˳��
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
	//У�����ͷ����
	if( ! IsValidHeaderCode(lpBase))
		return 0;
	//У�鲽��
	iStep = GetRecordStep(lpBase, iSize);
	if(iStep == 0)
		return 0;
	//У�����ݼ�¼��
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
	//Ϊ�˽�ѹ���ڱ�׼��seed�ļ�������ʹ��1000��У��
// 	if( ! IsValidDataRecord(lpBase, iStep))//У��mini-seed�е�1000��
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

	//���ļ�����ȡrecord�Ĵ�С
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
	//record��¼�ĳ���Ϊ256-32768��ÿ��step��һ��
	for( iStep = 256; iStep <= 32768; iStep<<=1)
	{
		lpChar = (char*)lpBase;
		iLeft = iSize;
		//����X��record
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

//��������ļ�����SEED�ļ��������ļ���У�鴦��
int AnalyzeSeedFile(void* lpBase, int iSize, int* lpSwap, int* lpStep)
{
	char * lpChar;
	int iStep;

	lpChar = (char*)lpBase;
	if(IS_DATA_RECORD_CHAR(lpChar[6]))
	{//mini-seed�ļ������ȵĵ��ļ���С��
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
	//ȡ�ò���
	iStep = GetRecordStep(lpBase, iSize);
	if(iStep == 0)
		return 0;//���������Ĵ���û�б�Ҫ�ڽ�����ȥ
	*lpStep = iStep;
	
	*lpSwap = 0;//Ĭ�ϲ��ý���
	//ȡ�ý����ֽ�˳��
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
	if(itmp == 1)//ʱ����δ����ȷ
		iswap = 1;
	*pSwap = iswap;
	return 1;
}
