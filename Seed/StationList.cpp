
#define CSEED_FILE_CLASS_IMPLEMENT

#include <memory.h>
#include <Windows.h>
#include "OperateStruct.h"//����Station�ṹ��

#include "StationList.h"

CStationList::CStationList()
{
	iElementCnt = 0;
	header = new Station;
	header->lpNext = NULL;
	tail = header;
}

CStationList::~CStationList()
{
	Free();
	delete header;
}

int CStationList::AddElement(Station* lpElement)
{
	Station * lptmp;
	lptmp = new Station;
	memcpy(lptmp, lpElement, sizeof(Station));
	tail->lpNext = lptmp;
	tail = lptmp;
	tail->lpNext = NULL;
	iElementCnt ++;
	return iElementCnt;
}

//index��0��ʼ���
Station* CStationList::LocateElement(int index)
{
	int n;
	Station* lpStation;
	if(index >= iElementCnt || index < 0 )
		return NULL;
	lpStation = header->lpNext;
	for( n = 0; n < index; n++)
		lpStation = lpStation->lpNext;
	return lpStation;
}

//����̨վ������
int CStationList::GetElementCnt()
{
	return iElementCnt;
}

//ֻ�ͷ���������ͷ��㲻�ͷ�
void CStationList::Free()
{
	Station* lpCur;
	lpCur = header->lpNext;
	while (lpCur != NULL)
	{
		tail = lpCur;
		lpCur = lpCur->lpNext;
		delete tail;
	}
	//��ʼ��
	tail = header;
	header->lpNext = NULL;
	iElementCnt = 0;
}

int CStationList::LocateIndexByStation( WCHAR* lpStation )
{
	Station* lpCurt;
	int index;
	index = 0;
	lpCurt = header->lpNext;
	while(lpCurt != NULL)
	{
		if( 0 == memcmp(lpCurt->station, lpStation, wcslen(lpStation)*sizeof(WCHAR)) )
			return index;
		lpCurt = lpCurt->lpNext;
		index ++;
	}
	return -1;
}

int CStationList::LocateIndexByChannel( WCHAR* lpChannel )
{
	Station* lpCurt;
	int index;
	index = 0;
	lpCurt = header->lpNext;
	while(lpCurt != NULL)
	{
		if( 0 == memcmp(lpCurt->channel, lpChannel, wcslen(lpChannel)*sizeof(WCHAR)) )
			return index;
		index ++;
		lpCurt = lpCurt->lpNext;
	}
	return -1;
}

int CStationList::LocateIndexByStatChnn( const WCHAR* lpStation, const WCHAR* lpChannel )
{
	Station* lpCurt;
	int index;
	index = 0;
	lpCurt = header->lpNext;
	while(lpCurt != NULL)
	{
		if( 0 == memcmp(lpCurt->station, lpStation, wcslen(lpStation)*sizeof(WCHAR)) )
			if( 0 == memcmp(lpCurt->channel, lpChannel, wcslen(lpChannel)*sizeof(WCHAR)) )
			return index;
		index ++;
		lpCurt = lpCurt->lpNext;
	}
	return -1;
}

int CStationList::LocateIndex( Station* pStation )
{
	int index;
	Station* lpCurt;
	lpCurt = header->lpNext;
	index = 0;
	while(index < iElementCnt)
	{
		if(lpCurt == pStation)
			return index;
		lpCurt = lpCurt->lpNext;
		index ++;
	}
	return -1;
}
