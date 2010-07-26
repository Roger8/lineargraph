
//����seed file ��
#define CSEED_FILE_CLASS_IMPLEMENT

#include <wchar.h>
#include "rdseedlib.h"
#include "Convert.h"

#include "OperateStruct.h"
#include "MapFile.h"
#include "StationList.h"
#include "SeedLog.h"
#include "OutFile.h"//��������ͷ��Ϣʱ��Ҫ

#include "TxtExport.h"
#include "BinExport.h"
#include "MSeedExport.h"
#include "SeedExport.h"

#include "SeedFile.h"

//��׺��������
const WCHAR *extension[] = {
	L"txt",
	L"binx",
	L"mseed",
	L"seed",
	L"info.txt",
	L"tmp"
};

//��seed�ļ���У�飬����ȡ���е�̨վ������Ϣ
int CSeedFile::open( const WCHAR* pFileName ,CSeedLog* lpLog)
{
	int iLeft, nRet;
	Station station;
	FIX_DATA_HEADER * lpFDH;
	FIX_DATA_HEADER* lpNext;

	//�����ͷ��ļ�
	close();

	nRet = TRUE;
	//���seed�ļ��ĺϷ���
	if( seed->MapFile(pFileName, 0) == NULL )
		return FALSE;

	//����log�ļ�
	SetLogFile(lpLog);
	//��־��¼
	if(log != NULL)
	{
		SYSTEMTIME time;

		GetLocalTime(&time);
		sprintf_s(log->logString, MAX_LOG_STR_SIZE, "%04d-%02d-%02d,%02d:%02d:%02d", 
			time.wYear, time.wMonth, time.wDay, 
			time.wHour, time.wMinute, time.wSecond);
		log->FormatAndAppendLog(info_seed, info_open, log->logString);
	}

	if(! IsValidSeed(seed->m_lpBase, seed->m_iSize))//���﷽ʽʹ��1000��У�飬
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

	//��־�������־
	if(log != NULL)
	{
		char filenameASCII[MAX_PATH];
		WcharNameToCharName(filenameASCII, sizeof(filenameASCII), pFileName, wcslen(pFileName));
		sprintf_s(log->logString, MAX_LOG_STR_SIZE, "file name: %s. file size: %d bytes",
							filenameASCII, seed->m_iSize);
		log->FormatAndAppendLog(info_seed, info_file, log->logString);
	}


	//��ȡ̨վ��Ϣ
	lpFDH = (FIX_DATA_HEADER*)GetFirstDataRecordAddr((char*)seed->m_lpBase, \
									seed->m_iSize, m_step);
	while (lpFDH != NULL)
	{
		BTIME tStart, tend;
		iLeft = seed->m_iSize - ((char*)lpFDH - (char*)(seed->m_lpBase));
		//�����Ϣ
		charTowchar(station.station, lpFDH->szStationID, 5);
		charTowchar(station.channel, lpFDH->szChannelID, 3);

		if(AnalyzeStationInfo((char*)lpFDH, iLeft, m_step, m_swap, (char**)(&lpNext), &station.iRecordLen, 
			&tStart, &tend, &station.nSample, &station.nlost, 
			&station.freq.numerator, &station.freq.denominator) == 0)//�����ش����ֹͣ����
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
			//��־�����̨վ��־��Ϣ
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

		//׼��ȡ��һ��̨վ��������Ϣ
		lpFDH = lpNext;
	}
	//ǿ�Ʊ���
	if(log != NULL)
		log->SaveLog();
	return TRUE;
}

//�ͷ�map�ļ�
//�ͷ�list�е����ݵ㻺��
//�ͷ�list����
int CSeedFile::close()
{
	//ֻ��¼һ��close�ļ��¼�
	if (log != NULL && seed->IsMapFileValid())
	{
		//��־����¼close��־
		SYSTEMTIME time;
		GetLocalTime(&time);
		sprintf_s(log->logString, MAX_LOG_STR_SIZE, "%04d-%02d-%02d,%02d:%02d:%02d", 
			time.wYear, time.wMonth, time.wDay, 
			time.wHour, time.wMinute, time.wSecond);
		log->FormatAndAppendLog(info_seed, info_close, log->logString);
	}

	seed->UnmapFile();
	
	//�ͷ�����buffer
	FreeStationData();

	//�ͷ�station�ṹ��
	list->Free();

	if(log != NULL)
		log->SaveLog();

	return TRUE;
}

