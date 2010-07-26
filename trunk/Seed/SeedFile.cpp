
//导出seed file 类
#define CSEED_FILE_CLASS_IMPLEMENT

#include <wchar.h>
#include "rdseedlib.h"
#include "Convert.h"

#include "OperateStruct.h"
#include "MapFile.h"
#include "StationList.h"
#include "SeedLog.h"
#include "OutFile.h"//导出控制头信息时需要

#include "TxtExport.h"
#include "BinExport.h"
#include "MSeedExport.h"
#include "SeedExport.h"

#include "SeedFile.h"

//后缀常量数组
const WCHAR *extension[] = {
	L"txt",
	L"binx",
	L"mseed",
	L"seed",
	L"info.txt",
	L"tmp"
};

//打开seed文件，校验，并提取所有的台站分量信息
int CSeedFile::open( const WCHAR* pFileName ,CSeedLog* lpLog)
{
	int iLeft, nRet;
	Station station;
	FIX_DATA_HEADER * lpFDH;
	FIX_DATA_HEADER* lpNext;

	//首先释放文件
	close();

	nRet = TRUE;
	//检查seed文件的合法性
	if( seed->MapFile(pFileName, 0) == NULL )
		return FALSE;

	//设置log文件
	SetLogFile(lpLog);
	//日志记录
	if(log != NULL)
	{
		SYSTEMTIME time;

		GetLocalTime(&time);
		sprintf_s(log->logString, MAX_LOG_STR_SIZE, "%04d-%02d-%02d,%02d:%02d:%02d", 
			time.wYear, time.wMonth, time.wDay, 
			time.wHour, time.wMinute, time.wSecond);
		log->FormatAndAppendLog(info_seed, info_open, log->logString);
	}

	if(! IsValidSeed(seed->m_lpBase, seed->m_iSize))//这里方式使用1000块校验，
		nRet = FALSE;
	else if(FALSE == AnalyzeSeedFile(seed->m_lpBase, seed->m_iSize, &m_swap, &m_step) )
		nRet = FALSE;

	if(nRet == FALSE)
	{
		close();
		return FALSE;
	}

	//set the file name and path
	SplitNameAndPath(pFileName, name, path);

	//日志，添加日志
	if(log != NULL)
	{
		char filenameASCII[MAX_PATH];
		WcharNameToCharName(filenameASCII, sizeof(filenameASCII), pFileName, wcslen(pFileName));
		sprintf_s(log->logString, MAX_LOG_STR_SIZE, "file name: %s. file size: %d bytes",
							filenameASCII, seed->m_iSize);
		log->FormatAndAppendLog(info_seed, info_file, log->logString);
	}


	//获取台站信息
	lpFDH = (FIX_DATA_HEADER*)GetFirstDataRecordAddr((char*)seed->m_lpBase, \
									seed->m_iSize, m_step);
	while (lpFDH != NULL)
	{
		BTIME tStart, tend;
		iLeft = seed->m_iSize - ((char*)lpFDH - (char*)(seed->m_lpBase));
		//填充信息
		charTowchar(station.station, lpFDH->szStationID, 5);
		charTowchar(station.channel, lpFDH->szChannelID, 3);

		if(AnalyzeStationInfo((char*)lpFDH, iLeft, m_step, m_swap, (char**)(&lpNext), &station.iRecordLen, 
			&tStart, &tend, &station.nSample, &station.nlost, 
			&station.freq.numerator, &station.freq.denominator) == 0)//出现重大错误，停止分析
			break;
		station.lpAddr = (char*)lpFDH;
		BTimeToSystemTime(&tStart, &station.time);
		BTimeToSystemTime(&tend, &station.tend);
		station.pData = NULL;
		station.bufitemcnt = 0;
		station.iDataCnt = 0;
		station.dataFreq.numerator = station.freq.numerator;
		station.dataFreq.denominator = station.freq.denominator;
		memcpy(&station.datatime, &station.time, sizeof(SYSTEMTIME));
		station.lpNext = NULL;
		list->AddElement(&station);


		if (log != NULL && log->IsDetailLog() )
		{
			//日志，添加台站日志信息
			char szStation[6], szChannel[4];
			float freq;
			freq = ((float)station.freq.numerator) / ((float)station.freq.denominator);
			strcpy_ULN(szStation, lpFDH->szStationID, 5);
			strcpy_ULN(szChannel, lpFDH->szChannelID, 3);
			sprintf_s(log->logString, MAX_LOG_STR_SIZE, 
				"station:%s. channel:%s. "
				"frequency:%f. total samples:%d. lost samples:%d"
				"start time:%04d-%02d-%02d,%02d:%02d:%02d"
				"end time:%04d-%02d-%02d,%02d:%02d:%02d", 
				szStation, szChannel, freq, station.nSample, station.nlost, 
				station.time.wYear, station.time.wMonth, station.time.wDay,
				station.time.wHour, station.time.wMinute, station.time.wSecond, 
				station.tend.wYear, station.tend.wMonth, station.tend.wDay, 
				station.tend.wHour, station.tend.wMinute, station.tend.wSecond);
			log->FormatAndAppendLog(info_seed, info_station, log->logString);
		}

		//准备取下一个台站分量的信息
		lpFDH = lpNext;
	}
	//强制保存
	if(log != NULL)
		log->SaveLog();
	return TRUE;
}

