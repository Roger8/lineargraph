
#ifndef _ANALYZE_TOOLS_H_
#define _ANALYZE_TOOLS_H_

#include "DataRecordStruct.h"	//BTIME����

//У���ļ��ĺϷ���
int IsValidHeaderCode(void* lpBase);
int IsValidFixDataHeader(void* lpBase, int iStep);
int IsValidDataRecord(void* lpBase, int iStep);

int IsValidSeed(void* lpBase, int iSize);
int IsMiniSeed(void* lpBase, int iSize);

//����SEED�ļ���Ϣ
int GetRecordStep(void* lpBase, int iSize);
int GetStepFromSeedBlock(void* lpBase, int iSize);
int GetSwapFlag(void* lpFDH, int isize, int* pSwap);
int AnalyzeSeedFile(void* lpBase, int iSize, int* lpSwap, int* lpStep);
//�ӿ���ͷ�еõ�����ѹ�������ʽ


//�������ݿ�Ĺ��ߺ���
char* GetFirstDataRecordAddr(char* lpBase, int iSize, int iStep);

//����ĺ�����DataRecordCtrl.cpp��
//����ĺ�����Ĭ��ǰ���ǣ�ָ���������Ѿ�У��Ϊ�Ϸ�SEED�ļ�

//���Ѿ�ȷ��ΪSEED���ļ�����ȡ�ض�̨վ���������ݵ�ַ
char* GetDataRecordLen(char* lpStart, int iSize, int iStep, int* lpLen = 0, \
					 int iSwap = 0, int* lpTotalCnt = 0);
//�ӵ�ǰλ�ã�Ѱ����һ��̨վ���������ݵ�ַ
char* GetNextDataRecordAddr(char* lpBase, int iSize, int iStep);
//�ӵ�ǰλ�ÿ�ʼ��Ѱ���ض�̨վ���������ݵ�ַ
char* GetDataRecordAddr(char* lpBase, int iSize, int iStep, char* lpStation, char* lpChannel);
//����ǰλ�ÿ�ʼ��Ѱ���ض�ʱ�����ڵĿ�
char* LocateRecordByTime(char* lpBase, int iSize, int iStep, int iSwap, BTIME* lptime);
//�����ļ��е�������Ϣ
//���Ѿ�ȷ��ΪSEED���ļ�����ȡ�ض�̨վ���������ݵ�ַ
int AnalyzeStationInfo(char* lpBase, int iSize, int iStep, int iSwap, 
					   char** lpNext, int* lpLen, 
					   BTIME* lpstart, BTIME* lpend, int* lpcnt, int* lplost, 
					   int* pnumerator, int* pdenominator);

//ȡ��ָ��record������
int GetRecordInfo(FIX_DATA_HEADER* lpFDH, int iStep, int iSwap, int* lpNumerator, 
				  int* lpDenominator, BTIME* lpStart, BTIME* lpEnd, int* lpCnt);
int GetRecordRate(FIX_DATA_HEADER* lpFDH, int iSwap, int* lpNumerator, int* lpDenominator);
void GetRecordBTime(FIX_DATA_HEADER* lpFDH, int iSwap, BTIME* lpTime);
void GetRecordSampleCnt(FIX_DATA_HEADER* lpFDH, int iSwap, int* lpcnt);
//�õ����ݵ�����
char* Get1000Block(FIX_DATA_HEADER* lpFDH, int iStep, int iSwap);
int GetDataAttrib(FIX_DATA_HEADER* lpFDH, int iStep, int iSwap, int* lpFormat, \
				  int* lpOrder, int* lpLen);




#endif