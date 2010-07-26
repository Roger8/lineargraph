

#pragma once 

#include <windows.h>

#define ARRAY_DATA		0
#define SAMPLE_DATA		1

class CExport
{
public:
	CExport(){}
	virtual ~CExport(){}
	virtual int Create(const WCHAR* pfile, void* ppara) = 0;
	virtual int ExportData(int* pdata, int itype, int icnt, void* ppara) = 0;
	virtual int Close(void* ppara) = 0;

	virtual int IsExportValid() = 0;
};
