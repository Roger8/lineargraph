
//导出Seed manager类
#define CSEED_FILE_CLASS_IMPLEMENT

#include <Windows.h>
#include "rdseedlib.h"
#include "Convert.h"
#include "SeedFile.h"
#include "SeedList.h"

#include "TxtExport.h"
#include "BinExport.h"
#include "MSeedExport.h"
#include "SeedExport.h"

#include "SeedLog.h"
#include "OperateStruct.h"
#include "SeedManager.h"

void CSeedManager::SetLogFile( CSeedLog* plog )
{
	m_plog = plog;
}

CSeedManager::CSeedManager()
{
	m_plog = NULL;
	m_freq.numerator = 1;
	m_freq.denominator = 1;
	m_list = new CSeedList;
	m_defFormat = DE_STEIM1;
	m_exp = NULL;
	m_filetype = EXPORT_TXT;
	m_range = 0;
	m_pos = 0;
	m_text = 0;
}

CSeedManager::~CSeedManager()
{
	m_list->Free();
	delete m_list;
}

extern const WCHAR *extension[];//from the SeedFile.cpp

void CSeedManager::GetMergeFileName( const WCHAR* path, WCHAR* pfile, CtrlParam* pctrl)
{
	float freq;
	WCHAR* frequnit;
	if(m_filetype > 4)
		m_filetype = 4;
	freq = ((float)pctrl->numerator)/((float)pctrl->denominator);
	if(freq >= 1.0)
	{
		frequnit = L"Hz";
	}
	else
	{
		freq = float(1.0/freq);
		frequnit = L"s";
	}
	if(m_filetype == EXPORT_TXT || m_filetype == EXPORT_BIN)
		swprintf_s(pfile, MAX_PATH, L"%s\\%s.%s.%f%s.%04d%02d%02d%02d%02d%02d.%s", 
			path, pctrl->station, pctrl->channel, freq, frequnit, 
			pctrl->tstart.wYear, pctrl->tstart.wMonth, pctrl->tstart.wDay, 
			pctrl->tstart.wHour, pctrl->tstart.wMinute, pctrl->tstart.wSecond, 
			extension[m_filetype]);
	else
		swprintf_s(pfile, MAX_PATH, L"%s\\%s.%s.merge.%04d%02d%02d%02d%02d%02d.%s", 
			path, pctrl->station, pctrl->channel,
			pctrl->tstart.wYear, pctrl->tstart.wMonth, pctrl->tstart.wDay, 
			pctrl->tstart.wHour, pctrl->tstart.wMinute, pctrl->tstart.wSecond, 
			extension[m_filetype]);
}


int CSeedManager::SetMergeParam( const WCHAR* pstation, const WCHAR* pchannel )
{
	wcscpy_s(m_station, sizeof(m_station)/sizeof(WCHAR), pstation);
	wcscpy_s(m_channel, sizeof(m_channel)/sizeof(WCHAR), pchannel);
	m_list->Free();
	return TRUE;
}

