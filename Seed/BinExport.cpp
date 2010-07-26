
#include <Windows.h>
#include "rdseedlib.h"

#include "BinExport.h"
#include "BinHeaderFormat.h"
#include "OperateStruct.h"

CBinExport::CBinExport(){
	m_hfile = INVALID_HANDLE_VALUE;
}

CBinExport::~CBinExport(){
	Close(NULL);
}

int CBinExport::Create( const WCHAR* pfile, void* ppara )
{
	DWORD writed;
	Close(NULL);

	m_hfile = CreateFile(pfile, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, 
		NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(m_hfile == INVALID_HANDLE_VALUE)
		return FALSE;
	FormatBinHeader((CtrlParam*)ppara);
	if( 0 == WriteFile(m_hfile, &m_binheader, sizeof(m_binheader), &writed, NULL))
	{
		Close(NULL);
		return FALSE;
	}
	return TRUE;
}
//parameter is not used 
int CBinExport::Close( void* ppara )
{
	CloseHandle(m_hfile);
	m_hfile = INVALID_HANDLE_VALUE;
	return TRUE;
}

//pdata: the address of the data buffer 
//itype: the data type
//icnt: the integer data count
//ppara: ignored 
//return the integer count writed into the file 
int CBinExport::ExportData( int* pdata, int itype, int icnt, void* ppara )
{
	int* pbuf;
	int cnt;
	int data;
	if(m_hfile == INVALID_HANDLE_VALUE || icnt < 0 )
		return -1;
	//sample data 
	if(itype != ARRAY_DATA)
	{
		DWORD writed;
		pbuf = new int[SAMPLE_DATA_BUF];
		data = (int)pdata;
		for(int i = 0; i < SAMPLE_DATA_BUF && i < icnt; i++)
			pbuf[i] = data;
		cnt = 0;
		for(int i = 0; i < icnt/SAMPLE_DATA_BUF; i++)
		{
			if(0 == WriteFile(m_hfile, pbuf, sizeof(int)*SAMPLE_DATA_BUF, &writed, NULL))
			{
				delete[] pbuf;
				return -1;
			}
			cnt += SAMPLE_DATA_BUF;
		}
		//write the left bytes 
		if( cnt < icnt && 
			(0 == WriteFile(m_hfile, pbuf, (icnt-cnt)*sizeof(int), &writed, NULL)))
		{
			delete[] pbuf;
			return -1;
		}
		delete[] pbuf;
		return icnt;
	}else{
		DWORD writed;
		if(0 == WriteFile(m_hfile, pdata, icnt*sizeof(int), &writed, NULL))
			return -1;
		else
			return icnt;
	}
}

int CBinExport::IsExportValid()
{
	return (m_hfile != INVALID_HANDLE_VALUE);
}


void CBinExport::FormatBinHeader( CtrlParam* pinfo )
{
	memset(&m_binheader, 0, sizeof(BIN_HEADER_LOCAL));
	wcharTochar(m_binheader.station, pinfo->station, wcslen(pinfo->station));
	wcharTochar(m_binheader.channel, pinfo->channel, wcslen(pinfo->channel));
	m_binheader.numerator = pinfo->numerator;
	m_binheader.denominator = pinfo->denominator;
	SystemTimeToFileTime(&pinfo->tstart, &m_binheader.tstart);
	strncpy(m_binheader.datatype, 
		BIN_FORMAT_SINT32, strlen(BIN_FORMAT_SINT32));
}