//释放map文件
//释放list中的数据点缓冲
//释放list链表
int CSeedFile::close()
{
	//只记录一次close文件事件
	if (log != NULL && seed->IsMapFileValid())
	{
		//日志，记录close日志
		SYSTEMTIME time;
		GetLocalTime(&time);
		sprintf_s(log->logString, MAX_LOG_STR_SIZE, "%04d-%02d-%02d,%02d:%02d:%02d", 
			time.wYear, time.wMonth, time.wDay, 
			time.wHour, time.wMinute, time.wSecond);
		log->FormatAndAppendLog(info_seed, info_close, log->logString);
	}

	seed->UnmapFile();
	
	//释放数据buffer
	FreeStationData();

	//释放station结构体
	list->Free();

	if(log != NULL)
		log->SaveLog();

	return TRUE;
}

//挂起状态，仅仅释放文件占用内存
void CSeedFile::suspend()
{
	//仅仅释放文件
	seed->SuspendFile();
}
//恢复打开状态，
int CSeedFile::resume()
{
	int iLeft, index;
	Station* pstation;
	FIX_DATA_HEADER * lpFDH;
	FIX_DATA_HEADER* lpNext;
	//只有已经处理过的或则挂起过的seed文件才可以通过下面的检查
	if(seed->ResumeFile() == NULL)
		return FALSE;

	//重新获取台站信息
	lpFDH = (FIX_DATA_HEADER*)GetFirstDataRecordAddr((char*)seed->m_lpBase, \
		seed->m_iSize, m_step);

	index = 0;
	while (lpFDH != NULL)
	{
		//仅仅只改变与seed原始文件内存地址相关的信息，其他不变
		iLeft = seed->m_iSize - ((char*)lpFDH - (char*)seed->m_lpBase);
		//更新信息
		pstation = list->LocateElement(index);
		index ++;
		pstation->lpAddr = (char*)lpFDH;
		//准备取下一个台站分量的信息
		lpNext = (FIX_DATA_HEADER*)GetDataRecordLen((char*)lpFDH, iLeft, m_step, \
			0, m_swap, 0);
		lpFDH = lpNext;
	}
	return TRUE;
}


//释放buffer的数据
int CSeedFile::FreeStationData()
{
	int n;
	Station* lpCur;
	//释放数据buffer
	for(n = 0; n < list->GetElementCnt(); n++)
	{
		lpCur = list->LocateElement(n);
		if(lpCur != NULL)
		{
			if(lpCur->pData != NULL)
				delete[] lpCur->pData;
			lpCur->pData = NULL;
		}
	}
	return TRUE;
}

//解压特定台站分量的信息
int CSeedFile::DecompStation( int index )
{
	Station* lpCur;
	FIX_DATA_HEADER* lpFDH;
	FIX_DATA_HEADER* lpMax;
	int* lpOne;
	int ncnt, ntotal;

	//显示判断文件的有效性
	if(!seed->IsMapFileValid())
		return FALSE;
	lpCur = list->LocateElement(index);
	if(lpCur != NULL)
	{
		//释放掉已有的数据空间
		if(lpCur->pData != NULL)
			delete[] lpCur->pData;
		//重新分配空间
		lpCur->bufitemcnt = lpCur->nSample + m_step*8;
		lpCur->pData = new int[lpCur->bufitemcnt];
		
		lpFDH = (FIX_DATA_HEADER*)lpCur->lpAddr;
		lpMax = (FIX_DATA_HEADER*)((char*)lpFDH + lpCur->iRecordLen);
		lpOne = lpCur->pData;

		//日志，开始解压特定台站分量的数据
		if(log != NULL)
		{
			char szStation[6], szChannel[4];
			float freq;
			freq = ((float)lpCur->dataFreq.numerator) / ((float)lpCur->dataFreq.denominator);
			strcpy_ULN(szStation, lpFDH->szStationID, 5);
			strcpy_ULN(szChannel, lpFDH->szChannelID, 3);
			sprintf_s(log->logString, MAX_LOG_STR_SIZE, "station name:%s. "
				"channel name:%s. sample frequency:%d. "
				"sample start time: %04d-%02d-%02d,%02d:%02d:%02d", 
				szStation, szChannel, freq, 
				lpCur->time.wYear, lpCur->time.wMonth, lpCur->time.wDay, 
				lpCur->time.wHour, lpCur->time.wMinute, lpCur->time.wSecond);
			log->FormatAndAppendLog(info_seed, info_start_unpack, log->logString);
		}
		//解压
		ntotal = 0;
		while(lpFDH < lpMax)
		{
			ncnt= DecompOneRecord((char*)lpFDH, lpOne, m_step*8);
			if(ncnt < 0)//解压出错
				break;
			ntotal += ncnt;
			if(ntotal > lpCur->nSample)//内存保护
				break;
			lpOne += ncnt;
			lpFDH = (FIX_DATA_HEADER*)((char*)lpFDH + m_step);
		}
		//强制保存日志
		if(log != NULL)
			log->SaveLog();
		
		//返回总共解压得到的点数
		lpCur->iDataCnt = ntotal;

		return lpCur->iDataCnt;
	}
	return FALSE;
}

