
#ifndef _SEED_LIST_H_
#define _SEED_LIST_H_

#include <Windows.h>

class CSeedFile;
struct Station;

struct MergeSeedCell
{
	CSeedFile* seed;
	Station* sinfo;
	int index;		//����seed���ݴ���ĵ��ò���
	MergeSeedCell* next;
};

class CSeedList
{
public:
	CSeedList();
	~CSeedList();

	int AddElement(CSeedFile* lpElement, int index, Station* pstation);
	void DelFirstElement();
	MergeSeedCell* NextElement(MergeSeedCell* lpCurt);
	int GetElementCnt();
	void Free();
private:

	MergeSeedCell * header;
	int iElementCnt;
};

#endif