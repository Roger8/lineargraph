
#pragma once

#include "ExportMacro.h"
#include "Export.h"

#define MAX_TXT_BUF_LEN		256

#define TXT_EXPORT_ARRAY	0
#define TXT_EXPORT_SAMPLE	1

class COutFile;


// class SEED_FILE_CLASS_LIB CTxtExport
class CTxtExport : public CExport
{
public:
	CTxtExport();
	~CTxtExport();
	//
	int Create(const WCHAR* pfile, void* ppara);
	int ExportData(int* pdata, int itype, int icnt, void* ppara);
	int Close(void* ppara);

	int IsExportValid();

	//
	int m_position;
private:
	//COutFile 都是由CTxtExport自己创建的，始终存在，没有必要使用前检查其有效性
	COutFile* m_txt;
// 	//CSeedLog是由外界传入的，使用前必需检查其有效性
// 	CSeedLog* log;//log是全局的，仅一个
	SYSTEMTIME m_time;
	int Print_asm(char* lpBuf, int integer);
	void GetNextSampleTime(int* interval);
};
