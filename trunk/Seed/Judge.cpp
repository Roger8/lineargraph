
#include "Judge.h"
int IsUNstr(char* lpstr, int icnt)
{
	int i;
	for( i = 0; i < icnt; i++)
		if( !IS_UN_CHAR(lpstr[i]) )
			return 0;
	return 1;
}
int IsULNstr(char* lpstr, int icnt)
{
	int i;
	for( i = 0; i < icnt; i++)
		if( !IS_ULN_CHAR(lpstr[i]) )
			return 0;
	return 1;
}
int IsUN_NAME(char* lpName, int icnt)
{
	int i;
	for( i = 0; i < icnt; i++ )
		if( !IS_UN_NAME_CHAR(lpName[i]) )
			return 0;
	return 1;
}
