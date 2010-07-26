
#pragma once 

#include "ExportMacro.h"
#include "OperateStruct.h"

#include <string>

class CSeedLog;
class CSeedList;
struct MergeSeedCell;
struct CtrlParam;
class CExport;


#define MAX_INT_VALUE			0x7fffffff
#define ADD_FIRST_NEW_DATA		MAX_INT_VALUE

#define DECOMP_SPECIFIED_STATION	0
#define DECOMP_SPECIFIED_CHANNEL	1
#define DECOMP_SPECIFIED_STAT_CHN	2



class SEED_FILE_CLASS_LIB CSeedManager
{
public:
	typedef void (*ProgressRange)(int);
	typedef void (*ProgressPos)(int);
	typedef void (*PaneText)(const std::wstring&, bool);
	CSeedManager();
	~CSeedManager();
	//特别声明，这里的文件名数组参数的数组元素大小必须为windows.h中定义的MAX_PATH的大小
	//同样的，路径也必须是这样的MAX_PATH大小

	//analyze the files to be merged 
	int SetMergeParam(const WCHAR* pstation, const WCHAR* pchannel);

	int AddFirstMergeFile(const WCHAR* pfile);//////////////////////////////////

	int AddMergeFile(const WCHAR* pfile);
	int GetMergeFilesInfo(SYSTEMTIME* pstart, SYSTEMTIME* pend, int* pnumerator, 
		int* pdenominator, __int64* pabsent);
	void CleanMergeFileList();
	//output the export file 
	int AdjustValidParam(CtrlParam* pctrl);
	int CreateOutputFile(const WCHAR* path, CtrlParam* pctrl);
	int WriteOutputFile(CtrlParam* pctrl);
	int CloseOutputFile();

	//special decompress 
	int DcmpFileBySpecifiedInfo(const WCHAR* path, const WCHAR* pfile, CtrlParam* pctrl, int specify);

	void SetLogFile(CSeedLog* plog);
	void SetDefaultFormat(int format);
	void SetProgressParam(ProgressRange range, ProgressPos pos, PaneText text);
	//进度跟踪变量
	int m_position;
	int m_whole;

private:
	CSeedList* m_list;
	CSeedLog* m_plog;
	int m_defFormat;
	SYSTEMTIME m_start;
	SYSTEMTIME m_end;
	CExport* m_exp;
	int m_filetype;
	FREQ m_freq;
	ProgressRange m_range;
	ProgressPos m_pos;
	PaneText m_text;
	WCHAR m_station[STATION_STR_LEN];
	WCHAR m_channel[CHANNEL_STR_LEN];

	void GetMergeFileName(const WCHAR* path, WCHAR* pfile, CtrlParam* pctrl);

	int OutputDcmpData(CtrlParam* pctrl);
	int OutputOriginalData(CtrlParam* pctrl);
	int AddLostData(SYSTEMTIME* pcurt, SYSTEMTIME* plast, int addval, CtrlParam* pctrl);
};
