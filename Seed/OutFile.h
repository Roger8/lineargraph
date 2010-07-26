
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
	//创建或打开文件
	int CreateFile(const TCHAR* lpFile, DWORD dwCreateFlag, int iBufferSize,int iBorderSize);
	//文件读写指针调整到末尾
	int AdjustFileToEnd();
	//指针调整
	int Adjust(int iLen = -1);
	//判断文件是否有效
	int IsFileValid();
	//文件保存
	int Save();
	//释放缓冲空间
	void Free();
private:
	char* m_lpBase;
	char* m_lpMax;
};


#endif