
#include "MapFile.h"

void* CMapFile::MapFile(const TCHAR *lpFile, int iMapSize)
{
	//释放数据
	if(m_hFile != INVALID_HANDLE_VALUE || m_hMapping != NULL)
		UnmapFile();

	//打开文件
	m_hFile = CreateFile(lpFile, GENERIC_READ/* | GENERIC_WRITE*/, FILE_SHARE_READ, 
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(m_hFile == INVALID_HANDLE_VALUE)
		return NULL;

	//创建映射文件
	m_hMapping = CreateFileMapping(m_hFile, NULL, PAGE_READONLY,
		0, iMapSize, NULL );
	if(m_hMapping == NULL)
	{
		UnmapFile();
		return NULL;
	}
	//提交映射文件
	m_lpBase = MapViewOfFile(m_hMapping, FILE_MAP_READ, 0, 0, 0);
	if(m_lpBase == NULL)
	{
		UnmapFile();
		return NULL;
	}
	//
	m_iSize = ::GetFileSize(m_hFile, NULL);
	if(iMapSize == 0)
		m_iMapSize = m_iSize;
	else 
		m_iMapSize = iMapSize;
	return m_lpBase;
}

/* release the memory and delete the temporary file and don't save the file */
void CMapFile::UnmapFile()
{
	if(m_hMapping != NULL)
	{
		UnmapViewOfFile(m_lpBase);
		CloseHandle(m_hMapping);
		m_hMapping = NULL;
	}
	if(m_hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hFile);
		m_hFile = INVALID_HANDLE_VALUE;
	}
	m_lpBase = NULL;
}
//just the file stay at the open state
void CMapFile::SuspendFile()
{
	if(m_hMapping != NULL)
	{
		UnmapViewOfFile(m_lpBase);
		CloseHandle(m_hMapping);
		m_hMapping = NULL;
	}
	m_lpBase = NULL;
}
void* CMapFile::ResumeFile()
{
	//already mapped the file 
	if(m_hMapping != NULL)
		return m_lpBase;
	//
	if(m_hFile != INVALID_HANDLE_VALUE)
	{
		//if file is valid, map the file
		//创建映射文件
		m_hMapping = CreateFileMapping(m_hFile, NULL, PAGE_READONLY,
			0, m_iMapSize, NULL );
		if(m_hMapping == NULL)
		{
			UnmapFile();
			return NULL;
		}
		//提交映射文件
		m_lpBase = MapViewOfFile(m_hMapping, FILE_MAP_READ, 0, 0, 0);
		if(m_lpBase == NULL)
		{
			UnmapFile();
			return NULL;
		}
		return m_lpBase;
	}
	else//invalid file handle, just return null
		return NULL;
}

int CMapFile::IsMapFileValid()
{
	if(m_hFile == INVALID_HANDLE_VALUE)
		return FALSE;
	else if(m_hMapping == 0)
		return FALSE;
	else
		return TRUE;
}

unsigned int CMapFile::GetFileSize()
{
	return m_iSize;
}

unsigned int CMapFile::GetMapFileSize()
{
	return m_iMapSize;
}
