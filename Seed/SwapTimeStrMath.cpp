
#include <memory.h>
#include "Judge.h"
#include "SwapTimeStrMath.h"


//字符串转换为32位整数
int strtoint(char* lpStr, int icnt)
{
	int n;
	int resault;
	resault = 0;
	for( n = 0; n < icnt; n++ )
	{
		resault *= 10;
		resault += lpStr[n] - '0';
	}
	return resault;
}

void Swap4f(void* byte4)
{
	char *temp;
	char buf;
	temp = (char*)byte4;
	buf = temp[0];
	temp[0] = temp[3];
	temp[3] = temp[0];
	buf = temp[1];
	temp[1] = temp[2];
	temp[2] = temp[1];
}

void Swap8d(void* byte8)
{
	int *newdata = (int*)byte8;
	int temp;
	temp = newdata[1];
	newdata[1] = SWAP4(newdata[0]);
	newdata[0] = SWAP4(temp);
}

int strcpy_seed(char* lpDest, const char* lpSource)
{
	int i = 0;
	while(lpSource[i] != '~')
	{
		lpDest[i] = lpSource[i];
		i++;
	}
	/* set a zero follow the last valid character */
	lpDest[i] = 0;
	return i;
}

int power2(int iCount)
{
	int ans = 1;
	if(iCount < 0)
		return 0;
	return ans<<iCount;
}

int power2_seed_r(int icnt)
{
	if(icnt >= MIN_SEED_RECORD_LEN_BIT && icnt <= MAX_SEED_RECORD_LEN_BIT)
		return POWER2(icnt);
	else
		return 0;
}
int power2_data_r(int icnt)
{
	if(icnt >= MIN_DATA_RECORD_LEN_BIT && icnt <= MAX_DATA_RECORD_LEN_BIT)
		return POWER2(icnt);
	else
		return 0;
}

int maxbitpos(int num)
{
	int i;
	i = 0;
	while(1)
	{
		if( num == 0 )
			return i-1;
		num >>= 1;
		i++;
	}
}

int charTowchar(wchar_t*lpDest, const char*lpSource, int num)
{
	int i;
	for(i=0; i<num; i++)
	{
		if( (lpSource[i]>='a' && lpSource[i]<='z') || \
			(lpSource[i]>='A' && lpSource[i]<='Z') || \
			(lpSource[i]>='0' && lpSource[i]<='9') )
			lpDest[i] = (wchar_t)lpSource[i];
		else
			lpDest[i] = 0;
	}
	lpDest[i] = 0;
	return i;
}
int wcharTochar(char*lpDest, const wchar_t*lpSource, int num)
{
	int i;
	for(i = 0; i < num; i++)
	{
		lpDest[i] = (char)lpSource[i];
	}
	lpDest[i] = 0;
	return i;
}
int strcpy_ULN(char* lpDest, const char* lpSource, int cnt)
{
	int n;
	for( n = 0; n < cnt; n++ )
	{
		lpDest[n] = IS_ULN_CHAR(lpSource[n]) ? lpSource[n] : 0;
	}
	lpDest[n] = 0;
	return n;
}
int strcpy_UN_NAME(char* lpDest, const char* lpSource, int cnt)
{
	int n;
	for( n = 0; n < cnt; n++ )
	{
		lpDest[n] = IS_UN_NAME_CHAR(lpSource[n]) ? lpSource[n] : 0;
	}
	lpDest[n] = 0;
	return n;
}


unsigned int DayToMonth(unsigned int uiYear, unsigned int uiDays)
{
	unsigned int uiMonth;
	unsigned int uiFebruary;
	if(uiDays > 366)
		return INVALID_DAY;

	/* get the right days of February */
	if(IS_LEAP_YEAR(uiYear))
		uiFebruary = 29;
	else uiFebruary = 28;

	/* get the day and month */
	for( uiMonth = 1; uiMonth < 13; uiMonth ++ )
	{
		switch(uiMonth)
		{
		case 2:
			if(uiDays > uiFebruary)
				uiDays -= uiFebruary;
			else 
				goto next;
			break;
		case 1:
		case 3:
		case 5:
		case 7:
		case 8:
		case 10:
		case 12:
			if(uiDays > 31)
				uiDays -= 31;
			else 
				goto next;
			break;
		case 4:
		case 6:
		case 9:
		case 11:
			if(uiDays > 30)
				uiDays -= 30;
			else 
				goto next;
			break;
		}
	}
	/* get the data and return */
next:
	return (uiMonth<<16) | ((unsigned int)uiDays);

}