int CSeedFile::GetStationCount()
{
	return list->GetElementCnt();
}

Station* CSeedFile::GetStationInfo( int index )
{
	return list->LocateElement(index);
}

int CSeedFile::GetStationIndex(const WCHAR* pstation, const WCHAR* pchannel)
{
	return list->LocateIndexByStatChnn(pstation, pchannel);
}

//把指定的一个record中的数据解压到内存缓冲中
int CSeedFile::DecompOneRecord( char* lpBase, int* lpData, int iBufLen )
{
	FIX_DATA_HEADER* lpFDH;
	FRAME_STEIM* lpFrame;
	int iFormat, iOrder, iLen;
	int iSample, iNeedCnt;
	int x0, xn;
	unsigned short ustmp, usCnt;

	lpFDH = (FIX_DATA_HEADER*) lpBase;
	ustmp = lpFDH->usDataOffset;
	usCnt = lpFDH->usSampleNum;
	if(m_swap)
	{
		ustmp = SWAP2(ustmp);
		usCnt = SWAP2(usCnt);
	}
	iNeedCnt = usCnt;//仅仅是类型转换
	//提取信息，准备解压
	lpFrame = (FRAME_STEIM*)(lpBase + ustmp);
	if( GetDataAttrib((FIX_DATA_HEADER*)lpBase, m_step, m_swap, &iFormat, &iOrder, &iLen) == 0)
	{
		//非标准mini-seed文件，没有相应的1000块，使用默认设置
		iFormat = m_defFormat;
		iOrder = m_swap;
		iLen = m_step;
	}
	else
		iLen = POWER2(iLen);

	//日志，添加解压信息
	if(log != NULL && log->IsDetailLog())
	{
		char szSequ[8];
		strcpy_ULN(szSequ, lpFDH->szSeqNum, 6);
		sprintf_s(log->logString, MAX_LOG_STR_SIZE, "record sequence:%s. "
			"encoding code:%d. word order:%d. record length:%d", 
			szSequ, iFormat, iOrder, iLen);
		log->FormatAndAppendLog(info_seed, info_unpack_record, log->logString);
	}

	//解压
	switch(iFormat)
	{
	case DE_STEIM1:
		iSample = unpack_steim1(lpFrame, iLen - ustmp, iOrder, lpData, \
								iBufLen, &x0, &xn);
		break;
	case DE_STEIM2:
		iSample = unpack_steim2(lpFrame, iLen - ustmp, iOrder, lpData, \
			iBufLen, &x0, &xn);
		break;
	default:
		//日志，不能解压的数据
		if(log != NULL)
		{
			sprintf_s(log->logString, MAX_LOG_STR_SIZE, "format code:%d", iFormat);
			log->FormatAndAppendLog(error_seed, erro_unknow_code, log->logString);
		}
		return -1;
	}

	//检查错误
	if(lpData[iNeedCnt -1] != xn)//数据不一致
	{
		//日志，数据不一致
		if(log != NULL)
		{
			sprintf_s(log->logString, MAX_LOG_STR_SIZE, "the last data should be:%d", 
						xn);
			log->FormatAndAppendLog(warning_seed, warn_data_differ, log->logString);
		}
	}

	if(iSample > iNeedCnt)//解压出来的数据量过多
	{
		//日志，解压的数据过多
		if(log != NULL)
		{
			sprintf_s(log->logString, MAX_LOG_STR_SIZE, "total decompressed samples:%d. "
				"the needed samples:%d. "
				"the exceeded samples:%d", 
				iSample, iNeedCnt, iSample - iNeedCnt);
			log->FormatAndAppendLog(warning_seed, warn_data_exceed,log->logString);
		}
	}
	else if(iSample < iNeedCnt)//解压出来的数据量不足
	{
		//填补最后面的数据
		int n, last;
		last = iSample - 1;
		for(n = iSample; n < iNeedCnt; n++)
			lpData[n] = lpData[last];
		
		//日志，解压的数据缺少
		if(log != NULL)
		{
			sprintf_s(log->logString, MAX_LOG_STR_SIZE, "total decompressed samples:%d. "
				"the needed samples:%d. "
				"the missing samples:%d. "
				"use the last data to fill:%d", 
				iSample, iNeedCnt, iNeedCnt - iSample, lpData[last]);
			log->FormatAndAppendLog(warning_seed, warn_data_missing,log->logString);
		}
	}
	else//解压出来的数据刚好足数
		//日志，解压的数据正确
		if(log != NULL && log->IsDetailLog())
		{
			sprintf_s(log->logString, MAX_LOG_STR_SIZE, "total decompressed samples:%d", 
				iSample);
			log->FormatAndAppendLog(info_seed, info_data_ok,log->logString);
		}

	return iNeedCnt;
}

