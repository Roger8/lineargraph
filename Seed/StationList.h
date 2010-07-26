
#ifndef _STATION_LIST_H_
#define _STATION_LIST_H_

#include <Windows.h>

struct Station;

class CStationList
{
public:
	CStationList();
	~CStationList();
	
	int AddElement(Station* lpElement);

	Station* LocateElement(int index);

	int LocateIndex(Station* pStation);
	int LocateIndexByStation(WCHAR* lpStation);
	int LocateIndexByChannel(WCHAR* lpChannel);
	int LocateIndexByStatChnn(const WCHAR* lpStation, const WCHAR* lpChannel);

	int GetElementCnt();
	void Free();
private:
	Station * header;
	Station * tail;
	int iElementCnt;
};


#endif