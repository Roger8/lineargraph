
#pragma once 

#include "ExportMacro.h"

#include "Export.h"
#include "BinHeaderFormat.h"

struct CtrlParam;

#define SAMPLE_DATA_BUF			(1024*100)


class CBinExport : public CExport
{
public:
	CBinExport();
	~CBinExport();

	int Create(const WCHAR* pfile, void* ppara);
	int ExportData(int* pdata, int itype, int icnt, void* ppara);
	int Close(void* ppara);
	int IsExportValid();

private:
	HANDLE m_hfile;
	BIN_HEADER_LOCAL m_binheader;

	void FormatBinHeader(CtrlParam* pinfo);
};
