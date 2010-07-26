
#ifndef _EXTRACT_BLOCKETTE_H_
#define _EXTRACT_BLOCKETTE_H_

//check for the validity of the block code 
int IsValidIndexBlockCode(int code);
int IsValidDictionaryBlockCode(int code);
int IsValidStationBlockCode(int code);
int IsValidTimespanBlockCode(int code);
int IsValidBlockCode(int code );
//locate the special header 
void* LocateHeaderAddr(char* lpBase, int iStep, int iSize, int ID);
//locate the data section record address 
void* LocateFirstDataRecord(char* lpBase, int iStep, int iSize);
//the content of the data must be a seed control header record,
//this function will not check for the valid seed header.
void* FindBlockInOneRecord(char* lpRecord, int iStep, int ID, int iGnore, int* iLeft);

void* FindBlockInControlHeader(char* lpBaseH, int iStep, int iSize, int ID);

void* LocateBlockette(char* lpBase, int iStep, int iSize, int ID);

#endif