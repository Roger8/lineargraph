
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
	//COutFile ������CTxtExport�Լ������ģ�ʼ�մ��ڣ�û�б�Ҫʹ��ǰ�������Ч��
	COutFile* m_txt;
// 	//CSeedLog������紫��ģ�ʹ��ǰ����������Ч��
// 	CSeedLog* log;//log��ȫ�ֵģ���һ��
	SYSTEMTIME m_time;
	int Print_asm(char* lpBuf, int integer);
	void GetNextSampleTime(int* interval);
};
