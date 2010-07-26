
#ifndef _HEADER_EXTRACT_H_
#define _HEADER_EXTRACT_H_

// #define RECORD_INFO_OVER		1
// #define RECORD_INFO_CONTINUE	2

//提取seed文件中的指定控制头信息
int ReadVolumeIndexHeader(char* lpHeader,  int iStep, int iSize, char* lpBuf);
int ReadDictionaryHeader(char* lpHeader,  int iStep, int iSize, char* lpBuf);
int ReadStationHeader(char* lpHeader,  int iStep, int iSize, char* lpBuf);
int ReadTimeSpanHeader(char* lpHeader,  int iStep, int iSize, char* lpBuf);
//提取所有的控制头信息
int ReadControlHeader(char* lpHeader,  int iStep, int iSize, char* lpBuf);

//提取一块记录中的有效信息，返回填入到buffer的信息的长度bytes
int ExtractOneCtrlHeader(char* lpHeader, int iSize, int iCopy, int* lpLost, char* lpBuf);

#endif