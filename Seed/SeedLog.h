
#ifndef _SEED_LOG_H_
#define _SEED_LOG_H_

#include <map>
#include <string>

#include "ExportMacro.h"

class COutFile;

//枚举定义
enum LogType{
	info_seed,
	warning_seed,
	error_seed
};
//枚举定义
enum InfoCode{
	info_open,
	info_file,
	info_station,
	info_close,
	info_unpack_record,
	info_data_ok,
	info_start_unpack,
	info_header_export,
	info_data_export,
	info_extract,
	info_add_value,
	info_add_station,
	info_merge_file_created,

	warn_data_differ,
	warn_data_exceed,
	warn_data_missing,

	erro_data_attr,
	erro_unknow_code,
	erro_header_create_failed,
	erro_export_file_create_failed,
	erro_data_export
};

#define MAX_LOG_STR_SIZE	1024*2
#define LOG_FILE_NAME		L"seed.log"


// #ifdef CSEED_FILE_CLASS_IMPLEMENT
// class __declspec(dllexport) COutFile;
// #endif

class SEED_FILE_CLASS_LIB CSeedLog
{
public:
	CSeedLog();
	~CSeedLog();

	//打开日志文件
	int OpenLog();
	int CloseLog();
	int EmptyLog();
	int SaveLog();
	int AppendLog(char* lpstring, int iLen);
	int FormatAndAppendLog(LogType type, InfoCode code, char* lpString);

	int IsLogValid();
	int SetDetailLog(int idetail);
	int IsDetailLog();

	int GetLogFileName(WCHAR * lpName, int iLen);
	char logString[MAX_LOG_STR_SIZE];

private:
	//COutFile map都是由CSeedLog自己创建的，始终存在，没有必要使用前检查其有效性
	COutFile* logFile;
	int m_detail;
	WCHAR fileName[MAX_PATH];

	std::map<InfoCode, std::string>* logmap;
	void InitLogMap();
};



#endif