//初始化两个类，和其他的成员
CSeedFile::CSeedFile()
{
	seed = new CMapFile;
	list = new CStationList;
	log = NULL;
// 	lpNext = NULL;
	m_step = 4096;
	m_swap = 0;
	m_position = 0;
	m_defFormat = DE_STEIM1;
	name[0] = 0;
	path[0] = 0;
}

//释放数据缓冲和类成员
CSeedFile::~CSeedFile()
{
	close();
	delete list;
	delete seed;
}

//保证log文件可用，或者没有log文件
int CSeedFile::SetLogFile( CSeedLog* lpLog )
{
	//如果外来的log无效，则保持原来的log不变
	if(lpLog != NULL)
	{
		if(lpLog->OpenLog() != FALSE)//如果log打开失败，则保持原log不变
			log = lpLog;
	}
	else
		log = NULL;
	return TRUE;
}

void CSeedFile::SetDefaultFormat( int iformat )
{
	if(iformat == DE_STEIM1)
		m_defFormat = DE_STEIM1;
	else
		m_defFormat = DE_STEIM2;
}


int CSeedFile::GetExportFileName( const WCHAR* lpPath, WCHAR* lpName, int iType, Station* lpst )
{
	WCHAR* lptmp;
	WCHAR nameTmp[MAX_PATH];

	//随便给个初值
	wcscpy_s(nameTmp, MAX_PATH, L"no.name");

	if (lpst != NULL)
	{
		//generate the special name 
		WCHAR * lpUnit;
		float ffreq;
		ffreq  = ((float)lpst->dataFreq.numerator) / ((float)lpst->dataFreq.denominator);
		int tmp;
		if(ffreq >= 1)
		{
			tmp = (int)ffreq;
			lpUnit = L"Hz";
		}
		else 
		{
			tmp = (int)(1.0/ffreq);
			lpUnit = L"s";
		}
		if(iType == EXPORT_TXT || iType == EXPORT_BIN)
		{
			swprintf_s(nameTmp, MAX_PATH, L"%s.%s.%d%s.%04d%02d%02d%02d%02d%02d", 
				lpst->station, lpst->channel, tmp, lpUnit, 
				lpst->datatime.wYear, lpst->datatime.wMonth, lpst->datatime.wDay, 
				lpst->datatime.wHour, lpst->datatime.wMinute, lpst->datatime.wSecond);
		}
		else{
			swprintf_s(nameTmp, MAX_PATH, L"%s.%s.%04d%02d%02d%02d%02d%02d",
				lpst->station, lpst->channel,
				lpst->datatime.wYear, lpst->datatime.wMonth, lpst->datatime.wDay, 
				lpst->datatime.wHour, lpst->datatime.wMinute, lpst->datatime.wSecond);
		}
	}
	//choose the file name 
	switch(iType)
	{
	case EXPORT_TXT:
	case EXPORT_BIN:
	case EXPORT_MSEED:
	case EXPORT_SEED:
		lptmp = nameTmp;
		break;
	case EXPORT_HEADER:
		lptmp = name;
		break;
	default:
		lptmp = L"no.name";
		iType = 5;
		break;
	}
	//generate the file full path name 
	swprintf_s(lpName, MAX_PATH, L"%s\\%s.%s", 
					lpPath, lptmp, extension[iType]);
	return TRUE;
}