//����״̬�������ͷ��ļ�ռ���ڴ�
void CSeedFile::suspend()
{
	//�����ͷ��ļ�
	seed->SuspendFile();
}
//�ָ���״̬��
int CSeedFile::resume()
{
	int iLeft, index;
	Station* pstation;
	FIX_DATA_HEADER * lpFDH;
	FIX_DATA_HEADER* lpNext;
	//ֻ���Ѿ�������Ļ���������seed�ļ��ſ���ͨ������ļ��
	if(seed->ResumeFile() == NULL)
		return FALSE;

	//���»�ȡ̨վ��Ϣ
	lpFDH = (FIX_DATA_HEADER*)GetFirstDataRecordAddr((char*)seed->m_lpBase, \
		seed->m_iSize, m_step);

	index = 0;
	while (lpFDH != NULL)
	{
		//����ֻ�ı���seedԭʼ�ļ��ڴ��ַ��ص���Ϣ����������
		iLeft = seed->m_iSize - ((char*)lpFDH - (char*)seed->m_lpBase);
		//������Ϣ
		pstation = list->LocateElement(index);
		index ++;
		pstation->lpAddr = (char*)lpFDH;
		//׼��ȡ��һ��̨վ��������Ϣ
		lpNext = (FIX_DATA_HEADER*)GetDataRecordLen((char*)lpFDH, iLeft, m_step, \
			0, m_swap, 0);
		lpFDH = lpNext;
	}
	return TRUE;
}


