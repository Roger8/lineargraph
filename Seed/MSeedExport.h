
#pragma once 

#include "Export.h"

class CMSeedExport : public CExport
{
public:
	CMSeedExport();
	~CMSeedExport();

	int Create(const WCHAR* pfile, void* ppara);
	int ExportData(int* pdata, int itype, int icnt, void* ppara);
	int Close(void* ppara);
	int IsExportValid();
private:
	HANDLE m_hfile;
};
