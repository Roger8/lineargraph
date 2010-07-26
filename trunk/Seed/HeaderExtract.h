
#ifndef _HEADER_EXTRACT_H_
#define _HEADER_EXTRACT_H_

// #define RECORD_INFO_OVER		1
// #define RECORD_INFO_CONTINUE	2

//��ȡseed�ļ��е�ָ������ͷ��Ϣ
int ReadVolumeIndexHeader(char* lpHeader,  int iStep, int iSize, char* lpBuf);
int ReadDictionaryHeader(char* lpHeader,  int iStep, int iSize, char* lpBuf);
int ReadStationHeader(char* lpHeader,  int iStep, int iSize, char* lpBuf);
int ReadTimeSpanHeader(char* lpHeader,  int iStep, int iSize, char* lpBuf);
//��ȡ���еĿ���ͷ��Ϣ
int ReadControlHeader(char* lpHeader,  int iStep, int iSize, char* lpBuf);

//��ȡһ���¼�е���Ч��Ϣ���������뵽buffer����Ϣ�ĳ���bytes
int ExtractOneCtrlHeader(char* lpHeader, int iSize, int iCopy, int* lpLost, char* lpBuf);

#endif