
#ifndef _SEED_FILE_H_
#define _SEED_FILE_H_

#include "ExportMacro.h"

class CSeedLog;
class CMapFile;
class CStationList;
struct Station;
struct FRACTION;
struct CtrlParam;

class SEED_FILE_CLASS_LIB CSeedFile
{
public:
	CSeedFile();
	~CSeedFile();

	//打开文件，校验，提取所有的台站分量信息
	int open(const WCHAR* pFileName, CSeedLog* lpLog);
	//关闭文件，释放所有的缓存空间
	int close();
	//提取一个台站的数据,文件数据预览时使用,这里不检查数据的缺失和多余，统一放在一起
	int DecompStation(int index);
	//返回台站的数量
	int GetStationCount();
	//返回台站的信息
	Station* GetStationInfo(int index);

	//挂起状态，仅仅释放文件占用内存
	//可以重复挂起
	void suspend();
	//回复打开状态，
	//可以重复唤醒
	int resume();

	//导出各种数据
	int ExportData(const WCHAR* ppath, int index, CtrlParam* pctrl);
	int ExportHeaderInfo(const WCHAR* lpPath);

	//在特定的时间段内，抽取一个台站分量的数据点
	int DcmpExtrStationDataByTime(int index, SYSTEMTIME* lpStart, SYSTEMTIME* lpEnd, 
			int numerator, int denominator);
	//抽取一个台站分量的数据点，忽略时间的影响
// 	int DcmpExtrStationData(int index, int numerator, int denominator);

	//提取station list中的信息
	int GetStationIndex(const WCHAR* pstation, const WCHAR* pchannel);

	//释放解压的数据
	int FreeStationData();
	//设置日志
	int SetLogFile(CSeedLog* lpLog);
	//针对早期标准seed格式
	void SetDefaultFormat(int iformat);

	//进度跟踪变量
	int m_position;

private:
	//CFapFile CStationList 是由CSeedFile自己创建的，始终存在
	//CSeedLog是由外界传入的，可能为空，所以，log操作前必需检查log的有效性
	CMapFile* seed;
	CStationList* list;
	CSeedLog* log;	//全局只有一个log类，有外界传入
	int m_step, m_swap;
	int m_defFormat;
	WCHAR path[MAX_PATH];
	WCHAR name[MAX_PATH];

	//提取一个record的数据
	int DecompOneRecord(char* lpBase, int* lpData, int iBufLen);
	//按特定频率抽取一块里面的数据点
	int ExtractOneDataRecord(const int* lpData, int iCnt, FRACTION* pbegin, 
		FRACTION* pend, FRACTION* pstep, int* lpDest);

	int GetExportFileName(const WCHAR* lpPath, WCHAR* lpName, int iType, Station* lpst);
};

#endif