int CSeedManager::AddMergeFile( const WCHAR* pfile )
{
	//对所有的文件进行处理
	CSeedFile* seed;
	Station* pcurt;
	int index;
	seed = new CSeedFile;
	if(seed->open(pfile, m_plog) == TRUE)
	{
		index = seed->GetStationIndex(m_station, m_channel);
		if(index != -1)
		{
			seed->suspend();//释放原始seed文件
			//添加seed单元
			pcurt = seed->GetStationInfo(index);
			m_list->AddElement(seed, index, pcurt);
			if(m_plog != NULL && m_plog->IsDetailLog())
			{
				sprintf_s(m_plog->logString, sizeof(m_plog->logString), 
					"sequence:%d. start time:%4d-%2d-%2d,%2d:%2d:%2d", 
					m_list->GetElementCnt(), 
					pcurt->time.wYear, pcurt->time.wMonth, pcurt->time.wDay, 
					pcurt->time.wHour, pcurt->time.wMinute, pcurt->time.wSecond);
				m_plog->FormatAndAppendLog(info_seed, info_add_station, m_plog->logString);
			}
			return TRUE;
		}
		else
			seed->close();
	}
	delete seed;
	return FALSE;
}
int CSeedManager::GetMergeFilesInfo( SYSTEMTIME* pstart, SYSTEMTIME* pend, int* pnumerator, 
									int* pdenominator, __int64* pabsent )
{
	FREQ minfreq;//记录最小频率
	__int64 absent;//记录空缺的时间值
	FRACTION tmpfraction;
	MergeSeedCell* pcurt, *ppre;
	ppre = m_list->NextElement(NULL);
	//检查合法性
	if(ppre == NULL)
		return FALSE;

	//记录参数
	memcpy(&m_start, &ppre->sinfo->time, sizeof(SYSTEMTIME));//
	//返回起始时间
	if(pstart != NULL)
		memcpy(pstart, &m_start, sizeof(SYSTEMTIME));
	absent = 0;
	minfreq.numerator = ppre->sinfo->freq.numerator;
	minfreq.denominator = ppre->sinfo->freq.denominator;
	pcurt = m_list->NextElement(ppre);
	//计算时间空缺，记录最小采样率
	while(pcurt != NULL)
	{
		//记录最小采样率
		if( (minfreq.numerator*pcurt->sinfo->freq.denominator - 
			minfreq.denominator*pcurt->sinfo->freq.numerator) > 0)
		{
			minfreq.numerator = pcurt->sinfo->freq.numerator;
			minfreq.denominator = pcurt->sinfo->freq.denominator;
		}

		//计算下一个seed文件中数据的开始时间
		memcpy(&m_end, &ppre->sinfo->tend, sizeof(SYSTEMTIME));
		SystemTimeAddFraction(&m_end, &m_end, 
			(__int64)(ppre->sinfo->freq.denominator),
			(__int64)ppre->sinfo->freq.numerator);
		//比较时间有序的前后seed文件是否有时间空缺
		if(CmpSystemTimeS(&m_end, &pcurt->sinfo->time) < 0)
		{
			//计算时间空缺
			SystemTimeSubSystemTimeFraction(&pcurt->sinfo->time, &m_end, 
				&tmpfraction.numerator, &tmpfraction.denominator);
			absent += tmpfraction.numerator;//记录空缺的毫秒值
		}
		ppre = pcurt;
		pcurt = m_list->NextElement(pcurt);
	}
	//计算结尾数据点的时间
	memcpy(&m_end, &ppre->sinfo->tend, sizeof(SYSTEMTIME));//记录结束时间
	//记录参数
	m_freq.numerator = minfreq.numerator;//记录最低频率
	m_freq.denominator = minfreq.denominator;//记录最低频率
	//返回结束时间
	if(pend != NULL)
		memcpy(pend, &m_end, sizeof(SYSTEMTIME));
	//返回频率
	if(pnumerator != NULL && pdenominator != NULL)
	{
		*pnumerator = m_freq.numerator;
		*pdenominator = m_freq.denominator;
	}
	//返回缺少点数
	if(pabsent != NULL)
		*pabsent = absent;
	return TRUE;
}

void CSeedManager::CleanMergeFileList()
{
	m_list->Free();
}

