
#ifndef _MAP_FILE_H_
#define _MAP_FILE_H_

#include <Windows.h>


//��һ���ļ��ύΪ�ڴ�ӳ���ļ���ֻ����
class CMapFile{
public:
	
	HANDLE m_hFile;
	HANDLE m_hMapping;
	void* m_lpBase;
	unsigned int m_iSize;
	int m_iMapSize;

	CMapFile()
	{
		m_hFile = INVALID_HANDLE_VALUE;
		m_hMapping = 0;
		m_lpBase = 0;
	}
	~CMapFile()
	{
		UnmapFile();
	}

	void* MapFile(const TCHAR* lpFile, int iMapSize);
	void SuspendFile();
	void* ResumeFile();
	void UnmapFile();
	int IsMapFileValid();
	unsigned int GetFileSize();
	unsigned int GetMapFileSize();
};


#endif