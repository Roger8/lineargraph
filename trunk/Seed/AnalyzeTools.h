
#ifndef _ANALYZE_TOOLS_H_
#define _ANALYZE_TOOLS_H_

#include "DataRecordStruct.h"	//BTIME定义

//校验文件的合法性
int IsValidHeaderCode(void* lpBase);
int IsValidFixDataHeader(void* lpBase, int iStep);
int IsValidDataRecord(void* lpBase, int iStep);

int IsValidSeed(void* lpBase, int iSize);
int IsMiniSeed(void* lpBase, int iSize);

//分析SEED文件信息
int GetRecordStep(void* lpBase, int iSize);
int GetStepFromSeedBlock(void* lpBase, int iSize);
int GetSwapFlag(void* lpFDH, int isize, int* pSwap);
int AnalyzeSeedFile(void* lpBase, int iSize, int* lpSwap, int* lpStep);
//从控制头中得到数据压缩编码格式


//分析数据块的工具函数
char* GetFirstDataRecordAddr(char* lpBase, int iSize, int iStep);

//下面的函数在DataRecordCtrl.cpp中
//下面的函数的默认前提是，指定的数据已经校验为合法SEED文件

//从已经确认为SEED的文件中提取特定台站分量的数据地址
char* GetDataRecordLen(char* lpStart, int iSize, int iStep, int* lpLen = 0, \
					 int iSwap = 0, int* lpTotalCnt = 0);
//从当前位置，寻找下一个台站分量的数据地址
char* GetNextDataRecordAddr(char* lpBase, int iSize, int iStep);
//从当前位置开始，寻找特定台站分量的数据地址
char* GetDataRecordAddr(char* lpBase, int iSize, int iStep, char* lpStation, char* lpChannel);
//栋当前位置开始，寻找特定时间所在的块
char* LocateRecordByTime(char* lpBase, int iSize, int iStep, int iSwap, BTIME* lptime);
//分析文件中的数据信息
//从已经确认为SEED的文件中提取特定台站分量的数据地址
int AnalyzeStationInfo(char* lpBase, int iSize, int iStep, int iSwap, 
					   char** lpNext, int* lpLen, 
					   BTIME* lpstart, BTIME* lpend, int* lpcnt, int* lplost, 
					   int* pnumerator, int* pdenominator);

//取得指定record的属性
int GetRecordInfo(FIX_DATA_HEADER* lpFDH, int iStep, int iSwap, int* lpNumerator, 
				  int* lpDenominator, BTIME* lpStart, BTIME* lpEnd, int* lpCnt);
int GetRecordRate(FIX_DATA_HEADER* lpFDH, int iSwap, int* lpNumerator, int* lpDenominator);
void GetRecordBTime(FIX_DATA_HEADER* lpFDH, int iSwap, BTIME* lpTime);
void GetRecordSampleCnt(FIX_DATA_HEADER* lpFDH, int iSwap, int* lpcnt);
//得到数据的属性
char* Get1000Block(FIX_DATA_HEADER* lpFDH, int iStep, int iSwap);
int GetDataAttrib(FIX_DATA_HEADER* lpFDH, int iStep, int iSwap, int* lpFormat, \
				  int* lpOrder, int* lpLen);




#endif