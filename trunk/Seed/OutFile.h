
#ifndef _OUT_FILE_H_
#define _OUT_FILE_H_

#include <windows.h>

#define MIN_BUFFER_SIZE_CO				1024*1024
#define NORMAL_BUFFER_SIZE_CO			1024*1024*4

class COutFile
{
public:
	HANDLE m_hFile;
	char* m_lpCurrent;
	int m_iBufferSize;
	int m_iBorderSize;

	int m_iError;

	COutFile();
	~COutFile();
	//��������ļ�
	int CreateFile(const TCHAR* lpFile, DWORD dwCreateFlag, int iBufferSize,int iBorderSize);
	//�ļ���дָ�������ĩβ
	int AdjustFileToEnd();
	//ָ�����
	int Adjust(int iLen = -1);
	//�ж��ļ��Ƿ���Ч
	int IsFileValid();
	//�ļ�����
	int Save();
	//�ͷŻ���ռ�
	void Free();
private:
	char* m_lpBase;
	char* m_lpMax;
};


#endif