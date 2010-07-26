
//������delete seed fileʱ��Ҫʹ������������
#define CSEED_FILE_CLASS_IMPLEMENT

#include "SeedFile.h"
#include "Convert.h"
#include "OperateStruct.h"

#include "SeedList.h"

CSeedList::CSeedList()
{
	header = new MergeSeedCell;
	header->seed = NULL;
	header->next = NULL;
	header->sinfo = NULL;
	header->index = 0;
	iElementCnt = 0;
}

CSeedList::~CSeedList()
{
	Free();
	delete header;
}

void CSeedList::Free()
{
	MergeSeedCell *curt, *next;
	curt = header->next;
	while(curt != NULL)
	{
		next = curt->next;
		if(curt->seed != NULL)
			delete curt->seed;	//�ͷ�seedfile
		delete curt;//�ͷ�merge cells
		curt = next;
	}
	//���³�ʼ��
	header->next = NULL;
	iElementCnt = 0;
}

MergeSeedCell* CSeedList::NextElement( MergeSeedCell* lpCurt )
{
	if(lpCurt == NULL)
		return header->next;
	else
		return lpCurt->next;
}


int CSeedList::AddElement( CSeedFile* lpElement, int index, Station* pstation )
{
	//�ú���ֻ��������µĵ�Ԫ�������ǵ�Ԫ�Ŀ�����
	MergeSeedCell* lpCell;
	MergeSeedCell*last, * curt;
	//
	iElementCnt ++;
	//���쵥Ԫ
	lpCell = new MergeSeedCell;
	lpCell->seed = lpElement;
	lpCell->index = index;
	lpCell->sinfo = pstation;
	lpCell->next = NULL;

	last = header;
	curt  = header->next;
	while(curt != NULL)
	{
		if( CmpSystemTimeS(&curt->sinfo->time, &lpCell->sinfo->time) > 0 )//Ӧ�ò��뵽ǰ��
		{
			last->next = lpCell;
			lpCell->next = curt;
			return TRUE;
		}
		last = curt;
		curt = curt->next;
	}
	last->next = lpCell;
	return TRUE;

}

int CSeedList::GetElementCnt()
{
	return iElementCnt;
}

void CSeedList::DelFirstElement()
{
	MergeSeedCell* pfirst;
	if(header->next != NULL)
	{
		pfirst = header->next;
		header->next = pfirst->next;
		if(pfirst->seed != NULL)
			delete pfirst->seed;
		delete pfirst;
		iElementCnt --;
	}
}