int CSeedFile::ExportHeaderInfo( const WCHAR* lpPath )
{
	int iLeft, iLen, iCopy;
	int nRet;
	char* lpBegin;
	WCHAR headerFile[MAX_PATH];
	COutFile OutHeader;
	
	if( !seed->IsMapFileValid() )
		return FALSE;

	if(lpPath == NULL)
		lpPath = path;

	GetExportFileName(lpPath, headerFile, EXPORT_HEADER, NULL);
	char szheaderFile[MAX_PATH];
	WcharNameToCharName(szheaderFile, sizeof(szheaderFile), headerFile, wcslen(headerFile));
	//因为提取信息时的格式字符的存在，
	//使得buffer安全的border要比record大小要大一点
	//这里使用了2倍
	if(FALSE == OutHeader.CreateFile(headerFile, CREATE_ALWAYS, NORMAL_BUFFER_SIZE_CO, m_step*2))
	{
		//日志，添加日志
		if(log != NULL)
		{
			sprintf_s(log->logString, sizeof(log->logString), "the header file name:%s", 
				szheaderFile);
			log->FormatAndAppendLog(error_seed, erro_header_create_failed, log->logString);
		}
		nRet = FALSE;
	}
	else{
		//提取头信息
		lpBegin = (char*)seed->m_lpBase;
		iLeft = seed->m_iSize;
		iCopy = 0;
		iLen = 0;
		while (iLeft >= m_step)
		{
			//提取信息
			if( IS_CONTROL_HEADER_CHAR(lpBegin[6]) )
			{
				nRet= ExtractOneCtrlHeader(lpBegin, m_step, iCopy, &iCopy, OutHeader.m_lpCurrent);
				OutHeader.Adjust(nRet);
			}
			iLen += nRet;
			iLeft -= m_step;//剩下的字符数
			lpBegin += m_step;//下一个块
		}
		//保存并关闭文件
		OutHeader.Save();
		OutHeader.Free();
		//添加日志
		if(log != NULL)
		{
			sprintf_s(log->logString, sizeof(log->logString), 
				"the header file name:%s. header file size: %d", 
				szheaderFile, iLen);
			log->FormatAndAppendLog(info_seed, info_header_export, log->logString);
		}
		nRet = iLen;
	}
	//一个系列操作后，就保存下log
	if(log != NULL)
		log->SaveLog();

	return nRet;
}

//只用到了control参数中的ctrl和type成员
int CSeedFile::ExportData(const WCHAR* ppath, int index, CtrlParam* pctrl)
{
	CExport* pexp;
	Station* pcurt;
	int* pdata;
	int cnt, ret;
	WCHAR filename[MAX_PATH];
	if(ppath == NULL)
		ppath = path;
	pcurt = GetStationInfo(index);
	if(pcurt == NULL)
		return FALSE;
	switch (pctrl->type)
	{
	case EXPORT_TXT:
		{
			if(pcurt->pData == NULL)
				return FALSE;
			pexp = new CTxtExport;
			pdata = pcurt->pData;
			cnt = pcurt->iDataCnt;
		}
		break;
	case EXPORT_BIN:
		{
			if(pcurt->pData == NULL)
				return FALSE;
			pexp = new CBinExport;
			pdata = pcurt->pData;
			cnt = pcurt->iDataCnt;
		}break;
	case EXPORT_MSEED:
		{
			if(seed->IsMapFileValid() == FALSE)
				return FALSE;
			pexp = new CMSeedExport;
			pdata = (int*)pcurt->lpAddr;
			cnt = pcurt->iRecordLen;
		}
		break;
	case EXPORT_SEED:
		{
			if(seed->IsMapFileValid() == FALSE)
				return FALSE;
			pexp = new CSeedExport;
			pdata = (int*)pcurt->lpAddr;
			cnt = pcurt->iRecordLen;
		}
		break;
	default:
		return FALSE;
	}
	GetExportFileName(ppath, filename, pctrl->type, pcurt);
	char szfilename[MAX_PATH];
	WcharNameToCharName(szfilename, MAX_PATH, filename, wcslen(filename));
	CtrlParam param;
	wcscpy_s(param.station, sizeof(param.station)/sizeof(WCHAR), pcurt->station);
	wcscpy_s(param.channel, sizeof(param.channel)/sizeof(WCHAR), pcurt->channel);
	memcpy(&param.tstart, &pcurt->datatime, sizeof(SYSTEMTIME));
	param.numerator = pcurt->dataFreq.numerator;
	param.denominator = pcurt->dataFreq.denominator;
	param.ctrl = pctrl->ctrl;
	if(FALSE == pexp->Create(filename, &param))
	{
		if(log != NULL)
		{
			DWORD res = GetLastError();
			sprintf_s(log->logString, sizeof(log->logString), "file name:%s. error code:%d", 
				filename, res);
			log->FormatAndAppendLog(error_seed, erro_export_file_create_failed, log->logString);
		}
		ret = FALSE;
	} else if(-1 == pexp->ExportData(pdata, ARRAY_DATA, cnt, &param))
	{
		if(log != NULL)
		{
			sprintf_s(log->logString, sizeof(log->logString), "data length:%d", cnt);
			log->FormatAndAppendLog(error_seed, erro_data_export, log->logString);
		}
		ret = FALSE;
	} else{
		if(log != NULL)
		{
			SYSTEMTIME local;
			GetLocalTime(&local);
			sprintf_s(log->logString, MAX_LOG_STR_SIZE, 
				"file export time:%04d-%02d-%02d,%02d:%02d:%02d. "
				"file name:%s. total length:%d",
				local.wYear, local.wMonth, local.wDay, 
				local.wHour, local.wMinute, local.wSecond, 
				szfilename, cnt);
			log->FormatAndAppendLog(info_seed, info_data_export, log->logString);
		}
		ret = TRUE;
	}
	pexp->Close(NULL);
	delete pexp;

	if(log != NULL)
		log->SaveLog();

	return ret;
}

