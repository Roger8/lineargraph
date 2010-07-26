
#include <memory.h>
#include "SwapTimeStrMath.h"
#include "Judge.h"
#include "HeaderExtract.h"

//����ͷ�궨��
#define SEQUENCE_NUMBER			6
#define CONTROL_HEADER_INDEX	8
#define BLOCK_TYPEWORD_LEN		3
#define BLOCK_LENWORD_LEN		4

#define MAX_BLOCK_ID			74
#define MIN_BLOCK_ID			5

#define LOGREC					"logrec "
#define LOGREC_LEN				(sizeof(LOGREC)-1)
#define TYPE					" type "
#define TYPE_LEN				(sizeof(TYPE)-1)
#define BLOCK_TYPE				"    type "
#define BLOCK_TYPE_LEN			(sizeof(BLOCK_TYPE)-1)
#define LEN						" len "
#define LEN_LEN					(sizeof(LEN)-1)

#define INDEX_TYPE				"'V'"
#define DICTIONARY_TYPE			"'A'"
#define STATION_TYPE			"'S'"
#define TIMESPAN_TYPE			"'T'"

#define INDEX_C_TYPE			"'V*'"
#define DICTIONARY_C_TYPE		"'A*'"
#define STATION_C_TYPE			"'S*'"
#define TIMESPAN_C_TYPE			"'T*'"

#define OVER_LINE_STR			"\r\n"

#define OVER_LINE_LEN				2
#define TYPE_CHAR_LEN				3
#define TYPE_CHAR_LEN_CONTINUE		4

#define  ADD_OVER_LINE(lpDest)	{memcpy(lpDest, OVER_LINE_STR, OVER_LINE_LEN);lpDest += OVER_LINE_LEN;}

#define ADD_STR(lpDest, lpSrc, len)		{memcpy(lpDest, lpSrc, len);lpDest += len;}

#define ADD_CHAR(lpDest, ch)			{*lpDest = ch; lpDest++;}

#define IS_BLOCK_TYPE(lpstr)	( ( IS_DIGIT(lpstr[0]) ) && ( IS_DIGIT(lpstr[1]) ) && \
						( IS_DIGIT(lpstr[2]) ) )

#define IS_BLOCK_LEN(lpstr)	( IS_DIGIT(lpstr[0]) && IS_DIGIT(lpstr[1]) && \
						IS_DIGIT(lpstr[2]) && IS_DIGIT(lpstr[3]) )

//��ȡһ���¼�е���Ч��Ϣ������ȡ�õ�����Ϣ�ĳ���bytes
//lpHeader: ��ĵĵ�ַ
//iSize: ���С
//iCopy: Ҫ��ֱ�Ӹ��Ƶ���Ϣ����Сbytes
//lpLost: ���ػ�ȱ�ٵ����ݣ���Ϊһ��Blockette�����ݿ��Կ�Խ����record
//lpBuf: ���ݵ�������
//ע������һ���飬�ݸ�ʽҪ�󣬿���������볤��������һ��record�еģ�
//�����ֿܷ�������record��
int ExtractOneCtrlHeader(char* lpHeader, int iSize, int iCopy, \
					 int* lpLost, char* lpBuf)
{
	int iLeft;
	int iBlockLen;
	char* lpBlock;
	char* lpBufBase;
	char* lpBlockLen;
	lpBufBase = lpBuf;

	//������¼��ǰ���8�����кź��ַ�
	lpBlock = lpHeader + 8;
	iLeft = iSize - 8;

	//����ֱ�Ӹ���iIgnoreҪ�������
	if(iCopy >= iLeft)
	{
		//���Ҫ���Ƶ��������ڿ��е���Ϣ��
		ADD_STR(lpBuf, lpBlock, iLeft);
		*lpLost = iCopy - iLeft;
		return iLeft;
	}

	//���߸ÿ��б����������µĿ����
	ADD_STR(lpBuf, lpBlock, iCopy);
	iLeft -= iCopy;
	lpBlock += iCopy;
	
	//��ӿ��ʽ��Ϣ
	ADD_OVER_LINE(lpBuf);	//����
	ADD_STR(lpBuf, LOGREC, LOGREC_LEN);
	ADD_STR(lpBuf, lpHeader, SEQUENCE_NUMBER);
	ADD_STR(lpBuf, TYPE, TYPE_LEN);
	ADD_CHAR(lpBuf, '\'');
	if(lpHeader[7] == '*')
	{
		ADD_STR(lpBuf, &lpHeader[6], 2);
	}
	else
	{
		ADD_CHAR(lpBuf, lpHeader[6]);
	}
	ADD_CHAR(lpBuf, '\'');

	//�������еĿ�
	while(1)
	{
		//�ж��Ƿ������һ��Blockette
		if(iLeft < BLOCK_TYPEWORD_LEN + BLOCK_LENWORD_LEN )
		{//�����Է���һ���µ�Blockette
			*lpLost = 0;
			break;
		}
		lpBlockLen = lpBlock + 3;
		if( ! (IS_BLOCK_TYPE(lpBlock) && IS_BLOCK_LEN(lpBlockLen)) )
		{//û���µ�blockette
			*lpLost = 0;
			break;
		}

		//���block��ʾ��ʽ
		ADD_OVER_LINE(lpBuf);	//������ʾ
		ADD_STR(lpBuf, BLOCK_TYPE, BLOCK_TYPE_LEN);//��ӿ����͸�ʽ�ַ���
		ADD_STR(lpBuf, lpBlock, BLOCK_TYPEWORD_LEN);//��������ַ���
		ADD_STR(lpBuf, LEN, LEN_LEN);//��ӳ��ȸ�ʽ�ַ���
		ADD_STR(lpBuf, &lpBlock[3], BLOCK_LENWORD_LEN);//��ӳ����ַ���
		ADD_CHAR(lpBuf, ' ');
		ADD_CHAR(lpBuf, ':');
		ADD_CHAR(lpBuf, ' ');
		//
		iBlockLen = strtoint(lpBlockLen, BLOCK_LENWORD_LEN);
		iLeft -= BLOCK_LENWORD_LEN + BLOCK_TYPEWORD_LEN;
		lpBlockLen += BLOCK_LENWORD_LEN;//ָ��blockette������
		lpBlock += iBlockLen;//�õ���һ����ĵ�ַ��������ڵĻ�
		iBlockLen -= BLOCK_LENWORD_LEN + BLOCK_TYPEWORD_LEN;//�õ�ʵ�ʵ����ݳ���
		//�ж�һ��blockette�Ƿ��������ڸÿ���
		if(iBlockLen <= iLeft)
		{//�ÿ�ȫ�������record��
			ADD_STR(lpBuf, lpBlockLen, iBlockLen);//��ӵ�buffer��
			iLeft -= iBlockLen;
		}
		else{
			//�����Ϣ��ֲ�����������record��
			ADD_STR(lpBuf, lpBlockLen, iLeft);
			*lpLost = iBlockLen - iLeft;
			break;
		}
	}
	//������д�ĳ���
	return lpBuf - lpBufBase;
}