int CSeedManager::AdjustValidParam( CtrlParam* pctrl )
{
	if(m_list->GetElementCnt() <= 0 || pctrl == NULL)
		return FALSE;
	GetMergeFilesInfo(NULL, NULL, NULL, NULL, NULL);//get the information of the list of files 
	//提取参数
	if(CmpSystemTimeS(&pctrl->tstart, &m_start) < 0)
		memcpy(&pctrl->tstart, &m_start, sizeof(SYSTEMTIME));
	else if(CmpSystemTimeS(&pctrl->tend, &m_end) > 0)
		memcpy(&pctrl->tstart, &m_end, sizeof(SYSTEMTIME));
	//
	if(CmpSystemTimeS(&pctrl->tend, &m_end) > 0)
		memcpy(&pctrl->tend, &m_end, sizeof(SYSTEMTIME));
	else if(CmpSystemTimeS(&pctrl->tend, &m_start) < 0)
		memcpy(&pctrl->tend, &m_start, sizeof(SYSTEMTIME));
	//
	if(CmpSystemTimeS(&pctrl->tstart, &pctrl->tend) >= 0)
		memcpy(&pctrl->tstart, &pctrl->tend, sizeof(SYSTEMTIME));
	//提取抽取频率
	if( ( pctrl->numerator <= 0) || (pctrl->denominator <= 0) || 
		((m_freq.numerator*(pctrl->denominator)- m_freq.denominator*(pctrl->numerator)) < 0))
	{
		pctrl->numerator = m_freq.numerator;
		pctrl->denominator = m_freq.denominator;
	}
	return TRUE;
}
//使用到numerator、denominator、type、ctrl、起始时间，终止时间
int CSeedManager::CreateOutputFile( const WCHAR* path, CtrlParam* pctrl )
{
	int ret = TRUE;
	WCHAR filename[MAX_PATH];
	//CtrlParam ctrl;
	CloseOutputFile();
	if(m_list->GetElementCnt() <= 0)
		return FALSE;
// 	memcpy(&ctrl, pctrl, sizeof(CtrlParam));
// 	wcscpy_s(ctrl.station, sizeof(ctrl.station)/sizeof(WCHAR), m_station);
// 	wcscpy_s(ctrl.channel, sizeof(ctrl.channel)/sizeof(WCHAR), m_channel);
	wcscpy_s(pctrl->station, sizeof(pctrl->station)/sizeof(WCHAR), m_station);
	wcscpy_s(pctrl->channel, sizeof(pctrl->channel)/sizeof(WCHAR), m_channel);
	if(FALSE == AdjustValidParam(pctrl))
		return FALSE;
	switch (pctrl->type)
	{
	case EXPORT_TXT:
		m_exp = new CTxtExport;
		break;
	case EXPORT_BIN:
		m_exp = new CBinExport;
		break;
	case EXPORT_MSEED:
		m_exp = new CMSeedExport;
		break;
	case EXPORT_SEED:
		m_exp = new CSeedExport;
		break;
	default:
		return FALSE;
	}
	m_filetype = pctrl->type;
	GetMergeFileName(path, filename, pctrl);
	char szfilename[MAX_PATH];
	WcharNameToCharName(szfilename, MAX_PATH, filename, wcslen(filename));
	if(FALSE == m_exp->Create(filename, pctrl))
	{
		if(m_plog != NULL)
		{
			DWORD res = GetLastError();
			sprintf_s(m_plog->logString, sizeof(m_plog->logString), "file name:%s. error code:%d", 
				szfilename, res);
			m_plog->FormatAndAppendLog(error_seed, erro_export_file_create_failed, m_plog->logString);
		}
		delete m_exp;
		m_exp = NULL;
		ret = FALSE;
	}else if(m_plog != NULL)
	{
		SYSTEMTIME loc;
		GetLocalTime(&loc);
		sprintf_s(m_plog->logString, sizeof(m_plog->logString), "file name:%s."
			"at time:%4d-%2d-%2d,%2d:%2d:%2d", 
			szfilename, 
			loc.wYear, loc.wMonth, loc.wDay, 
			loc.wHour, loc.wMinute, loc.wSecond);
		m_plog->FormatAndAppendLog(info_seed, info_merge_file_created, m_plog->logString);
	}

	//保存日志
	if(m_plog != NULL)
		m_plog->SaveLog();

	return ret;
}

int CSeedManager::WriteOutputFile( CtrlParam* pctrl )
{
// 	CtrlParam ctrl;
	int ret = TRUE;

	if(m_list->GetElementCnt() <= 0 || m_exp == NULL)
		return FALSE;
// 	memcpy(&ctrl, pctrl, sizeof(CtrlParam));
	if(FALSE == AdjustValidParam(pctrl))
		return FALSE;

	switch (pctrl->type)
	{
	case EXPORT_TXT:
	case EXPORT_BIN:
		if(FALSE == OutputDcmpData(pctrl))
			ret  =FALSE;
		break;
	case EXPORT_MSEED:
	case EXPORT_SEED:
		if (FALSE == OutputOriginalData(pctrl))
			ret = FALSE;
		break;
	default:
		return FALSE;
	}
	if(m_plog != NULL)
	{
		m_plog->FormatAndAppendLog(info_seed, info_data_export, "");
		m_plog->SaveLog();
	}
	return ret;
}


