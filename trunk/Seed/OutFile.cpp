
#include "OutFile.h"

COutFile::COutFile()
{
	m_hFile = INVALID_HANDLE_VALUE;
	m_lpCurrent = NULL;
	m_lpBase = NULL;
	m_iBorderSize = 0;
	m_iBufferSize = 0;
	m_iError = 0;
	m_lpMax = NULL;
}

COutFile::~COutFile()
{
	Free();
}

int COutFile::CreateFile( const TCHAR* lpFile, DWORD dwCreateFlag, int iBufferSize,int iBorderSize )
{
	//释放空间
	Free();

	m_hFile = ::CreateFile(lpFile, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, 
		NULL, dwCreateFlag, FILE_ATTRIBUTE_NORMAL, NULL);
	if(m_hFile == INVALID_HANDLE_VALUE)
		return FALSE;

	//缓存不可过小
	if(iBufferSize < MIN_BUFFER_SIZE_CO)
		iBufferSize = MIN_BUFFER_SIZE_CO;
	if(iBorderSize > iBufferSize)
		iBorderSize = iBufferSize>>2;//取1/4大小的缓存边界
	
	m_lpBase = new char[iBufferSize];
	if(m_lpBase == NULL)
	{
		::CloseHandle(m_hFile);
		m_hFile = INVALID_HANDLE_VALUE;
		return FALSE;
	}
	
	m_iError = 0;
	m_lpCurrent = m_lpBase;

	m_iBufferSize = iBufferSize;
	m_iBorderSize = iBorderSize;

	m_lpMax = m_lpBase + (iBufferSize - iBorderSize);

	return TRUE;
}

int COutFile::Adjust(int iLen)
{
	DWORD dwBytesWrited;
	if(m_hFile == INVALID_HANDLE_VALUE)
		return FALSE;

	//使用其他方法得到长度
	if(iLen == -1)
		iLen = strlen(m_lpCurrent);

	//调整指针
	m_lpCurrent += iLen;
	if(m_lpCurrent >= m_lpMax)
	{
		if(::WriteFile(m_hFile, m_lpBase, (m_lpCurrent - m_lpBase), &dwBytesWrited, NULL)== 0)
		{
			/* if write data failed, we ignore the data in the buffer, just for security , */
			/*avoid memory accessed illegally */
			m_lpCurrent = m_lpBase;
			m_iError = 1;
			return FALSE;
		}
		m_lpCurrent = m_lpBase;
	}

	return TRUE;
}

int COutFile::AdjustFileToEnd()
{
	if(m_hFile == INVALID_HANDLE_VALUE)
		return FALSE;
	
	if(SetFilePointer(m_hFile, NULL, NULL, FILE_END) == INVALID_SET_FILE_POINTER)
		return FALSE;
	else
		return TRUE;
}

int COutFile::IsFileValid()
{
	if(m_hFile == INVALID_HANDLE_VALUE)
		return FALSE;
	else
		return TRUE;
}

int COutFile::Save()
{
	DWORD dwBytesWrited;
	int iRnt;

	if(m_hFile == INVALID_HANDLE_VALUE)
		return FALSE;

	if(::WriteFile(m_hFile, m_lpBase, (m_lpCurrent - m_lpBase), &dwBytesWrited, NULL)== 0)
	{
		m_iError = 1;
		iRnt = FALSE;
	}
	else
		iRnt = TRUE;

	/* if write data failed, we ignore the data in the buffer, just for security , */
	/*avoid memory accessed illegally */
	m_lpCurrent = m_lpBase;

	return iRnt;
}

void COutFile::Free()
{
	if(m_hFile != INVALID_HANDLE_VALUE)
	{
		Save();
		CloseHandle(m_hFile);
		m_hFile = INVALID_HANDLE_VALUE;
		delete[] m_lpBase;
		m_lpBase= NULL;
	}
	m_iError= 0;
}