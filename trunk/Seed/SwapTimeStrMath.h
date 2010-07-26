
#ifndef _SWAP_TIME_STR_MATH_H_
#define _SWAP_TIME_STR_MATH_H_

//��ȡBTIME���ݽṹ
#include "DataRecordStruct.h"
#include <memory.h>

//ת����
/* convert the string to integer */
#define 	STR2TOINT(lpstr)		( (lpstr[0]-48)*10 + (lpstr[1]-48))
#define 	STR3TOINT(lpstr)		( (lpstr[0]-48)*100 + (lpstr[1]-48)*10 + (lpstr[2]-48) )
#define 	STR4TOINT(lpstr)		( (lpstr[0]-48)*1000 + (lpstr[1]-48)*100 + \
	(lpstr[2]-48)*10 + (lpstr[3]-48) )
#define 	STR5TOINT(lpstr)		( (lpstr[0]-48)*10000 + (lpstr[1]-48)*1000 + \
	(lpstr[2]-48)*100 + (lpstr[3]-48)*10 + (lpstr[4]-48) )
#define 	STR6TOINT(lpstr)		( (lpstr[0]-48)*100000 + (lpstr[1]-48)*10000 + \
	(lpstr[2]-48)*1000 + (lpstr[3]-48)*100 + (lpstr[4]-48)*10 + (lpstr[5]-48) )

/* swap the word order. And the "i" must be an integer */
#define SWAP2(i)				( ((i>>8)&0xff) | ((i&0xff)<<8))
#define SWAP4(i)				( ((i>>24)&0xff) | ((i&0xff)<<24) | ((i>>8)&0xff00) | ((i&0xff00)<<8))

#define POWER2(i)		(1<<i)



//ʱ��ת����
#define INVALID_DAY		0xffffffff
#define YEAR_4_MASK		0x00000003
#define IS_LEAP_YEAR(y)		( ((y&YEAR_4_MASK) == 0) \
	&& (y%100 != 0) \
	|| (y%400 == 0) )



//ʱ�����������
#define BTIME_MINI_SECOND_MAX		10000


#define NORMAL_YEAR_SECONDS	(__int64)(365*24*3600)
#define LEAP_YEAR_SECONDS		(__int64)(366*24*3600)
#define DAY_SECONDS			(__int64)(24*3600)

#define HOUR_SECONDS			3600
#define MINUTE_SECONDS		60

#define MINI_SECONDS_CNT		10000

#define _4_YEARS_SECONDS		(LEAP_YEAR_SECONDS + 3*NORMAL_YEAR_SECONDS)
#define _100_YEARS_SECORDS	(24*LEAP_YEAR_SECONDS + 76*NORMAL_YEAR_SECONDS)
#define _400_YEARS_SECONDS	(97*LEAP_YEAR_SECONDS + 303*NORMAL_YEAR_SECONDS)

//��ȫ�Ƚ�
#define CMPBTIMEA(a, b)	(memcmp((char*)a, (char*)b, sizeof(BTIME)))
//������������ֽ�
#define CMPBTIME(a, b)	(memcmp((char*)a, (char*)b, sizeof(BTIME)-3))


#define MIN_SEED_RECORD_LEN				256
#define MAX_SEED_RECORD_LEN				32768
//the power of 2
#define MIN_SEED_RECORD_LEN_BIT			8
#define MAX_SEED_RECORD_LEN_BIT			15

//���ݿ��record length
#define MIN_DATA_RECORD_LEN_BIT			2
#define MAX_DATA_RECORD_LEN_BIT			256

//6���ַ���ת��Ϊ32Ϊ����
int strtoint(char* lpStr, int icnt);

/* ���� 4-bytes �� 8-bytes �ĸ������� */
void Swap4f(void* );
void Swap8d(void*);

int power2(int );
int maxbitpos(int );
int power2_seed_r(int );
int power2_data_r(int);

//�ַ��������������ڽ���ַ���������һ��0
int strcpy_seed(char* lpDest, const char* lpSource);
int charTowchar(wchar_t*lpDest, const char*lpSource, int num);
int wcharTochar(char*lpDest, const wchar_t*lpSource, int num);
int strcpy_ULN(char* lpDest, const char* lpSource, int cnt);
int strcpy_UN_NAME(char* lpDest, const char* lpSource, int cnt);

//ʱ��ת������
unsigned int DayToMonth(unsigned int uiYear, unsigned int uiDays);
int MonthToDay(unsigned int uiYear, unsigned int uiMonth, unsigned int uiDay);

int IsValidBTime(const BTIME* lpTime, int* lpSwap);
int CmpBTIMEAF(BTIME* lpA, BTIME*lpB);
int CmpBTIMEF(BTIME* lpA, BTIME*lpB);
int CmpBTIMEAS(BTIME* lpA, BTIME*lpB);
int CmpBTIMES(BTIME* lpA, BTIME*lpB);
//ʱ����ѧ�����������long long������1/10000��Ϊ��λ��
//�����ʱ��Ӽ�ת��������û�в��Թ���ֻҪBTimeToLen��LenToBTime
//û������Ͳ��������⣬��Ϊ�����ĺ���������������������
void BTimeToLen(const BTIME* lpTime, long long* lpLLong);
void LenToBTime(BTIME* lpTime, long long* lpLLong);
void BTimeAddLen_L(BTIME* lpTime, long long dAdd);
// void BTimeSubLen_L(BTIME* lpTime, long long dSub);
// void BTimeSubLen_b(BTIME* lpTime, long long dSub);
// void BTimeAddBTime(BTIME* lpDest, const BTIME* lpAdd);
//void BTimeSubBTime_L(BTIME* lpDest, const BTIME* lpSub);
void BTimeSubBTime_L(const BTIME* x, const BTIME* y, long long* lpDiff);

#endif
