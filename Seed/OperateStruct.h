
#ifndef _OPERATE_STRCUT_H_
#define _OPERATE_STRCUT_H_

#define STATION_STR_LEN		6
#define CHANNEL_STR_LEN		6

#define EXPORT_TXT		0
#define EXPORT_BIN		1
#define EXPORT_MSEED	2
#define EXPORT_SEED		3
#define EXPORT_HEADER	4


struct FREQ
{
	int numerator;
	int denominator;
};

struct FRACTION
{
	__int64 intpart;
	__int64 numerator;
	__int64 denominator;
};

struct FRACTION_S
{
	int intpart;
	int numerator;
	int denominator;
};

//台站信息结构体
struct Station
{
	//station records information
	WCHAR station[STATION_STR_LEN];
	WCHAR channel[CHANNEL_STR_LEN];
	FREQ freq;
	int nSample;
	int nlost;
	char* lpAddr;
	int iRecordLen;
	SYSTEMTIME time;
	SYSTEMTIME tend;
	//unpack data information
	int* pData;
	int bufitemcnt;
	int iDataCnt;
	FREQ dataFreq;
	SYSTEMTIME datatime;
	//next station pointer
	Station * lpNext;
};

//control parameter 
struct CtrlParam{
	WCHAR station[STATION_STR_LEN];
	WCHAR channel[CHANNEL_STR_LEN];
// 	char* pdata;
// 	int cnt;
	SYSTEMTIME tstart;
	SYSTEMTIME tend;
	int numerator;
	int denominator;
	int ctrl;
	int addval;
	int type;
};
#endif