//从iCnt的数据点缓冲lpData中，从偏移lpOffset开始，按照dStep开始抽取数据
//结果放在lpDest中,这里强制要求所有分数的分母完全相同
//这个函数调整begin offset，不调整end offset，也不改变step
int CSeedFile::ExtractOneDataRecord( const int* lpData, int iCnt, FRACTION* pbegin, 
									FRACTION* pend, FRACTION* pstep, int* lpDest)

{
	int nCurrent, nSamples;
	__int64 locMax;
	int * lpBase;

	//抽取到数据尾部的特别命令
	if(pend->intpart < 0)
		locMax = iCnt;
	else if((iCnt == 0) || (pbegin->intpart > pend->intpart) ||
		((pbegin->intpart == pend->intpart) && (pbegin->numerator > pend->numerator)) )
		return 0;
	else
		locMax = pend->intpart;

	lpBase = lpDest;
	nSamples = 0;

	while(1)
	{
		//涉及到补数的问题，这里可以做一定的选择优化
		if(pbegin->intpart < 0)//补充该块数据的首个数据
			nCurrent = 0;
		else
			nCurrent = (int)pbegin->intpart;

		if(nCurrent >= iCnt || (__int64)nCurrent > locMax)
			break;
		*lpDest = lpData[nCurrent];
		lpDest ++;
		//调整分数,准备取下一个点
		pbegin->intpart += pstep->intpart;//计算整数部分
		pbegin->numerator += pstep->numerator;//分子相加
		if(pbegin->numerator >= pbegin->denominator)//假分数化简
		{
			pbegin->intpart ++;
			pbegin->numerator -= pbegin->denominator;
		}
		nSamples ++;//记录抽取到的点数
	}
	
	return nSamples;//返回抽取到的点数
}

