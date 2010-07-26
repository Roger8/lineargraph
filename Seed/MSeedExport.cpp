
#include "MSeedExport.h"

CMSeedExport::CMSeedExport(){
	m_hfile = INVALID_HANDLE_VALUE;
}

CMSeedExport::~CMSeedExport(){
	Close(NULL);
}

//ignore the second parameter 
int CMSeedExport::Create( const WCHAR* pfile, void* ppara )
{
	Close(NULL);

	m_hfile = CreateFile(pfile, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, 
		NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(m_hfile == INVALID_HANDLE_VALUE)
		return FALSE;
	else
		return TRUE;
}

//ignore the parameter
int CMSeedExport::Close( void* ppara )
{
	CloseHandle(m_hfile);
	m_hfile = INVALID_HANDLE_VALUE;
	return TRUE;
}

//only the first three parameters are useful 
//pdata: the address of the mini-seed data 
//icnt: the bytes size of the mini-seed data 
//itype
int CMSeedExport::ExportData( int* pdata, int itype, int icnt, void* ppara )
{
	//if the itype is used, we will write the mini-seed file here
	DWORD writed;
	if(m_hfile == INVALID_HANDLE_VALUE)
		return -1;
	if(0 == WriteFile(m_hfile, pdata, icnt, &writed, NULL))
		return -1;
	else
		return icnt;
}

int CMSeedExport::IsExportValid()
{
	return (m_hfile != INVALID_HANDLE_VALUE);
}