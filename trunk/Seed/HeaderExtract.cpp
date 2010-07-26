
#include <memory.h>
#include "SwapTimeStrMath.h"
#include "Judge.h"
#include "HeaderExtract.h"

//控制头宏定义
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

//提取一块记录中的有效信息，返回取得到的信息的长度bytes
//lpHeader: 块的的地址
//iSize: 块大小
//iCopy: 要求直接复制的信息量大小bytes
//lpLost: 返回还缺少的数据，因为一个Blockette的数据可以跨越两个record
//lpBuf: 数据导出缓冲
//注：对于一个块，据格式要求，块的类型域与长度域是在一个record中的，
//不可能分开在两个record中
int ExtractOneCtrlHeader(char* lpHeader, int iSize, int iCopy, \
					 int* lpLost, char* lpBuf)
{
	int iLeft;
	int iBlockLen;
	char* lpBlock;
	char* lpBufBase;
	char* lpBlockLen;
	lpBufBase = lpBuf;

	//跳过记录块前面的8个序列号和字符
	lpBlock = lpHeader + 8;
	iLeft = iSize - 8;

	//首先直接复制iIgnore要求的数据
	if(iCopy >= iLeft)
	{
		//如果要复制的数量大于块中的信息量
		ADD_STR(lpBuf, lpBlock, iLeft);
		*lpLost = iCopy - iLeft;
		return iLeft;
	}

	//否者该块中必有其他的新的块存在
	ADD_STR(lpBuf, lpBlock, iCopy);
	iLeft -= iCopy;
	lpBlock += iCopy;
	
	//添加块格式信息
	ADD_OVER_LINE(lpBuf);	//换行
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

	//导出所有的块
	while(1)
	{
		//判断是否存在下一个Blockette
		if(iLeft < BLOCK_TYPEWORD_LEN + BLOCK_LENWORD_LEN )
		{//不足以放下一个新的Blockette
			*lpLost = 0;
			break;
		}
		lpBlockLen = lpBlock + 3;
		if( ! (IS_BLOCK_TYPE(lpBlock) && IS_BLOCK_LEN(lpBlockLen)) )
		{//没有新的blockette
			*lpLost = 0;
			break;
		}

		//添加block显示格式
		ADD_OVER_LINE(lpBuf);	//换行显示
		ADD_STR(lpBuf, BLOCK_TYPE, BLOCK_TYPE_LEN);//添加块类型格式字符串
		ADD_STR(lpBuf, lpBlock, BLOCK_TYPEWORD_LEN);//添加类型字符串
		ADD_STR(lpBuf, LEN, LEN_LEN);//添加长度格式字符串
		ADD_STR(lpBuf, &lpBlock[3], BLOCK_LENWORD_LEN);//添加长度字符串
		ADD_CHAR(lpBuf, ' ');
		ADD_CHAR(lpBuf, ':');
		ADD_CHAR(lpBuf, ' ');
		//
		iBlockLen = strtoint(lpBlockLen, BLOCK_LENWORD_LEN);
		iLeft -= BLOCK_LENWORD_LEN + BLOCK_TYPEWORD_LEN;
		lpBlockLen += BLOCK_LENWORD_LEN;//指向blockette的内容
		lpBlock += iBlockLen;//得到下一个块的地址，如果存在的话
		iBlockLen -= BLOCK_LENWORD_LEN + BLOCK_TYPEWORD_LEN;//得到实际的内容长度
		//判断一个blockette是否完整的在该块中
		if(iBlockLen <= iLeft)
		{//该块全部在这个record中
			ADD_STR(lpBuf, lpBlockLen, iBlockLen);//添加到buffer中
			iLeft -= iBlockLen;
		}
		else{
			//这个信息块分布在至少两个record中
			ADD_STR(lpBuf, lpBlockLen, iLeft);
			*lpLost = iBlockLen - iLeft;
			break;
		}
	}
	//返回填写的长度
	return lpBuf - lpBufBase;
}

//提取seed文件中的指定控制头信息
int ReadVolumeIndexHeader(char* lpHeader,  int iStep, int iSize, char* lpBuf)
{
	int iCopy, iLen;
	iCopy = 0;
	iLen = 0;
	while (1)
	{
		if(iSize < iStep)//如果剩下的字符数少于一个块的大小，就认为操作结束
			break;
		//提取信息
		if(lpHeader[6] == 'V')
			iLen += ExtractOneCtrlHeader(lpHeader, iStep, iCopy, &iCopy,  lpBuf);
		iSize -= iStep;//剩下的字符数
		lpHeader += iStep;//下一个块
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
		if(iSize < iStep)//如果剩下的字符数少于一个块的大小，就认为操作结束
			break;
		//提取信息
		if(lpHeader[6] == 'A')
			iLen += ExtractOneCtrlHeader(lpHeader, iStep, iCopy, &iCopy,  lpBuf);
		iSize -= iStep;//剩下的字符数
		lpHeader += iStep;//下一个块
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
		if(iSize < iStep)//如果剩下的字符数少于一个块的大小，就认为操作结束
			break;
		//提取信息
		if(lpHeader[6] == 'S')
			iLen += ExtractOneCtrlHeader(lpHeader, iStep, iCopy, &iCopy,  lpBuf);
		iSize -= iStep;//剩下的字符数
		lpHeader += iStep;//下一个块
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
		if(iSize < iStep)//如果剩下的字符数少于一个块的大小，就认为操作结束
			break;
		//提取信息
		if(lpHeader[6] == 'T')
			iLen += ExtractOneCtrlHeader(lpHeader, iStep, iCopy, &iCopy,  lpBuf);
		iSize -= iStep;//剩下的字符数
		lpHeader += iStep;//下一个块
	}
	return iLen;
}

//提取所有的控制头信息
int ReadControlHeader(char* lpHeader,  int iStep, int iSize, char* lpBuf)
{
	int iCopy, iLen;
	iCopy = 0;
	iLen = 0;
	while (1)
	{
		if(iSize < iStep)//如果剩下的字符数少于一个块的大小，就认为操作结束
			break;
		//提取信息
		if(IS_CONTROL_HEADER_CHAR(lpHeader[6]))//如果是控制信息头
			iLen += ExtractOneCtrlHeader(lpHeader, iStep, iCopy, &iCopy,  lpBuf);
		iSize -= iStep;//剩下的字符数
		lpHeader += iStep;//下一个块
	}
	return iLen;

}