//�ͷ�buffer������
int CSeedFile::FreeStationData()
{
	int n;
	Station* lpCur;
	//�ͷ�����buffer
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

//��ѹ�ض�̨վ��������Ϣ
int CSeedFile::DecompStation( int index )
{
	Station* lpCur;
	FIX_DATA_HEADER* lpFDH;
	FIX_DATA_HEADER* lpMax;
	int* lpOne;
	int ncnt, ntotal;

	//��ʾ�ж��ļ�����Ч��
	if(!seed->IsMapFileValid())
		return FALSE;
	lpCur = list->LocateElement(index);
	if(lpCur != NULL)
	{
		//�ͷŵ����е����ݿռ�
		if(lpCur->pData != NULL)
			delete[] lpCur->pData;
		//���·���ռ�
		lpCur->bufitemcnt = lpCur->nSample + m_step*8;
		lpCur->pData = new int[lpCur->bufitemcnt];
		
		lpFDH = (FIX_DATA_HEADER*)lpCur->lpAddr;
		lpMax = (FIX_DATA_HEADER*)((char*)lpFDH + lpCur->iRecordLen);
		lpOne = lpCur->pData;

		//��־����ʼ��ѹ�ض�̨վ����������
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
		//��ѹ
		ntotal = 0;
		while(lpFDH < lpMax)
		{
			ncnt= DecompOneRecord((char*)lpFDH, lpOne, m_step*8);
			if(ncnt < 0)//��ѹ����
				break;
			ntotal += ncnt;
			if(ntotal > lpCur->nSample)//�ڴ汣��
				break;
			lpOne += ncnt;
			lpFDH = (FIX_DATA_HEADER*)((char*)lpFDH + m_step);
		}
		//ǿ�Ʊ�����־
		if(log != NULL)
			log->SaveLog();
		
		//�����ܹ���ѹ�õ��ĵ���
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

//��ָ����һ��record�е����ݽ�ѹ���ڴ滺����
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
	iNeedCnt = usCnt;//����������ת��
	//��ȡ��Ϣ��׼����ѹ
	lpFrame = (FRAME_STEIM*)(lpBase + ustmp);
	if( GetDataAttrib((FIX_DATA_HEADER*)lpBase, m_step, m_swap, &iFormat, &iOrder, &iLen) == 0)
	{
		//�Ǳ�׼mini-seed�ļ���û����Ӧ��1000�飬ʹ��Ĭ������
		iFormat = m_defFormat;
		iOrder = m_swap;
		iLen = m_step;
	}
	else
		iLen = POWER2(iLen);

	//��־����ӽ�ѹ��Ϣ
	if(log != NULL && log->IsDetailLog())
	{
		char szSequ[8];
		strcpy_ULN(szSequ, lpFDH->szSeqNum, 6);
		sprintf_s(log->logString, MAX_LOG_STR_SIZE, "record sequence:%s. "
			"encoding code:%d. word order:%d. record length:%d", 
			szSequ, iFormat, iOrder, iLen);
		log->FormatAndAppendLog(info_seed, info_unpack_record, log->logString);
	}

	//��ѹ
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
		//��־�����ܽ�ѹ������
		if(log != NULL)
		{
			sprintf_s(log->logString, MAX_LOG_STR_SIZE, "format code:%d", iFormat);
			log->FormatAndAppendLog(error_seed, erro_unknow_code, log->logString);
		}
		return -1;
	}

	//������
	if(lpData[iNeedCnt -1] != xn)//���ݲ�һ��
	{
		//��־�����ݲ�һ��
		if(log != NULL)
		{
			sprintf_s(log->logString, MAX_LOG_STR_SIZE, "the last data should be:%d", 
						xn);
			log->FormatAndAppendLog(warning_seed, warn_data_differ, log->logString);
		}
	}

	if(iSample > iNeedCnt)//��ѹ����������������
	{
		//��־����ѹ�����ݹ���
		if(log != NULL)
		{
			sprintf_s(log->logString, MAX_LOG_STR_SIZE, "total decompressed samples:%d. "
				"the needed samples:%d. "
				"the exceeded samples:%d", 
				iSample, iNeedCnt, iSample - iNeedCnt);
			log->FormatAndAppendLog(warning_seed, warn_data_exceed,log->logString);
		}
	}
	else if(iSample < iNeedCnt)//��ѹ����������������
	{
		//�����������
		int n, last;
		last = iSample - 1;
		for(n = iSample; n < iNeedCnt; n++)
			lpData[n] = lpData[last];
		
		//��־����ѹ������ȱ��
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
	else//��ѹ���������ݸպ�����
		//��־����ѹ��������ȷ
		if(log != NULL && log->IsDetailLog())
		{
			sprintf_s(log->logString, MAX_LOG_STR_SIZE, "total decompressed samples:%d", 
				iSample);
			log->FormatAndAppendLog(info_seed, info_data_ok,log->logString);
		}

	return iNeedCnt;
}

//��ʼ�������࣬�������ĳ�Ա
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

//�ͷ����ݻ�������Ա
CSeedFile::~CSeedFile()
{
	close();
	delete list;
	delete seed;
}

//��֤log�ļ����ã�����û��log�ļ�
int CSeedFile::SetLogFile( CSeedLog* lpLog )
{
	//���������log��Ч���򱣳�ԭ����log����
	if(lpLog != NULL)
	{
		if(lpLog->OpenLog() != FALSE)//���log��ʧ�ܣ��򱣳�ԭlog����
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

	//��������ֵ
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
	//��Ϊ��ȡ��Ϣʱ�ĸ�ʽ�ַ��Ĵ��ڣ�
	//ʹ��buffer��ȫ��borderҪ��record��СҪ��һ��
	//����ʹ����2��
	if(FALSE == OutHeader.CreateFile(headerFile, CREATE_ALWAYS, NORMAL_BUFFER_SIZE_CO, m_step*2))
	{
		//��־�������־
		if(log != NULL)
		{
			sprintf_s(log->logString, sizeof(log->logString), "the header file name:%s", 
				szheaderFile);
			log->FormatAndAppendLog(error_seed, erro_header_create_failed, log->logString);
		}
		nRet = FALSE;
	}
	else{
		//��ȡͷ��Ϣ
		lpBegin = (char*)seed->m_lpBase;
		iLeft = seed->m_iSize;
		iCopy = 0;
		iLen = 0;
		while (iLeft >= m_step)
		{
			//��ȡ��Ϣ
			if( IS_CONTROL_HEADER_CHAR(lpBegin[6]) )
			{
				nRet= ExtractOneCtrlHeader(lpBegin, m_step, iCopy, &iCopy, OutHeader.m_lpCurrent);
				OutHeader.Adjust(nRet);
			}
			iLen += nRet;
			iLeft -= m_step;//ʣ�µ��ַ���
			lpBegin += m_step;//��һ����
		}
		//���沢�ر��ļ�
		OutHeader.Save();
		OutHeader.Free();
		//�����־
		if(log != NULL)
		{
			sprintf_s(log->logString, sizeof(log->logString), 
				"the header file name:%s. header file size: %d", 
				szheaderFile, iLen);
			log->FormatAndAppendLog(info_seed, info_header_export, log->logString);
		}
		nRet = iLen;
	}
	//һ��ϵ�в����󣬾ͱ�����log
	if(log != NULL)
		log->SaveLog();

	return nRet;
}

//ֻ�õ���control�����е�ctrl��type��Ա
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

//��iCnt�����ݵ㻺��lpData�У���ƫ��lpOffset��ʼ������dStep��ʼ��ȡ����
//�������lpDest��,����ǿ��Ҫ�����з����ķ�ĸ��ȫ��ͬ
//�����������begin offset��������end offset��Ҳ���ı�step
int CSeedFile::ExtractOneDataRecord( const int* lpData, int iCnt, FRACTION* pbegin, 
									FRACTION* pend, FRACTION* pstep, int* lpDest)

{
	int nCurrent, nSamples;
	__int64 locMax;
	int * lpBase;

	//��ȡ������β�����ر�����
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
		//�漰�����������⣬���������һ����ѡ���Ż�
		if(pbegin->intpart < 0)//����ÿ����ݵ��׸�����
			nCurrent = 0;
		else
			nCurrent = (int)pbegin->intpart;

		if(nCurrent >= iCnt || (__int64)nCurrent > locMax)
			break;
		*lpDest = lpData[nCurrent];
		lpDest ++;
		//��������,׼��ȡ��һ����
		pbegin->intpart += pstep->intpart;//������������
		pbegin->numerator += pstep->numerator;//�������
		if(pbegin->numerator >= pbegin->denominator)//�ٷ�������
		{
			pbegin->intpart ++;
			pbegin->numerator -= pbegin->denominator;
		}
		nSamples ++;//��¼��ȡ���ĵ���
	}
	
	return nSamples;//���س�ȡ���ĵ���
}

//���ʧ�ܣ�����0
int CSeedFile::DcmpExtrStationDataByTime( int index, SYSTEMTIME* lpStart, 
								  SYSTEMTIME* lpEnd, int numerator, int denominator)
{
	int nSample;
	FRACTION timediff;
	FREQ extfreq;
	Station* lpCurnt;

	//��ʾ�ж��ļ�����Ч��
	if(!seed->IsMapFileValid())
		return FALSE;

	lpCurnt = GetStationInfo(index);
	if(lpCurnt == NULL)
		return FALSE;
	
	//�����Ϣ
	lpCurnt->dataFreq.numerator = lpCurnt->freq.numerator;
	lpCurnt->dataFreq.denominator = lpCurnt->freq.denominator;
	memcpy(&lpCurnt->datatime, &lpCurnt->time, sizeof(SYSTEMTIME));
	lpCurnt->datatime.wDayOfWeek = 0;
	//�ͷŵ����е����ݿռ�
	if(lpCurnt->pData != NULL)
		delete lpCurnt->pData;
	lpCurnt->pData = NULL;
	lpCurnt->bufitemcnt = 0;
	lpCurnt->iDataCnt = 0;

	//���ʱ��ĺϷ���
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


	//�ж�ʱ���Ⱥ�
	SystemTimeSubSystemTimeFraction(lpEnd, lpStart, &timediff.numerator, &timediff.denominator);
	if(timediff.numerator <= 0 )
		return TRUE;

	//�����Ч��Ϣ
	memcpy(&lpCurnt->datatime, lpStart, sizeof(SYSTEMTIME));

	//����ȡƵ��
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
	//���������Ϣ
	lpCurnt->dataFreq.numerator = extfreq.numerator;
	lpCurnt->dataFreq.denominator = extfreq.denominator;

	//�����ڴ�ռ�
	nSample = (int)(timediff.numerator*extfreq.numerator/
				(timediff.denominator*extfreq.denominator)) + 1;
	
	//���·���ռ䣬�����һ���ռ�
	lpCurnt->pData = new int[nSample + 1];
	lpCurnt->bufitemcnt = nSample + 1;

	//��λ��ѹ��record��
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
	//������ʱ������
	lpRecordBuf = new int[m_step*8];	//���ѹ����Ϊ7������
	lpDest = lpCurnt->pData;
	//�����ȡ��������
	extStep.numerator = extfreq.denominator*lpCurnt->freq.numerator*1000;//ʹ�ú��뵥λ
	extStep.denominator = extfreq.numerator*lpCurnt->freq.denominator*1000;//ʹ�ú��뵥λ
	extStep.intpart = extStep.numerator/extStep.denominator;
	extStep.numerator -= extStep.intpart*extStep.denominator;

	//������ֹƫ��
	BTIME first;
	GetRecordBTime((FIX_DATA_HEADER*)lpBegin, m_swap, &first);
	BTimeSubBTimeFraction(&max, &first, &fend.numerator, & fend.denominator);
	//�任��ʹ����ͬ�ķ�ĸ(1000*ԭƵ�ʷ�ĸ*��ȡƵ�ʷ���)
	fend.numerator *= lpCurnt->freq.numerator* extfreq.numerator;//����ƫ�Ʒ���
	fend.denominator *= lpCurnt->freq.denominator * extfreq.numerator;//����ƫ�Ʒ�ĸ
	fend.intpart = fend.numerator/fend.denominator;
	fend.numerator -= fend.denominator * fend.intpart;
	
	nSample = 0;
	m_position = 0;
	SYSTEMTIME firstsys;//������ʼʱ��
	SYSTEMTIME beginsys;//��ȡ���ݵ���ʼʱ��
	__int64 begin_numerator_factor;
	begin_numerator_factor = lpCurnt->freq.numerator * extfreq.numerator;
	memcpy(&beginsys, lpStart, sizeof(SYSTEMTIME));//��¼��ʼʱ��
	while(lpBegin <= lpMax)
	{
		nRecordSample = DecompOneRecord(lpBegin, lpRecordBuf, m_step*8);
		if(nRecordSample < 0)//��ѹ����
			break;
		GetRecordBTime((FIX_DATA_HEADER*)lpBegin, m_swap, &first);//�õ����ݿ����ʼʱ��
		BTimeToSystemTime(&first, &firstsys);
		SystemTimeSubSystemTimeFraction(&beginsys, &firstsys, &fbegin.numerator, &fbegin.denominator);
		fbegin.numerator *= begin_numerator_factor;//����ƫ�Ʒ���
		fbegin.denominator = fend.denominator;//����ʹ��ͬ���ķ�ĸ����
		fbegin.intpart = fbegin.numerator/fbegin.denominator;
		fbegin.numerator -= fbegin.denominator * fbegin.intpart;
		//��ȡ����
		nExtractSample = ExtractOneDataRecord(lpRecordBuf, nRecordSample, 
					&fbegin, &fend, &extStep, lpDest);
		nSample += nExtractSample;
		//����Ӧ���Ȱ����ݳ�ȡ������һ�������У�
		//�ټ���Ƿ����㹻���ڴ�д���ݣ�û�еĻ�ֱ�ӽ���
		//��Ч�ʿ��ǣ��������ã����⣬Ӧ�ò���������ִ���
		//��Ϊ��ѹ���ص����ݵ��������ļ��е�Ԥ��ֵ��
		//���ҳ�ȡ��������Ԥ��ʱ��ͻ᷵�ء�
		lpDest += nExtractSample;
		lpBegin += m_step;
		m_position ++;
		//������һ����Ŀ�ʼʱ��
		SystemTimeAddFraction(&beginsys, &beginsys, 
			(extfreq.denominator*nExtractSample), extfreq.numerator);
		//������һ����Ľ���ƫ��
		fend.intpart -= extStep.intpart*nExtractSample;
		fend.numerator -= extStep.numerator*nExtractSample;
		fend.numerator += fend.intpart*fbegin.denominator;
		//�����ٷ���
		fend.intpart = fend.numerator/fend.denominator;
		fend.numerator -= fend.intpart * fend.denominator;
	}
	
	lpCurnt->iDataCnt = nSample;
	if(log != NULL)
	{
		sprintf_s(log->logString, sizeof(log->logString), "%d", nSample);
		log->FormatAndAppendLog(info_seed, info_extract, log->logString);
	}

	//һ���ܼ�����������ǿ�Ʊ�����־
	if(log != NULL)
		log->SaveLog();

	delete[] lpRecordBuf;
	return TRUE;
}

//����ֻ��ȡ���ݣ�����ʱ�䴦��
// int CSeedFile::DcmpExtrStationData( int index, int numerator, int denominator)
// {
// 	int nSample;
// 	FREQ extfreq;
// 	Station* lpCurnt;
// 
// 	//��ʾ�ж��ļ�����Ч��
// 	if(!seed->IsMapFileValid())
// 		return FALSE;
// 
// 	lpCurnt = GetStationInfo(index);
// 	if(lpCurnt == NULL)
// 		return FALSE;
// 
// 	//��������¼����ȡ���ĵ�����Ҫ�Ļ���
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
// 	//�����ȡ�ĵ���
// 	nSample = (int)(((__int64)lpCurnt->nSample)*lpCurnt->freq.denominator*extfreq.numerator/
// 		(((__int64)lpCurnt->freq.numerator)*extfreq.denominator)) + 1;
// 
// 	int * lpRecordBuf;
// 	int* lpDest;
// 	FRACTION extStep, fbegin, fend;
// 	int nRecordSample, nExtractSample;
// 	int nBufSize;
// 
// 	//�����Ϣ
// 	memcpy(&lpCurnt->datatime, &lpCurnt->time, sizeof(SYSTEMTIME));
// 	lpCurnt->datatime.wDayOfWeek = 0;
// 	lpCurnt->dataFreq.numerator = extfreq.numerator;
// 	lpCurnt->dataFreq.denominator = extfreq.denominator;
// 	//�ͷŵ����е����ݿռ�
// 	if(lpCurnt->pData != NULL)
// 		delete lpCurnt->pData;
// 	lpCurnt->pData = NULL;
// 	lpCurnt->bufitemcnt = 0;
// 	lpCurnt->iDataCnt = 0;
// 
// 	//���·���ռ䣬�����һ���ռ�
// 	nBufSize = m_step*8;
// 	lpRecordBuf = new int[nBufSize];	//���ѹ����Ϊ7������
// 	lpCurnt->pData = new int[nSample + 1];
// 	lpCurnt->bufitemcnt = nSample + 1;
// 	lpDest = lpCurnt->pData;
// 	//�����ȡ��������
// 	extStep.numerator = extfreq.denominator*lpCurnt->freq.numerator*1000;//ʹ�ú��뵥λ
// 	extStep.denominator = extfreq.numerator*lpCurnt->freq.denominator*1000;//ʹ�ú��뵥λ
// 	extStep.intpart = extStep.numerator/extStep.denominator;
// 	extStep.numerator -= extStep.intpart*extStep.denominator;
// 
// 	//׼������
// 	char* lpRecord;
// 	char* lpMax;
// 	//�����ʼƫ��
// 	fbegin.intpart = 0;
// 	fbegin.numerator = 0;
// 	fbegin.denominator = extStep.denominator;
// 	//�����ֹƫ��
// 	fend.intpart = -1;//���ݳ�ȡ����������
// 	nSample = 0;
// 	lpRecord = lpCurnt->lpAddr;
// 	lpMax = lpRecord + lpCurnt->iRecordLen - m_step;
// 	while(lpRecord <= lpMax)
// 	{
// 		nRecordSample = DecompOneRecord(lpRecord, lpRecordBuf, nBufSize);
// 		if(nRecordSample < 0)//��ѹ����
// 			break;
// 		//��ȡ
// 		nExtractSample = ExtractOneDataRecord(lpRecordBuf, nRecordSample, 
// 				&fbegin, &fend, &extStep, lpDest);
// 		//��¼�鵽�ĵ���
// 		nSample += nExtractSample;
// 		//����Ӧ���Ȱ����ݳ�ȡ������һ�������У�
// 		//�ټ���Ƿ����㹻���ڴ�д���ݣ�û�еĻ�ֱ�ӽ���
// 		//��Ч�ʿ��ǣ��������ã����⣬Ӧ�ò���������ִ���
// 		//��Ϊ��ѹ���ص����ݵ��������ļ��е�Ԥ��ֵ��
// 		//���ҳ�ȡ��������Ԥ��ʱ��ͻ᷵�ء�
// // 		if(nSample > lpCurnt->bufitemcnt)//����ڴ�
// // 			break;
// 		//����������
// 		lpDest += nExtractSample;
// 		//������¼��
// 		lpRecord += m_step;
// 		
// 		//������һ����Ŀ�ʼƫ��
// 		fbegin.intpart -= nRecordSample;
// 
// 		m_position ++;//��¼��ǰ����
// 	}
// 
// 	lpCurnt->iDataCnt = nSample;
// 	if(log != NULL)
// 	{
// 		sprintf_s(log->logString, sizeof(log->logString), "%d", nSample);
// 		log->FormatAndAppendLog(info_seed, info_extract, log->logString);
// 	}
// 
// 	//һ���ܼ�����������ǿ�Ʊ�����־
// 	if(log != NULL)
// 		log->SaveLog();
// 
// 	delete[] lpRecordBuf;
// 	return TRUE;
// }