int CSeedManager::AddLostData( SYSTEMTIME* pcurt, SYSTEMTIME* plast, int addval, CtrlParam* pctrl )
{
	int needcnt = 0;
	if(pctrl->type == EXPORT_MSEED || pctrl->type == EXPORT_SEED)
		return 0;
	if (CmpSystemTimeS(pcurt, plast) > 0)
	{//需要补数
		__int64 timeinterval, tmp;
		//补数
		SystemTimeSubSystemTimeFraction(pcurt, plast, &timeinterval, &tmp);
		needcnt = int(timeinterval*pctrl->numerator/(tmp*pctrl->denominator));
		//取得需要填补的数值
		if(pctrl->addval != ADD_FIRST_NEW_DATA)
			addval = pctrl->addval;
		if( -1 == m_exp->ExportData((int*)addval, SAMPLE_DATA, needcnt, pctrl))
			return -1;
		//记录日志
		if(m_plog != NULL)
		{
			double dfreq;
			dfreq = ((float)pctrl->numerator)/((float)pctrl->denominator);
			sprintf_s(m_plog->logString, MAX_LOG_STR_SIZE, "total added samples:%d"
				"added sample value:%d"
				"start time:%04d%02d%02d%02d%02d%02d"
				"end time:%04d%02d%02d%02d%02d%02d"
				"sample rate:%fHz", 
				needcnt, addval, 
				plast->wYear, plast->wMonth, plast->wDay, 
				plast->wHour, plast->wMinute, plast->wSecond, 
				pcurt->wYear, pcurt->wMonth, pcurt->wDay, 
				pcurt->wHour, pcurt->wMinute, pcurt->wSecond, 
				dfreq);
			m_plog->FormatAndAppendLog(info_seed, info_add_value, m_plog->logString);
		}
	}
	return needcnt;
}
int CSeedManager::CloseOutputFile()
{
	if(m_exp != NULL)
	{
		m_exp->Close(NULL);
		delete m_exp;
		m_exp = NULL;
	}
	return TRUE;
}
//这里只用到了pctrl的station，channel，ctrl，type，numerator，denominator
int CSeedManager::DcmpFileBySpecifiedInfo( const WCHAR* path, const WCHAR* pfile, CtrlParam* pctrl, int specify )
{
	int ret, ido;
	CSeedFile seed;
	Station* pcurt;
	if(FALSE == seed.open(pfile, m_plog))
		return FALSE;
	ret = TRUE;
	for(int i = 0; i < seed.GetStationCount(); i++)
	{
		pcurt = seed.GetStationInfo(i);
		if(pcurt != NULL)
		{
			switch (specify)
			{
			case DECOMP_SPECIFIED_STATION:
				ido = 
					(memcmp(pctrl->station, pcurt->station, wcslen(pctrl->station)) == 0) ? 1 : 0;
				break;
			case DECOMP_SPECIFIED_CHANNEL:
				ido = (pctrl->channel[2] == pcurt->channel[2]) ? 1 : 0;
				break;
			case DECOMP_SPECIFIED_STAT_CHN:
				ido = 
					((memcmp(pctrl->station, pcurt->station, wcslen(pctrl->station)) == 0) &&
					(pctrl->channel[2] == pcurt->channel[2]) ) ? 1 : 0;
				break;
			default:
				ret = FALSE;
				ido = 0;
				break;
			}
			if(ret == FALSE)
				break;
			if(ido)
			{
				//just use the frequency parameter , ignore the time 
				if (pctrl->type == EXPORT_TXT || pctrl->type == EXPORT_BIN)
				{
					if(FALSE == seed.DcmpExtrStationDataByTime(i, NULL, NULL, \
											pctrl->numerator, pctrl->denominator))
											continue;
				}
				if(FALSE == seed.ExportData(path, i, pctrl))
					continue;
			}
		}
	}
	seed.close();
	return ret;
}

int CSeedManager::AddFirstMergeFile( const WCHAR* pfile )
{
	CSeedFile seed;
	Station* pcurt;
	if(seed.open(pfile, m_plog) == FALSE)
		return FALSE;
	pcurt = seed.GetStationInfo(0);
	if(pcurt == NULL)
		return FALSE;
	wcscpy_s(m_station, sizeof(m_station)/sizeof(WCHAR), pcurt->station);
	wcscpy_s(m_channel, sizeof(m_channel)/sizeof(WCHAR), pcurt->channel);
	m_list->Free();
	return TRUE;
}