//如果失败，返回0
int CSeedFile::DcmpExtrStationDataByTime( int index, SYSTEMTIME* lpStart, 
								  SYSTEMTIME* lpEnd, int numerator, int denominator)
{
	int nSample;
	FRACTION timediff;
	FREQ extfreq;
	Station* lpCurnt;

	//显示判断文件的有效性
	if(!seed->IsMapFileValid())
		return FALSE;

	lpCurnt = GetStationInfo(index);
	if(lpCurnt == NULL)
		return FALSE;
	
	//填充信息
	lpCurnt->dataFreq.numerator = lpCurnt->freq.numerator;
	lpCurnt->dataFreq.denominator = lpCurnt->freq.denominator;
	memcpy(&lpCurnt->datatime, &lpCurnt->time, sizeof(SYSTEMTIME));
	lpCurnt->datatime.wDayOfWeek = 0;
	//释放掉已有的数据空间
	if(lpCurnt->pData != NULL)
		delete lpCurnt->pData;
	lpCurnt->pData = NULL;
	lpCurnt->bufitemcnt = 0;
	lpCurnt->iDataCnt = 0;

	//检查时间的合法性
	if(lpStart == NULL)
		lpStart = &lpCurnt->time;
	else if( CmpSystemTimeS(lpStart, &lpCurnt->time) < 0 )
		lpStart = &lpCurnt->time;
	else if(CmpSystemTimeS(lpStart, &lpCurnt->tend) > 0)
		return TRUE;

	if(lpEnd == NULL)
		lpEnd = &lpCurnt->tend;
	else if( CmpSystemTimeS(lpEnd, &lpCurnt->tend) > 0 )
		lpEnd = &lpCurnt->tend;
	else if( CmpSystemTimeS(lpEnd, &lpCurnt->time) < 0)
		return TRUE;


	//判断时间先后
	SystemTimeSubSystemTimeFraction(lpEnd, lpStart, &timediff.numerator, &timediff.denominator);
	if(timediff.numerator <= 0 )
		return TRUE;

	//填充有效信息
	memcpy(&lpCurnt->datatime, lpStart, sizeof(SYSTEMTIME));

	//检查抽取频率
	if( (numerator <= 0) || (denominator <= 0) || 
		( (lpCurnt->freq.numerator*denominator) - 
		(lpCurnt->freq.denominator*numerator) < 0 ) )
	{
		extfreq.numerator = lpCurnt->freq.numerator;
		extfreq.denominator = lpCurnt->freq.denominator;
	}
	else
	{
		extfreq.numerator = numerator;
		extfreq.denominator = denominator;
	}
	//填充数据信息
	lpCurnt->dataFreq.numerator = extfreq.numerator;
	lpCurnt->dataFreq.denominator = extfreq.denominator;

	//分配内存空间
	nSample = (int)(timediff.numerator*extfreq.numerator/
				(timediff.denominator*extfreq.denominator)) + 1;
	
	//重新分配空间，多分配一个空间
	lpCurnt->pData = new int[nSample + 1];
	lpCurnt->bufitemcnt = nSample + 1;

	//定位解压的record块
	char* lpBegin, * lpMax;
	BTIME begin, max;
	SystemTimeToBTime(lpStart, &begin);
	SystemTimeToBTime(lpEnd, &max);
	lpBegin = LocateRecordByTime(lpCurnt->lpAddr, lpCurnt->iRecordLen, m_step, m_swap, &begin);
	if(lpBegin == 0)
		lpBegin = lpCurnt->lpAddr;
	lpMax = LocateRecordByTime(lpCurnt->lpAddr, lpCurnt->iRecordLen, m_step, m_swap, &max);
	if(lpMax == 0)
		lpMax = lpCurnt->lpAddr + lpCurnt->iRecordLen - m_step;

	int * lpRecordBuf;
	int* lpDest;
	FRACTION fbegin, fend, extStep;
	int nRecordSample, nExtractSample;
	//分配临时缓冲区
	lpRecordBuf = new int[m_step*8];	//最大压缩率为7倍左右
	lpDest = lpCurnt->pData;
	//计算抽取步长参数
	extStep.numerator = extfreq.denominator*lpCurnt->freq.numerator*1000;//使用毫秒单位
	extStep.denominator = extfreq.numerator*lpCurnt->freq.denominator*1000;//使用毫秒单位
	extStep.intpart = extStep.numerator/extStep.denominator;
	extStep.numerator -= extStep.intpart*extStep.denominator;

	//计算终止偏移
	BTIME first;
	GetRecordBTime((FIX_DATA_HEADER*)lpBegin, m_swap, &first);
	BTimeSubBTimeFraction(&max, &first, &fend.numerator, & fend.denominator);
	//变换后，使用相同的分母(1000*原频率分母*抽取频率分子)
	fend.numerator *= lpCurnt->freq.numerator* extfreq.numerator;//计算偏移分子
	fend.denominator *= lpCurnt->freq.denominator * extfreq.numerator;//计算偏移分母
	fend.intpart = fend.numerator/fend.denominator;
	fend.numerator -= fend.denominator * fend.intpart;
	
	nSample = 0;
	m_position = 0;
	SYSTEMTIME firstsys;//数据起始时间
	SYSTEMTIME beginsys;//抽取数据的起始时间
	__int64 begin_numerator_factor;
	begin_numerator_factor = lpCurnt->freq.numerator * extfreq.numerator;
	memcpy(&beginsys, lpStart, sizeof(SYSTEMTIME));//记录起始时间
	while(lpBegin <= lpMax)
	{
		nRecordSample = DecompOneRecord(lpBegin, lpRecordBuf, m_step*8);
		if(nRecordSample < 0)//解压出错
			break;
		GetRecordBTime((FIX_DATA_HEADER*)lpBegin, m_swap, &first);//得到数据块的起始时间
		BTimeToSystemTime(&first, &firstsys);
		SystemTimeSubSystemTimeFraction(&beginsys, &firstsys, &fbegin.numerator, &fbegin.denominator);
		fbegin.numerator *= begin_numerator_factor;//计算偏移分子
		fbegin.denominator = fend.denominator;//这里使用同样的分母计算
		fbegin.intpart = fbegin.numerator/fbegin.denominator;
		fbegin.numerator -= fbegin.denominator * fbegin.intpart;
		//抽取数据
		nExtractSample = ExtractOneDataRecord(lpRecordBuf, nRecordSample, 
					&fbegin, &fend, &extStep, lpDest);
		nSample += nExtractSample;
		//这里应该先把数据抽取到另外一个缓冲中，
		//再检查是否有足够的内存写数据，没有的话直接结束
		//从效率考虑，这样不好，另外，应该不会出现这种错误
		//因为解压返回的数据点数就是文件中的预设值，
		//而且抽取点数超过预设时间就会返回。
		lpDest += nExtractSample;
		lpBegin += m_step;
		m_position ++;
		//计算下一个块的开始时间
		SystemTimeAddFraction(&beginsys, &beginsys, 
			(extfreq.denominator*nExtractSample), extfreq.numerator);
		//计算下一个块的结束偏移
		fend.intpart -= extStep.intpart*nExtractSample;
		fend.numerator -= extStep.numerator*nExtractSample;
		fend.numerator += fend.intpart*fbegin.denominator;
		//调整假分数
		fend.intpart = fend.numerator/fend.denominator;
		fend.numerator -= fend.intpart * fend.denominator;
	}
	
	lpCurnt->iDataCnt = nSample;
	if(log != NULL)
	{
		sprintf_s(log->logString, sizeof(log->logString), "%d", nSample);
		log->FormatAndAppendLog(info_seed, info_extract, log->logString);
	}

	//一项密集操作结束后，强制保存日志
	if(log != NULL)
		log->SaveLog();

	delete[] lpRecordBuf;
	return TRUE;
}

