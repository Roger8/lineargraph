
//����log��
#define CSEED_FILE_CLASS_IMPLEMENT

#include <wchar.h>
#include "OutFile.h"

#include "SeedLog.h"

//��ʽ��Ϣ����
char* logTypeStr[] = {
	"Info ",
	"Warn ",
	"Error"
};

int CSeedLog::OpenLog()
{
	if(logFile->IsFileValid())//log�Ѿ���
		return TRUE;

	GetLogFileName(fileName, MAX_PATH);
	if(FALSE == logFile->CreateFile(fileName, OPEN_ALWAYS, MIN_BUFFER_SIZE_CO, MAX_LOG_STR_SIZE))
		return FALSE;
	//�����ļ�ָ�룬׼���������
	return logFile->AdjustFileToEnd();
}

int CSeedLog::GetLogFileName(WCHAR* lpName, int iLen)
{
	int n;
	::GetModuleFileNameW(::GetModuleHandle(NULL), lpName, iLen);
	
	n = wcslen(lpName);
	for(; n > 0; n--)
	{
		if(lpName[n] == '\\')
			break;
	}
	wcscpy_s(&lpName[n+1], (MAX_PATH-n-1), LOG_FILE_NAME);
	return TRUE;
}

int CSeedLog::CloseLog()
{
	//��������
	logFile->Save();
	//�ͷ��ļ��ͻ���
	logFile->Free();
	return TRUE;
}

int CSeedLog::EmptyLog()
{
	logFile->Free();
	GetLogFileName(fileName, MAX_PATH);
	if(FALSE == logFile->CreateFile(fileName, CREATE_ALWAYS, MIN_BUFFER_SIZE_CO, MAX_LOG_STR_SIZE))
		return FALSE;
	return TRUE;
}

//ǿ�Ʊ��滺���е�����
int CSeedLog::SaveLog()
{
	return logFile->Save();
}

int CSeedLog::AppendLog(char* lpstring, int iLen)
{
	if(logFile->IsFileValid() == FALSE)
		return FALSE;

	if(iLen > MAX_LOG_STR_SIZE)//��ֹ�ڴ����
		iLen = MAX_LOG_STR_SIZE;
	memcpy(logFile->m_lpCurrent, logString, iLen);
	logFile->Adjust(iLen);
	return TRUE;
}

int CSeedLog::FormatAndAppendLog( LogType type, InfoCode code, char* lpString)
{
	if(logFile->IsFileValid() == FALSE)
		return FALSE;

	//����꾡��־ѡ��
	if(m_detail == 0)
	{//���ü�¼�꾡��־�����꾡��ֱ�ӷ���
		if(code == info_station || code == info_unpack_record || code == info_data_ok
			|| code == info_add_station)
			return TRUE;
	}
	//��ʽ���ַ���
	sprintf_s(logFile->m_lpCurrent, MAX_LOG_STR_SIZE, \
				"[%s %04d]: %s%s\r\n", logTypeStr[type], code, \
				(*logmap)[code].data(), lpString);
	//�Լ�����
	return logFile->Adjust(-1);
}


CSeedLog::CSeedLog()
{
	logmap = new std::map<InfoCode, std::string>;
	logFile = new COutFile;
	m_detail = 0;//no detail log 
	InitLogMap();
}

CSeedLog::~CSeedLog()
{
	CloseLog();
	delete logFile;
	delete logmap;
}

int CSeedLog::IsLogValid()
{
	return logFile->IsFileValid();
}

int CSeedLog::SetDetailLog( int idetail )
{
	if(idetail == 0)
		m_detail = 0;
	else
		m_detail = 1;
	return m_detail;
}

int CSeedLog::IsDetailLog()
{
	return m_detail;
}

//��ʼ��Log��
void CSeedLog::InitLogMap()
{
#define MAP(code, STR)		(*logmap)[code] = STR

	MAP(info_open, "open file at time:");
	MAP(info_file, "seed file information. ");
	MAP(info_station, "station records information. ");
	MAP(info_close, "close file at time:");
	MAP(info_unpack_record, "decompress one record. ");
	MAP(info_data_ok, "decompress successfully. ");
	MAP(info_start_unpack, "begin to decompress the records.");
	MAP(info_header_export, "header info export successfully. ");
	MAP(info_data_export, "data export successfully. ");
	MAP(info_extract, "total extracted samples:");
	MAP(info_add_value, "no enough samples. add the lost ones. ");
	MAP(info_add_station, "add new station.");
	MAP(info_merge_file_created, "create new merge file.");

	MAP(warn_data_differ, "the decompressed data verify failed. ");
	MAP(warn_data_exceed, "redundant data existed. ");
	MAP(warn_data_missing, "missing some samples. ");

	MAP(erro_data_attr, "can't get the right data attribute. can't unpack the record. ");
	MAP(erro_unknow_code, "unknown encoding format.can't unpack the record. ");
	MAP(erro_header_create_failed, "failed to create the header info file.");
	MAP(erro_export_file_create_failed, "failed to create the export file. ");
	MAP(erro_data_export, "data export failed. ");
}

