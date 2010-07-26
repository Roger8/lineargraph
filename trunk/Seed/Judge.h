
#ifndef _JUDGE_H_
#define _JUDGE_H_

//
#define  IS_DIGIT(i)			( (i>='0') && (i<='9') )
#define IS_UPPER_CASE(i)		( (i>='A') && (i<='Z') )
#define IS_LOWER_CASE(i)		( (i>='a') && (i<='z') )

//ÊÇ·ñÊÇ¿éµÄÐòÁÐ±êºÅÀàÐÍ×Ö·û
#define IS_CONTROL_HEADER_CHAR(i)	((i=='V') || (i=='A') || (i=='S') || (i=='T'))
#define  IS_DATA_RECORD_CHAR(i)	((i=='D') || (i=='R') || (i=='Q') || (i=='M'))
#define  IS_HEADER_CHAR(i)		((i=='V') || (i=='A') || (i=='S') || (i=='T') || \
	(i=='D') || (i=='R') || (i=='Q') || (i=='M'))

//ÊÇ·ñÊÇ¼ÌÐø×Ö·û
#define  IS_CONTINUE_CODE(i)		((i==' ') || (i=='*'))

//'U' 'N' ±àÂë
#define IS_UN_CHAR(i)		( IS_DIGIT(i) || IS_UPPER_CASE(i) )
#define IS_ULN_CHAR(i)		( IS_DIGIT(i) || IS_UPPER_CASE(i) || IS_LOWER_CASE(i) )
#define IS_UN_NAME_CHAR(i)	( IS_UN_CHAR(i) || i==' ' || i==0 )

#define  UN_CHAR_FILTER(i)	( IS_UN_CHAR ) ? i: 0

#define  IS_UN_STATION(i)		( IS_UN_CHAR(i[0]) && IS_UN_NAME_CHAR(i[1]) && \
		IS_UN_NAME_CHAR(i[2]) && IS_UN_NAME_CHAR(i[3]) && IS_UN_NAME_CHAR(i[4]) )

#define IS_UN_CHN(i)		( IS_UN_CHAR(i[0]) && IS_UN_NAME_CHAR(i[1]) && IS_UN_NAME_CHAR(i[2]) ) 

//×Ö·ûÅÐ¶Ïº¯Êý
int IsUNstr(char* lpstr, int icnt);
int IsULNstr(char* lpstr, int icnt);
int IsUN_NAME(char* lpName, int icnt);


#endif