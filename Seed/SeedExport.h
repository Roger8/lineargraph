
#pragma once 

#include "Export.h"

class CSeedExport : public CExport
{
public:
	CSeedExport();
	~CSeedExport();
	int Create(const WCHAR* pfile, void* ppara);
	int Close(void* ppara);
	int ExportData(int* pdata, int itype, int icnt, void* ppara);
	int IsExportValid();
};