int MonthToDay(unsigned int uiYear, unsigned int uiMonth, unsigned int uiDay)
{
	/* not use the first element */
	const unsigned int ciMonths[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	unsigned int i, days;

	if(uiMonth > 12 || uiMonth == 0 || uiDay == 0 )
		return 0;
	if(uiDay > ciMonths[uiMonth])
	{
		if(uiMonth != 2)
			return 0;
		else if( !( IS_LEAP_YEAR(uiYear) && uiDay == 29 ) )
			return 0;
	}

	days = uiDay;
	for(i = 1; i < uiMonth; i++)
	{
		days += ciMonths[i];
	}
	if(uiMonth > 2 && IS_LEAP_YEAR(uiYear))
		days ++;
	return days;
}

/* operate with the BTIME */
int IsValidBTime(const BTIME* lpTime, int* lpSwap)
{
	int iError;
	unsigned short usYear, usDays;
	iError = 0;//don't change the order of a word
	/* check for days */
	if(lpTime->usDay > 365)
	{
		if( !(IS_LEAP_YEAR(lpTime->usYear) && lpTime->usDay == 366) )
			iError = 1;
	}
	/* check for time */
	if(lpTime->ucHours > 23 || lpTime->ucMinutes> 59 || \
			lpTime->ucSeconds > 59 || lpTime->usMiniSec > 9999)
		iError = 1;
	/* check for the year */
	if(lpTime->usYear > 2200)//this software will not be used in 2200.
		iError = 1;
	if(lpSwap != 0)
		*lpSwap = iError;
	
	//swap the word order and try again
	if(iError == 1)
	{
		usYear = SWAP2(lpTime->usYear);
		if(usYear > 2200)
			return 0;
		usDays = SWAP2(lpTime->usDay);
		if(usDays  > 365)
		{
			if( !( IS_LEAP_YEAR(usYear) && usDays == 366) )
				return 0;
		}
		if(lpTime->ucHours > 23 || lpTime->ucMinutes> 59 || lpTime->ucSeconds > 59)
			return 0;
		usYear = SWAP2(lpTime->usMiniSec);
		if(usYear > 9999)
			return 0;
	}
	return 1;
}

int CmpBTIMEAF(BTIME* lpA, BTIME*lpB)
{
	return memcmp(lpA, lpB, sizeof(BTIME));
}
int CmpBTIMEF(BTIME* lpA, BTIME*lpB)
{
	return memcmp(lpA, lpB, sizeof(BTIME) - 3);
}
int CmpBTIMEAS(BTIME* lpA, BTIME*lpB)
{
	int nRet;
	nRet = (int)lpA->usYear - (int)lpB->usYear;
	if(nRet == 0)
	{
		nRet = (int)lpA->usDay - (int)lpB->usDay;
		if(nRet == 0)
		{
			nRet = (int)lpA->ucHours- (int)lpB->ucHours;
			if(nRet == 0)
			{
				nRet = (int)lpA->ucMinutes - (int)lpB->ucMinutes;
				if(nRet == 0)
				{
					nRet = (int)lpA->ucSeconds - (int)lpB->ucSeconds;
					if(nRet == 0)
						nRet = (int)lpA->usMiniSec - (int)lpB->usMiniSec;
				}
			}
		}
	}
	return nRet;
}
int CmpBTIMES(BTIME* lpA, BTIME*lpB)
{
	int nRet;
	nRet = (int)lpA->usYear - (int)lpB->usYear;
	if(nRet == 0)
	{
		nRet = (int)lpA->usDay - (int)lpB->usDay;
		if(nRet == 0)
		{
			nRet = (int)lpA->ucHours- (int)lpB->ucHours;
			if(nRet == 0)
			{
				nRet = (int)lpA->ucMinutes - (int)lpB->ucMinutes;
				if(nRet == 0)
				{
					nRet = (int)lpA->ucSeconds - (int)lpB->ucSeconds;
// 					if(nRet == 0)
// 						nRet = (int)lpA->usMiniSec - (int)lpB->usMiniSec;
				}
			}
		}
	}
	return nRet;
}

void BTimeToLen(const BTIME* lpTime, long long* lpLLong)
{
	unsigned int uiLeapYear, uiYear;
	long long lltmp;

	if(IsValidBTime(lpTime, 0) == 0)
	{
		*lpLLong = 0;
		return ;
	}
	/*information: 0000 year is not existed.*/
	uiYear = lpTime->usYear -1;	/* current year is for special use */
	uiLeapYear = uiYear>>2;	
	uiLeapYear -= uiYear/100;
	uiLeapYear += uiYear/400;

	/* get the years seconds */
	lltmp = uiLeapYear*LEAP_YEAR_SECONDS + (uiYear - uiLeapYear)*NORMAL_YEAR_SECONDS;

	/* get the special year seconds */
	lltmp += (lpTime->usDay-1)*DAY_SECONDS + lpTime->ucHours*HOUR_SECONDS + \
		lpTime->ucMinutes*MINUTE_SECONDS + (long long)lpTime->ucSeconds;

	lltmp *= MINI_SECONDS_CNT;	/* get the 1/10000 seconds */
	*lpLLong = lltmp + lpTime->usMiniSec;
	return ;
}
void LenToBTime(BTIME* lpTime, long long* lpLLong)
{
	long long llBuf;
	unsigned int uiYear, uiTmp, uiMinisec;

	/* get the seconds */
	llBuf = *lpLLong/MINI_SECONDS_CNT;
	uiMinisec = *lpLLong%MINI_SECONDS_CNT;
	uiYear = 0;	/* just the years counter */
	/*  */
	if(llBuf > NORMAL_YEAR_SECONDS)
	{
		/* find the 400 years units */
		uiYear = (unsigned int )((llBuf/_400_YEARS_SECONDS)*400);
		llBuf %= _400_YEARS_SECONDS;
		/* find the 100 years units */
		uiYear += (unsigned int)((llBuf/_100_YEARS_SECORDS)*100);
		llBuf%= _100_YEARS_SECORDS;
		/* find the 4 years units */
		uiYear += (unsigned int)((llBuf/_4_YEARS_SECONDS))*4;
		llBuf %= _4_YEARS_SECONDS;

		/* deal with last 3 or 4 years */
		uiTmp = (unsigned int)(llBuf/NORMAL_YEAR_SECONDS);
		/* for the forth year is a leap year, which is longer than the normal year */
		if(uiTmp == 4)
			uiTmp --;
		uiYear += uiTmp;
		llBuf -= uiTmp * NORMAL_YEAR_SECONDS;
	}

	/* here not whole year seconds */
	uiYear ++;/*the year sequence */
	lpTime->usYear = (unsigned short)uiYear;

	lpTime->usDay = (unsigned short)(llBuf/DAY_SECONDS) + 1;/* day sequence, no 000 dyas */
	llBuf %= DAY_SECONDS;

	lpTime->ucHours = (unsigned char)(llBuf/HOUR_SECONDS);
	llBuf %= HOUR_SECONDS;

	lpTime->ucMinutes = (unsigned char)(llBuf/MINUTE_SECONDS);
	llBuf %= MINUTE_SECONDS;

	lpTime->ucSeconds = (unsigned char)llBuf;
	lpTime->usMiniSec = (unsigned short)(uiMinisec);

	lpTime->ucUnused = 0;
	return ;
}
void BTimeAddLen_L(BTIME* lpTime, long long dAdd)
{
	long long llTemp;
	BTimeToLen(lpTime, &llTemp);
	llTemp += dAdd;
	if(llTemp <= 0 )
	{
		memset(lpTime, 0, sizeof(BTIME));
		return ;
	}
	LenToBTime(lpTime, &llTemp);
}
// void BTimeAddLen_L(BTIME* lpTime, long long dAdd)
// {	//当添加的时间有2和2年以上时，会出现问题，主要是年的计算上
// 	dAdd += lpTime->usMiniSec;
// 	lpTime->usMiniSec = dAdd%10000;
// 
// 	dAdd /= 10000;	/* get the seconds */
// 	dAdd += lpTime->ucSeconds;
// 	lpTime->ucSeconds = dAdd%60;
// 
// 	dAdd /= 60;
// 	dAdd += lpTime->ucMinutes;
// 	lpTime->ucMinutes = dAdd%60;
// 
// 	dAdd /= 60;
// 	dAdd += lpTime->ucHours;
// 	lpTime->ucHours = dAdd%24;
// 
// 	dAdd/=24;
// 	dAdd += lpTime->usDay;
// 	if(IS_LEAP_YEAR(lpTime->usYear))
// 	{
// 		//the days of a leap year can be 366, so mode by 367
// 		lpTime->usDay = dAdd%367;
// 		lpTime->usDay ++;
// 		lpTime->usYear += (unsigned short)(dAdd/367);
// 	}
// 	else 
// 	{
// 		//the days of a normal year can be 365, so mode by 366
// 		lpTime->usDay = dAdd%366;
// 		lpTime->usDay ++;
// 		lpTime->usYear += (unsigned short)(dAdd/366);
// 	}
// }

// void BTimeSubLen_L(BTIME* lpTime, long long dSub)
// {//被BTimeAddLen_L函数所代替
// 	long long llTemp;
// 	BTimeToLen(lpTime, &llTemp);
// 	if(llTemp <= dSub)
// 	{
// 		memset(lpTime, 0, sizeof(BTIME));
// 		return ;
// 	}
// 	llTemp -= dSub;
// 	LenToBTime(lpTime, &llTemp);
// }

// void BTimeSubLen_b(BTIME* lpTime, long long dSub)
// {
// 	BTIME tSub;
// 	LenToBTime(&tSub, &dSub);
// 	BTimeSubBTime_L(lpTime, &tSub);
// }

void BTimeSubBTime_L( const BTIME* x, const BTIME* y, long long* lpDiff )
{
	long long llx, lly;
	BTimeToLen(x, &llx);
	BTimeToLen(y, &lly);
	*lpDiff = llx - lly;
}
// void BTimeAddBTime(BTIME* lpDest, const BTIME* lpAdd)
// {
// 	unsigned int uiSum;
// 
// 	uiSum = lpAdd->usMiniSec + lpDest->usMiniSec;
// 	lpDest->usMiniSec = uiSum%10000;	/* mini seconds */
// 
// 	uiSum = (uiSum/10000) + lpAdd->ucSeconds + lpDest->ucSeconds;
// 	lpDest->ucSeconds = uiSum%60;	/* seconds */
// 
// 	uiSum = (uiSum/60) + lpAdd->ucMinutes + lpDest->ucMinutes;
// 	lpDest->ucMinutes = uiSum%60;	/* minutes */
// 
// 	uiSum = (uiSum/60) + lpAdd->ucHours + lpDest->ucHours;
// 	lpDest->ucHours= uiSum%24;	/* hours */
// 
// 	uiSum = (uiSum/24) + lpAdd->usDay + lpDest->usDay;
// 	if(IS_LEAP_YEAR(lpDest->usYear))
// 	{
// 		lpDest->usDay = uiSum%367;
// 		lpDest->usYear += (unsigned short)(uiSum/367);
// 	}
// 	else 
// 	{
// 		lpDest->usDay = uiSum%366;
// 		lpDest->usYear += (unsigned short)(uiSum/366);
// 	}
// }

// void BTimeSubBTime_L(BTIME* lpDest, const BTIME* lpSub)
// {
// 	int flag;
// 	unsigned short usTemp, usDays;
// 
// 	usTemp = 10000 + lpDest->usMiniSec - lpSub->usMiniSec;
// 	if(usTemp > 10000)
// 	{
// 		flag = 0;
// 		lpDest->usMiniSec = usTemp - 10000;
// 	}
// 	else 
// 	{
// 		lpDest->usMiniSec =usTemp;
// 		flag = 1;
// 	}
// 
// 	usTemp = 60 + lpDest->ucSeconds - lpSub->ucSeconds - flag;
// 	if(usTemp > 60)
// 	{
// 		flag = 0;
// 		lpDest->ucSeconds = (unsigned char)(usTemp - 60);
// 	}
// 	else 
// 	{
// 		flag = 1;
// 		lpDest->ucSeconds = (unsigned char)usTemp;
// 	}
// 
// 	usTemp = 60 + lpDest->ucMinutes - lpSub->ucMinutes- flag;
// 	if(usTemp > 60)
// 	{
// 		flag = 0;
// 		lpDest->ucMinutes = (unsigned char)(usTemp - 60);
// 	}
// 	else 
// 	{
// 		flag = 1;
// 		lpDest->ucMinutes = (unsigned char)usTemp;
// 	}
// 
// 	usTemp = 24 + lpDest->ucHours - lpSub->ucHours- flag;
// 	if(usTemp > 24)
// 	{
// 		flag = 0;
// 		lpDest->ucHours = (unsigned char)(usTemp - 24);
// 	}
// 	else 
// 	{
// 		flag = 1;
// 		lpDest->ucHours = (unsigned char)usTemp;
// 	}
// 
// 	usDays = lpDest->usYear - 1;
// 	if(IS_LEAP_YEAR(usDays))
// 		usDays = 366;
// 	else
// 		usDays = 365;
// 
// 	usTemp = usDays + lpDest->usDay - lpSub->usDay- flag;
// 
// 	if(usTemp > usDays)
// 	{
// 		flag = 0;
// 		lpDest->usDay = usTemp - usDays;
// 	}
// 	else 
// 	{
// 		flag = 1;
// 		lpDest->usDay = usTemp;
// 	}
// 
// 	/* year */
// 	lpDest->usYear = lpDest->usYear - lpSub->usYear - flag;
// }