//仅仅只抽取数据，不对时间处理
// int CSeedFile::DcmpExtrStationData( int index, int numerator, int denominator)
// {
// 	int nSample;
// 	FREQ extfreq;
// 	Station* lpCurnt;
// 
// 	//显示判断文件的有效性
// 	if(!seed->IsMapFileValid())
// 		return FALSE;
// 
// 	lpCurnt = GetStationInfo(index);
// 	if(lpCurnt == NULL)
// 		return FALSE;
// 
// 	//正常情况下计算抽取到的点数需要的缓冲
// 	if((numerator <= 0) || (denominator <= 0) ||
// 		( (lpCurnt->freq.numerator*denominator) - 
// 		(lpCurnt->freq.denominator*numerator) < 0 ) )
// 	{
// 		extfreq.numerator = lpCurnt->freq.numerator;
// 		extfreq.denominator = lpCurnt->freq.denominator;
// 	}
// 	else
// 	{
// 		extfreq.numerator = numerator;
// 		extfreq.denominator = denominator;
// 	}
// 	//计算抽取的点数
// 	nSample = (int)(((__int64)lpCurnt->nSample)*lpCurnt->freq.denominator*extfreq.numerator/
// 		(((__int64)lpCurnt->freq.numerator)*extfreq.denominator)) + 1;
// 
// 	int * lpRecordBuf;
// 	int* lpDest;
// 	FRACTION extStep, fbegin, fend;
// 	int nRecordSample, nExtractSample;
// 	int nBufSize;
// 
// 	//填充信息
// 	memcpy(&lpCurnt->datatime, &lpCurnt->time, sizeof(SYSTEMTIME));
// 	lpCurnt->datatime.wDayOfWeek = 0;
// 	lpCurnt->dataFreq.numerator = extfreq.numerator;
// 	lpCurnt->dataFreq.denominator = extfreq.denominator;
// 	//释放掉已有的数据空间
// 	if(lpCurnt->pData != NULL)
// 		delete lpCurnt->pData;
// 	lpCurnt->pData = NULL;
// 	lpCurnt->bufitemcnt = 0;
// 	lpCurnt->iDataCnt = 0;
// 
// 	//重新分配空间，多分配一个空间
// 	nBufSize = m_step*8;
// 	lpRecordBuf = new int[nBufSize];	//最大压缩率为7倍左右
// 	lpCurnt->pData = new int[nSample + 1];
// 	lpCurnt->bufitemcnt = nSample + 1;
// 	lpDest = lpCurnt->pData;
// 	//计算抽取步长参数
// 	extStep.numerator = extfreq.denominator*lpCurnt->freq.numerator*1000;//使用毫秒单位
// 	extStep.denominator = extfreq.numerator*lpCurnt->freq.denominator*1000;//使用毫秒单位
// 	extStep.intpart = extStep.numerator/extStep.denominator;
// 	extStep.numerator -= extStep.intpart*extStep.denominator;
// 
// 	//准备数据
// 	char* lpRecord;
// 	char* lpMax;
// 	//填充起始偏移
// 	fbegin.intpart = 0;
// 	fbegin.numerator = 0;
// 	fbegin.denominator = extStep.denominator;
// 	//填充终止偏移
// 	fend.intpart = -1;//数据抽取的特殊命令
// 	nSample = 0;
// 	lpRecord = lpCurnt->lpAddr;
// 	lpMax = lpRecord + lpCurnt->iRecordLen - m_step;
// 	while(lpRecord <= lpMax)
// 	{
// 		nRecordSample = DecompOneRecord(lpRecord, lpRecordBuf, nBufSize);
// 		if(nRecordSample < 0)//解压出错
// 			break;
// 		//抽取
// 		nExtractSample = ExtractOneDataRecord(lpRecordBuf, nRecordSample, 
// 				&fbegin, &fend, &extStep, lpDest);
// 		//记录抽到的点数
// 		nSample += nExtractSample;
// 		//这里应该先把数据抽取到另外一个缓冲中，
// 		//再检查是否有足够的内存写数据，没有的话直接结束
// 		//从效率考虑，这样不好，另外，应该不会出现这种错误
// 		//因为解压返回的数据点数就是文件中的预设值，
// 		//而且抽取点数超过预设时间就会返回。
// // 		if(nSample > lpCurnt->bufitemcnt)//检测内存
// // 			break;
// 		//调整缓冲区
// 		lpDest += nExtractSample;
// 		//调整记录块
// 		lpRecord += m_step;
// 		
// 		//计算下一个块的开始偏移
// 		fbegin.intpart -= nRecordSample;
// 
// 		m_position ++;//记录当前进度
// 	}
// 
// 	lpCurnt->iDataCnt = nSample;
// 	if(log != NULL)
// 	{
// 		sprintf_s(log->logString, sizeof(log->logString), "%d", nSample);
// 		log->FormatAndAppendLog(info_seed, info_extract, log->logString);
// 	}
// 
// 	//一项密集操作结束后，强制保存日志
// 	if(log != NULL)
// 		log->SaveLog();
// 
// 	delete[] lpRecordBuf;
// 	return TRUE;
// }