//��ȡseed�ļ��е�ָ������ͷ��Ϣ
int ReadVolumeIndexHeader(char* lpHeader,  int iStep, int iSize, char* lpBuf)
{
	int iCopy, iLen;
	iCopy = 0;
	iLen = 0;
	while (1)
	{
		if(iSize < iStep)//���ʣ�µ��ַ�������һ����Ĵ�С������Ϊ��������
			break;
		//��ȡ��Ϣ
		if(lpHeader[6] == 'V')
			iLen += ExtractOneCtrlHeader(lpHeader, iStep, iCopy, &iCopy,  lpBuf);
		iSize -= iStep;//ʣ�µ��ַ���
		lpHeader += iStep;//��һ����
	}
	return iLen;
}

int ReadDictionaryHeader(char* lpHeader,  int iStep, int iSize, char* lpBuf)
{
	int iCopy, iLen;
	iCopy = 0;
	iLen = 0;
	while (1)
	{
		if(iSize < iStep)//���ʣ�µ��ַ�������һ����Ĵ�С������Ϊ��������
			break;
		//��ȡ��Ϣ
		if(lpHeader[6] == 'A')
			iLen += ExtractOneCtrlHeader(lpHeader, iStep, iCopy, &iCopy,  lpBuf);
		iSize -= iStep;//ʣ�µ��ַ���
		lpHeader += iStep;//��һ����
	}
	return iLen;
}

int ReadStationHeader(char* lpHeader,  int iStep, int iSize, char* lpBuf)
{
	int iCopy, iLen;
	iCopy = 0;
	iLen = 0;
	while (1)
	{
		if(iSize < iStep)//���ʣ�µ��ַ�������һ����Ĵ�С������Ϊ��������
			break;
		//��ȡ��Ϣ
		if(lpHeader[6] == 'S')
			iLen += ExtractOneCtrlHeader(lpHeader, iStep, iCopy, &iCopy,  lpBuf);
		iSize -= iStep;//ʣ�µ��ַ���
		lpHeader += iStep;//��һ����
	}
	return iLen;
}

int ReadTimeSpanHeader(char* lpHeader,  int iStep, int iSize, char* lpBuf)
{
	int iCopy, iLen;
	iCopy = 0;
	iLen = 0;
	while (1)
	{
		if(iSize < iStep)//���ʣ�µ��ַ�������һ����Ĵ�С������Ϊ��������
			break;
		//��ȡ��Ϣ
		if(lpHeader[6] == 'T')
			iLen += ExtractOneCtrlHeader(lpHeader, iStep, iCopy, &iCopy,  lpBuf);
		iSize -= iStep;//ʣ�µ��ַ���
		lpHeader += iStep;//��һ����
	}
	return iLen;
}

//��ȡ���еĿ���ͷ��Ϣ
int ReadControlHeader(char* lpHeader,  int iStep, int iSize, char* lpBuf)
{
	int iCopy, iLen;
	iCopy = 0;
	iLen = 0;
	while (1)
	{
		if(iSize < iStep)//���ʣ�µ��ַ�������һ����Ĵ�С������Ϊ��������
			break;
		//��ȡ��Ϣ
		if(IS_CONTROL_HEADER_CHAR(lpHeader[6]))//����ǿ�����Ϣͷ
			iLen += ExtractOneCtrlHeader(lpHeader, iStep, iCopy, &iCopy,  lpBuf);
		iSize -= iStep;//ʣ�µ��ַ���
		lpHeader += iStep;//��һ����
	}
	return iLen;

}
