
#include "SeedExport.h"

CSeedExport::CSeedExport(){}

CSeedExport::~CSeedExport(){
	Close(NULL);
}

int CSeedExport::Create( const WCHAR* pfile, void* ppara )
{
	return FALSE;
}

int CSeedExport::Close( void* ppara )
{
	return TRUE;
}

int CSeedExport::ExportData( int* pdata, int itype, int icnt, void* ppara )
{
	return TRUE;
}

int CSeedExport::IsExportValid()
{
	return FALSE;
}