int CSeedManager::OutputDcmpData( CtrlParam* pctrl )
{
	int ret = TRUE;
	//开始解压数据
	MergeSeedCell *pcurt;
	CtrlParam loc;
	memcpy(&loc, pctrl, sizeof(CtrlParam));
	pcurt = m_list->NextElement(NULL);
	//进度条
	int pos = 0;
	m_position = 0;
	if(m_range != 0)
		m_range(m_list->GetElementCnt());
	if(m_text != 0)
	{
		if(pctrl->type == EXPORT_TXT)
			m_text((L"正在合并数据到TXT"), FALSE);
		if(pctrl->type == EXPORT_BIN)
			m_text((L"正在合并数据到BIN"), FALSE);
		else
			m_text((L"正在合并数据"), FALSE);
	}
	
	//开始解压
	while( pcurt != NULL)
	{
		if(m_pos != 0)//进度条更新
			m_pos(pos++);
		pcurt->seed->SetDefaultFormat(m_defFormat);
		pcurt->seed->SetLogFile(m_plog);
		//重新加载并分析文件，
		if(FALSE == pcurt->seed->resume())
		{
			ret = FALSE;
			break;
		}
		//按照要求对数据处理,解压抽取数据
		if(FALSE == pcurt->seed->DcmpExtrStationDataByTime(pcurt->index, &loc.tstart, 
			&loc.tend, loc.numerator, loc.denominator) )
			continue;
		//如果提取到了数据，则写入
		if (pcurt->sinfo->iDataCnt > 0)
		{
			//补数
			if (-1 == AddLostData(&pcurt->sinfo->datatime, &loc.tstart, *pcurt->sinfo->pData, &loc))
			{
				if(m_plog != NULL)
					m_plog->FormatAndAppendLog(error_seed, erro_data_export, "");
				ret = FALSE;
				break;
			}
			memcpy(&loc.tstart, &pcurt->sinfo->datatime, sizeof(SYSTEMTIME));
			if (-1 == m_exp->ExportData(pcurt->sinfo->pData, ARRAY_DATA, 
				pcurt->sinfo->iDataCnt, &loc) )
			{
				if(m_plog != NULL)
					m_plog->FormatAndAppendLog(error_seed, erro_data_export, "");
				ret = FALSE;
				break;
			}
			//计算下一个seed文件的第一个数据的开始时间
			memcpy(&loc.tstart, &pcurt->sinfo->datatime, sizeof(SYSTEMTIME));
			SystemTimeAddFraction(&loc.tstart, &loc.tstart, 
				pcurt->sinfo->dataFreq.denominator*pcurt->sinfo->iDataCnt, 
				pcurt->sinfo->dataFreq.numerator);
		}
		//准备下一个seed文件
		pcurt->seed->suspend();////////////////////////////////释放文件
		pcurt->seed->FreeStationData();///////////////////////////释放解压得到的数据缓冲
		pcurt = m_list->NextElement(pcurt);
		//释放前面已经处理过的seed文件
		//m_list->DelFirstElement();///////////////////////////////
		m_position ++;//记录进度
	}
	//补数
	if ( -1 == AddLostData(&loc.tend, &loc.tstart, 0, &loc))
	{
		if(m_plog != NULL)
			m_plog->FormatAndAppendLog(error_seed, erro_data_export, "");
		ret = FALSE;
	}
	return ret;
}

int CSeedManager::OutputOriginalData( CtrlParam* pctrl )
{
	int ret = TRUE;
	//开始解压数据
	MergeSeedCell *pcurt;
	pcurt = m_list->NextElement(NULL);
	m_position = 0;
	//进度条
	int pos = 0;
	m_position = 0;
	if(m_range != 0)
		m_range(m_list->GetElementCnt());
	if(m_text != 0)
	{
		if(pctrl->type == EXPORT_MSEED)
			m_text((L"正在合并数据到MSEED"), FALSE);
		if(pctrl->type == EXPORT_BIN)
			m_text((L"正在合并数据到SEED"), FALSE);
		else
			m_text((L"正在合并数据"), FALSE);
	}

	//开始解压
	while( pcurt != NULL)
	{
		if(m_pos != 0)//进度条更新
			m_pos(pos++);
		pcurt->seed->SetDefaultFormat(m_defFormat);
		pcurt->seed->SetLogFile(m_plog);
		//重新加载并分析文件，
		if(FALSE == pcurt->seed->resume())
		{
			ret = FALSE;
			break;
		}
		if (-1 == m_exp->ExportData((int*)pcurt->sinfo->lpAddr, ARRAY_DATA, 
			pcurt->sinfo->iRecordLen, pctrl) )
		{
			if(m_plog != NULL)
				m_plog->FormatAndAppendLog(error_seed, erro_data_export, "");
			ret = FALSE;
			break;
		}
		//准备下一个seed文件
		pcurt->seed->suspend();////////////////////////////////释放文件
		pcurt = m_list->NextElement(pcurt);
		//释放前面已经处理过的seed文件
		//m_list->DelFirstElement();///////////////////////////////
		m_position ++;//记录进度
	}
	return ret;
}

void CSeedManager::SetDefaultFormat( int format )
{
	if(format >= 0)
		m_defFormat = format;
}

void CSeedManager::SetProgressParam( ProgressRange range, ProgressPos pos, PaneText text )
{
	m_range = range;
	m_pos = pos;
	m_text = text;
}