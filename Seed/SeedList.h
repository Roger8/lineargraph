
#ifndef _SEED_LIST_H_
#define _SEED_LIST_H_

#include <Windows.h>

class CSeedFile;
struct Station;

struct MergeSeedCell
{
	CSeedFile* seed;
	Station* sinfo;
	int index;		//用于seed数据处理的调用参数
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